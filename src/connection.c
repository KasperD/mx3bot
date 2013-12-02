/*
 * src/connection.c
 *
 * Copyright 2001-2002 Colin O'Leary
 *
 * Handles several things:
 *    FIFO communication between server-parent and children.
 *    Creation and destruction of child processes.
 *    Parsing of messages sent between server and children.
 *    Calling of scripts by server.
 *    Queueing of messages for relaying between children, and for
 *       delayed execution of events.
 *    Signal handling.
 *
 */  

#include "mx3bot.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "connection.h"
#include "irc.h"
#include "log.h"
#include "misc.h"
#include "script.h"
#include "user.h"
#include "var.h"

#define N_CONNECTIONS 32
#define QUEUE_SIZE    512
#define QUEUE_MSGS    64

#define CON_CLOSED	0
#define CON_CLIENT	1
#define CON_DCC		2
#define CON_TELNET	3

#define CHILD_CMD	1
#define CHILD_OK	2
#define CHILD_FAIL	3
#define CHILD_PARSE	4

struct connection_queue
{
   int time;
   int qid;
};

struct connection
{
   /* common */
   int type;
   char addr[128];
   int port;
   int fd;
   int pid;
   struct connection_queue queue[QUEUE_MSGS];

   /* irc */
   char name[32];
   char nick[32];
   char prefix;
   char new_nick[32];

   /* dcc */
   int parent;
   int uid;
};

static char queue[QUEUE_SIZE][512];

static int child_init(int i);
static int child_shutdown(void);
static int read_input(int sock, int (*parse)(char *msg));

static int parse_server_input(char *msg);
static int parse_child_input(char *msg);

/* child irc handlers (callbacks from irc_client()) */
static int handle_notice(struct irc_handler_info *info);
static int handle_privmsg(struct irc_handler_info *info);
static int handle_ctcp(struct irc_handler_info *info);
static int handle_join(struct irc_handler_info *info);
static int handle_part(struct irc_handler_info *info);
static int handle_nick(struct irc_handler_info *info);
static int handle_quit(struct irc_handler_info *info);
static int handle_numeric(struct irc_handler_info *info);
static int handle_timer(struct irc_handler_info *info);
static int handle_topic(struct irc_handler_info *info);
static int handle_dcc(struct irc_handler_info *info);
static int handle_invite(struct irc_handler_info *info);

/* child irc handlers (called from handle_numeric()) */
static int handle_connect(struct irc_handler_info *info);
static int handle_who_reply(struct irc_handler_info *info);
static int handle_nick_inuse(struct irc_handler_info *info);
static int handle_cannot_join(struct irc_handler_info *info);

/* child command handlers */
static void child_msg(char *p);
static void child_ctcp(char *p);
static void child_notice(char *p);
static void child_join(char *p);
static void child_part(char *p);
static void child_topic(char *p);
static void child_mode(char *p);
static void child_kick(char *p);
static void child_disconnect(char *p);
static void child_reconnect(char *p);
static void child_nick(char *p);
static void child_irc(char *p);

/* parent command handlers */
static void server_hello(int cid, char *p);
static void server_msg(int cid, char *p);
static void server_ctcp(int cid, char *p);
static void server_notice(int cid, char *p);
static void server_snotice(int cid, char *p);
static void server_ctcpr(int cid, char *p);
static void server_join(int cid, char *p);
static void server_part(int cid, char *p);
static void server_nick(int cid, char *p);
static void server_quit(int cid, char *p);
static void server_connect(int cid, char *p);
static void server_update(int cid, char *p);
static void server_send(int cid, char *p);
static void server_topic(int cid, char *p);
static void server_note(int cid, char *p);
static void server_dcc(int cid, char *p);
static void server_invite(int cid, char *p);

/* signal handlers */
static RETSIGTYPE sigchild(int s);
static RETSIGTYPE sigint(int s);
static RETSIGTYPE sigterm(int s);
static RETSIGTYPE sigterm_child(int s);

static int send_to_parent(int cid, char *format, ...);
static int send_to_child(int cid, char *format, ...);

static struct connection conn[N_CONNECTIONS];
static int input, output, current_cid, parent_pid, count;
static char fifopath[256], infifo[256];
static int term_count;

int connection_init(char *fifo)
{
   int i, j;

   /* initialize table */
   for(i = 0; i < N_CONNECTIONS; i++)
   {
      conn[i].type = CON_CLOSED;
      conn[i].name[0] = 0;
      conn[i].addr[0] = 0;
      conn[i].port = 0;
      conn[i].fd = -1;

      for(j = 0; j < 64; j++)
      {
         conn[i].queue[j].time = 0;
         conn[i].queue[j].qid = -1;
      }
   }

   /* clear queue */
   for(i = 0; i < QUEUE_SIZE; i++)
      queue[i][0] = 0;

   /* open well-known fifo */
   input = open(fifo, O_RDWR);
   if(input == -1)
   {
      perror("open");
      return -1;
   }

   fcntl(input, F_SETFL, O_NONBLOCK);

   strncpy(fifopath, fifo, 256);

   /* set irc handlers */
   irc_init();
   irc_add_handler(IRC_CMD_NOTICE, handle_notice);
   irc_add_handler(IRC_CMD_PRIVMSG, handle_privmsg);
   irc_add_handler(IRC_CMD_CTCP, handle_ctcp);
   irc_add_handler(IRC_CMD_JOIN, handle_join);
   irc_add_handler(IRC_CMD_PART, handle_part);
   irc_add_handler(IRC_CMD_NICK, handle_nick);
   irc_add_handler(IRC_CMD_QUIT, handle_quit);
   irc_add_handler(IRC_CMD_NUMERIC, handle_numeric);
   irc_add_handler(IRC_CMD_TIMER, handle_timer);
   irc_add_handler(IRC_CMD_TOPIC, handle_topic);
   irc_add_handler(IRC_CMD_DCC, handle_dcc);
   irc_add_handler(IRC_CMD_INVITE, handle_invite);

   current_cid = -1;
   parent_pid = getpid();

   signal(SIGINT, sigint);
   signal(SIGCHLD, sigchild);

   signal(SIGPIPE, SIG_IGN);
   signal(SIGTERM, sigterm);

   /* number of children */
   count = 0;

   return 0;
}

int connection_shutdown(char *reason)
{
   int i, j;

   for(i = 0; i < N_CONNECTIONS; i++)
   {
      if(conn[i].type != CON_CLOSED)
      {
         /* empty message queue */
         for(j = 0; j < QUEUE_MSGS; j++)
         {
            conn[i].queue[j].time = 0;
            conn[i].queue[j].qid = -1;
         }

         /* queue DISCONNECT message */
         sprintf(queue[i], "DISCONNECT %s", reason);
         conn[i].queue[0].time = 0;
         conn[i].queue[0].qid = i;
      }
   }

   log_debug("Shutdown: Sending DISCONNECT to children.");

   return 0;
}

static int connection_find(char *cname)
{
   int i;

   for(i = 0; i < N_CONNECTIONS; i++)
   {
      if((conn[i].type != CON_CLOSED) &&
         (strcasecmp(conn[i].name, cname) == 0))
         return i;
   }

   return -1;
}

int connection_open(char *name, char *addr, int port, char *nick,
   char *username, char *realname, char prefix)
{
   int i, pid;

   for(i = 0; i < N_CONNECTIONS; i++)
   {
      if(conn[i].type == CON_CLOSED)
         break;
   }

   if(i == N_CONNECTIONS)
   {
      log_warning("Cannot connect: max connections reached.");
      return -1;
   }

   strncpy(conn[i].name, name, 32);
   strncpy(conn[i].addr, addr, 128);
   strncpy(conn[i].nick, nick, 32);
   conn[i].port = port;
   conn[i].type = CON_CLIENT;
   conn[i].prefix = prefix;

   pid = fork();
   if(pid < 0)
   {
      log_warning("Cannot connect: fork() failed.");
      return -1;
   }
   else if(pid == 0)
   {
      if(child_init(i) == 0)
      {
         /* run irc loop */
         irc_client(i, addr, port, nick, username, realname);

         /* purge users from this server */
         user_part(i, NULL, NULL);
      }

      child_shutdown();
      _exit(0);
   }

   log_debug("Creating new IRC connection (pid=%d).", pid);

   conn[i].pid = pid;
   count++;

   return i;        
}

int connection_open_dcc(int parent, char *addr, int port, int uid)
{
   int i, pid;

   for(i = 0; i < N_CONNECTIONS; i++)
   {
      if(conn[i].type == CON_CLOSED)
         break;
   }

   if(i == N_CONNECTIONS)
   {
      log_warning("Cannot connect: max connections reached.");
      return -1;
   }

   strncpy(conn[i].addr, addr, 128);
   conn[i].port = port;
   conn[i].type = CON_DCC;

   conn[i].name[0] = 0;
   conn[i].nick[0] = 0;
   conn[i].prefix = 0;
   conn[i].new_nick[0] = 0;

   conn[i].parent = parent;
   conn[i].uid = uid;

   pid = fork();
   if(pid < 0)
   {
      log_warning("Cannot connect: fork() failed.");
      return -1;
   }
   else if(pid == 0)
   {
      if(child_init(i) == 0)
      {
         irc_dcc(i, addr, port);
      }

      child_shutdown();
      _exit(0);
   }

   log_debug("Creating new DCC connection (pid=%d).", pid);

   conn[i].pid = pid;
   count++;

   return i;        
}

int connection_close(int pid)
{
   int i;

   for(i = 0; i < N_CONNECTIONS; i++)
   {
      if((conn[i].type != CON_CLOSED) &&
         (conn[i].pid == pid))
      {
         conn[i].type = CON_CLOSED;
         conn[i].fd = -1;
         conn[i].pid = -1;

         count--;
         log_debug("Connection count is now %d.", count);
         return 0;
      }
   }

   return -1;
}

void connection_set_target(int cid)
{
   current_cid = cid;
}

void connection_set_prefix(int cid, int prefix)
{
   if(cid == -1)
      cid = current_cid;

   conn[cid].prefix = prefix;
}

int connection_get_prefix(int cid)
{
   if(cid == -1)
      cid = current_cid;

   return conn[cid].prefix;
}

char *connection_get_name(int cid)
{
   if(cid == -1)
      cid = current_cid;

   if(conn[cid].type == CON_CLIENT)
      return conn[cid].name;
   else
      return conn[conn[cid].parent].name;
}

char *connection_get_nick(int cid)
{
   if(cid == -1)
      cid = current_cid;

   return conn[cid].nick;
}

void connection_set_nick(int cid, char *nick)
{
   if(cid == -1)
      cid = current_cid;

   log_message("Bot Nick: %s -> %s", conn[cid].nick, nick);

   strncpy(conn[cid].new_nick, nick, 32);
}

int connection_get_current(void)
{
   return current_cid;
}

int connection_send(char *format, ...)
{
   va_list ap;
   char buf[512];

   va_start(ap, format);
   vsnprintf(buf, 512, format, ap);
   va_end(ap);

   return send_to_child(current_cid, "%s", buf);
}

int connection_queue(char *cname, int delay, char *format, ...)
{
   va_list ap;
   int i, j, cid;

   cid = connection_find(cname);
   if(cid == -1)
      return -1;

   for(i = 0; i < QUEUE_SIZE; i++)
   {
      if(queue[i][0] == 0)
      {
         va_start(ap, format);
         vsnprintf(queue[i], 512, format, ap);
         va_end(ap);

         for(j = 0; j < 64; j++)
         {
            if(conn[cid].queue[j].qid == -1)
            {
               conn[cid].queue[j].qid = i;
               return 0;
            }
         }

         log_warning("Message queue overflow (cid=%d).", cid);
      }
   }

   return -1;
}

static int send_to_parent(int cid, char *format, ...)
{
   va_list ap;
   char buf[512];
   int len, r;

   sprintf(buf, "%d ", cid);
   len = strlen(buf);

   va_start(ap, format);
   vsnprintf(buf + len, 511 - len, format, ap);
   va_end(ap);

   len = strlen(buf);
   buf[len] = '\n';
   buf[len + 1] = 0;

   len = write(output, buf, strlen(buf));
   if(len == -1)
   {
      log_message("Lost connect to parent.  Disconnecting.");
      _exit(0);
   }

   if(input == -1)
   {
      log_warning("Input is not open (cid=%d).", cid);
      return 0;
   }

   /* handle anything the server sends back until OK or FAIL is sent */
   do
   {
      r = read_input(input, parse_child_input);
   }
   while((r != CHILD_OK) && (r != CHILD_FAIL));

   return 0;
}

static int send_to_child(int cid, char *format, ...)
{
   va_list ap;
   char buf[512];
   int len;

   va_start(ap, format);
   vsnprintf(buf, 511, format, ap);
   va_end(ap);

   if(conn[cid].type == CON_CLOSED || conn[cid].fd == -1)
   {
      log_warning("Trying to write to closed connection (cid=%d).", cid);
      return -1;
   }

   len = strlen(buf);
   buf[len] = '\n';
   buf[len + 1] = 0;

   len = write(conn[cid].fd, buf, strlen(buf));
   if(len == -1)
   {
      perror("write");
      return -1;
   }

   return 0;
}

static int child_init(int i)
{
   close(input);

   term_count = 0;   

   /* use different handler than parent */
   signal(SIGINT, sigterm_child);
   signal(SIGTERM, sigterm_child);

   /* open parent's well-known input */
   output = open(fifopath, O_WRONLY);
   if(output == -1)
   {
      perror("open");
      return -1;
   }

   /* open child's specific input */
   sprintf(infifo, "/tmp/mx3bot.%d", getpid());

   if(mkfifo(infifo, 0600) == -1)
   {
      perror("mkfifo");
      return -1;
   }

   input = open(infifo, O_RDWR);
   if(input == -1)   
   {
      perror("child/input open");
      return -1;
   }

   log_debug("Child (pid=%d) listening on `%s'.", getpid(), infifo);

   send_to_parent(i, "HELLO %s", infifo);

   return 0;
}

static int child_shutdown(void)
{
   log_debug("Removing `%s'.", infifo);
   unlink(infifo);

   return 0;
}


static int handle_notice(struct irc_handler_info *info)
{
   if(info->user[0] == 0 && info->host[0] == 0)
   {
      send_to_parent(info->server, "SNOTICE %s %s",
         info->nick, info->param);
   }
   else if(info->param[0] == 1)
   {
      char *ctcp, *param;

      ctcp = info->param + 1;
      ctcp = strtok(ctcp, " ");
      param = strtok(NULL, "\001");

      send_to_parent(info->server, "CTCPR %s %s %s %s %s",
         info->nick, info->user, info->host, ctcp, param);
   }
   else
   {
      send_to_parent(info->server, "NOTICE %s %s %s %s %s",
         info->target, info->nick, info->user, info->host, info->param);
   }

   return IRC_OK;
}

static int handle_privmsg(struct irc_handler_info *info)
{
   send_to_parent(info->server, "MSG %s %s %s %s %s",
      info->target, info->nick, info->user, info->host, info->param);

   return IRC_OK;
}

static int handle_ctcp(struct irc_handler_info *info)
{
   char *ctcp, *param;
   char reply[512], msg[512];

   ctcp = info->param + 1;
   ctcp = strtok(ctcp, " ");
   param = strtok(NULL, "\001");

   send_to_parent(info->server, "CTCP %s %s %s %s %s %s",
      info->target, info->nick, info->user, info->host, info->param,
      info->param2);

   if(strcmp(info->param, "VERSION") == 0)
   {
      var_get_def(0, "version", msg, 512, "http://bot.mx3.org");
      snprintf(reply, 512, "%cVERSION Mx3bot %s: %s%c", 1, VERSION,
         msg, 1);
      irc_notice(info->nick, reply);
   }
   else if(strcmp(info->param, "USERINFO") == 0)
   {
      var_get_def(0, "userinfo", msg, 512, "");
      snprintf(reply, 512, "%cUSERINFO %s%c", 1, msg, 1);
      irc_notice(info->nick, reply);
   }
   else if(strcmp(info->param, "PING") == 0)
   {
      snprintf(reply, 512, "%cPING %s%c", 1, info->param2, 1);
      irc_notice(info->nick, reply);
   }
   else if(strcmp(info->param, "CLIENTINFO") == 0)
   {
      snprintf(reply, 512, "%cCLIENTINFO %s%c", 1,
         "ACTION CLIENTINFO PING USERINFO VERSION TIME", 1);
      irc_notice(info->nick, reply);
   }
   else if(strcmp(info->param, "TIME") == 0)
   {
      snprintf(reply, 512, "%cTIME %s%c", 1, timestr(time(0)), 1);
      irc_notice(info->nick, reply);
   }

   return IRC_OK;
}

static int handle_join(struct irc_handler_info *info)
{
   /* when bot joins channel, update sone things */
   if(strcasecmp(info->nick, irc_getnick()) == 0)
   {
      /* get list of users */
      irc_who(info->target);

      /* get channel modes */
      //irc_mode(info->target, NULL, NULL);

      /* get channel banlist */
      //irc_mode(info->target, "b", NULL);
   }

   send_to_parent(info->server, "JOIN %s %s %s %s JOIN", info->target,
      info->nick, info->user, info->host);
   return IRC_OK;
}

static int handle_part(struct irc_handler_info *info)
{
   send_to_parent(info->server, "PART %s %s", info->target, info->nick);
   return IRC_OK;
}

static int handle_nick(struct irc_handler_info *info)
{
   send_to_parent(info->server, "NICK %s %s", info->param, info->nick);
   return IRC_OK;
}

static int handle_quit(struct irc_handler_info *info)
{
   send_to_parent(info->server, "QUIT %s", info->nick);
   return IRC_OK;
}

static int handle_numeric(struct irc_handler_info *info)
{
   switch(info->type)
   {
      case 4:	/* server information on connect */
         return handle_connect(info);
      case 352:	/* who reply */
         return handle_who_reply(info);
      case 433: /* nick is already in use */
         return handle_nick_inuse(info);
      case 471: /* cannot join: channel is full */
      case 473: /* cannot join: invite only */
      case 474: /* cannot join: banned */
      case 475: /* cannot join: keyed */
         return handle_cannot_join(info);
      default:
         return 0;
   }
}

static int handle_who_reply(struct irc_handler_info *info)
{
   char *chan, *user, *host, *server, *nick;

   chan = strtok(info->param, " ");
   user = strtok(NULL, " ");
   host = strtok(NULL, " ");
   server = strtok(NULL, " ");
   nick = strtok(NULL, " ");

   send_to_parent(info->server, "JOIN %s %s %s %s WHO", chan, nick, user,
      host);
   return IRC_OK;
}

static int handle_nick_inuse(struct irc_handler_info *info)
{
   log_warning("Nick is in use.");

   send_to_parent(info->server, "NOTE NICKINUSE");
   return IRC_OK;
}

static int handle_cannot_join(struct irc_handler_info *info)
{
   send_to_parent(info->server, "NOTE JOINFAILURE %s %d", info->param,
      info->type);
   return IRC_OK;
}

static int handle_timer(struct irc_handler_info *info)
{
   if(term_count > 1)
      irc_disconnect(NULL);
   else
      send_to_parent(info->server, "UPDATE");

   return IRC_OK;
}

static int handle_topic(struct irc_handler_info *info)
{
   send_to_parent(info->server, "TOPIC %s %s %s %s %s", info->nick,
      info->user, info->host, info->target, info->param);
   return IRC_OK;
}

static int handle_dcc(struct irc_handler_info *info)
{
   send_to_parent(info->server, "DCC %s", info->param);
   return IRC_OK;
}

static int handle_invite(struct irc_handler_info *info)
{
   send_to_parent(info->server, "INVITE %s %s %s %s", info->param,
      info->nick, info->user, info->host);
   return IRC_OK;
}

static int handle_connect(struct irc_handler_info *info)
{
   strtok(info->param, " ");
   send_to_parent(info->server, "CONNECT %s", info->param);
   return IRC_OK;
}

int connection_server(int delay)
{
   struct timeval tv;
   fd_set fds;
   int r;

   /* exit if there are no more children */
   if(count == 0)
      return -1;

   tv.tv_sec = delay / 1000;
   tv.tv_usec = (delay % 1000) * 1000;

   FD_ZERO(&fds);
   FD_SET(input, &fds);

   do
   {
      r = select(input + 1, &fds, NULL, NULL, &tv);
   }
   while((r == -1) && (errno == EINTR));

   if(!FD_ISSET(input, &fds))
      return 0;

   read_input(input, parse_server_input);

   return 0;
}

static int read_input(int sock, int (*parse)(char *msg))
{
   char buffer[512];
   int len, i, ch;

   i = 0;
   ch = 0;

   while(ch != '\n')
   {
      len = read(sock, &ch, 1);

      if(len == -1)
      {
         perror("read");
         return -1;
      }

      buffer[i] = ch;

      if(ch == '\n')
         buffer[i] = 0;
      else
         i++;
   }

   return parse(buffer);
}

static int parse_server_input(char *msg)
{
   char *id, *cmd, *param;
   int cid;

   id = strtok(msg, " ");
   cmd = strtok(NULL, " ");

   if(id == NULL)
   {
      log_warning("Garbage server input: no child id");
      return -1;
   }
   if(cmd == NULL)
   {
      log_warning("Garbage server input: no command");
      return -1;
   }

   param = strtok(NULL, "");
   cid = atoi(id);

   if(strcmp(cmd, "HELLO") == 0)
      server_hello(cid, param);
   else if(strcmp(cmd, "MSG") == 0)
      server_msg(cid, param);
   else if(strcmp(cmd, "CTCP") == 0)
      server_ctcp(cid, param);
   else if(strcmp(cmd, "NOTICE") == 0)
      server_notice(cid, param);
   else if(strcmp(cmd, "SNOTICE") == 0)
      server_snotice(cid, param);
   else if(strcmp(cmd, "CTCPR") == 0)
      server_ctcpr(cid, param);
   else if(strcmp(cmd, "JOIN") == 0)
      server_join(cid, param);
   else if(strcmp(cmd, "PART") == 0)
      server_part(cid, param);
   else if(strcmp(cmd, "NICK") == 0)
      server_nick(cid, param);
   else if(strcmp(cmd, "QUIT") == 0)
      server_quit(cid, param);
   else if(strcmp(cmd, "CONNECT") == 0)
      server_connect(cid, param);
   else if(strcmp(cmd, "UPDATE") == 0)
      server_update(cid, param);
   else if(strcmp(cmd, "SEND") == 0)
      server_send(cid, param);
   else if(strcmp(cmd, "TOPIC") == 0)
      server_topic(cid, param);
   else if(strcmp(cmd, "NOTE") == 0)
      server_note(cid, param);
   else if(strcmp(cmd, "DCC") == 0)
      server_dcc(cid, param);
   else if(strcmp(cmd, "INVITE") == 0)
      server_invite(cid, param);
   else
   {
      log_warning("Garbage server input: unknown command `%s'.", cmd);
   }   

   return 0;
}

static int parse_child_input(char *msg)
{
   char *cmd, *param;

   cmd = strtok(msg, " ");
   param = strtok(NULL, "");

   if(cmd == NULL)
   {
      log_warning("Garbage child input: no command");
      return CHILD_PARSE;
   }

   if(strcmp(cmd, "OK") == 0)
      return CHILD_OK;
   else if(strcmp(cmd, "FAIL") == 0)
      return CHILD_FAIL;
   if(strcmp(cmd, "MSG") == 0)
      child_msg(param);
   else if(strcmp(cmd, "CTCP") == 0)
      child_ctcp(param);
   else if(strcmp(cmd, "NOTICE") == 0)
      child_notice(param);
   else if(strcmp(cmd, "JOIN") == 0)
      child_join(param);
   else if(strcmp(cmd, "PART") == 0)
      child_part(param);
   else if(strcmp(cmd, "TOPIC") == 0)
      child_topic(param);
   else if(strcmp(cmd, "DISCONNECT") == 0)
      child_disconnect(param);
   else if(strcmp(cmd, "RECONNECT") == 0)
      child_reconnect(param);
   else if(strcmp(cmd, "NICK") == 0)
      child_nick(param);
   else if(strcmp(cmd, "MODE") == 0)
      child_mode(param);
   else if(strcmp(cmd, "KICK") == 0)
      child_kick(param);
   else if(strcmp(cmd, "IRC") == 0)
      child_irc(param);
   else
   {
      log_debug("Garbage child input: unknown command `%s'.", cmd);
      return CHILD_PARSE;
   }   

   return CHILD_CMD;
}

static void server_hello(int cid, char *msg)
{
   char *fifoname;

   fifoname = strtok(msg, " ");
   log_debug("Opening `%s' for output (cid=%d).", fifoname, cid);

   conn[cid].fd = open(fifoname, O_WRONLY);
   if(conn[cid].fd == -1)
   {
      perror("parent: open child");
      return;
   }

   send_to_child(cid, "OK HELLO");
}

static int user_status(int cid, int add, char *nick, char *user,
   char *host)
{
   int id;

   id = user_get_id(cid, nick);

   if(id == 0 && add)
   {
      /* add user to table */
      user_join(cid, NULL, nick, user, host);

      /* try again now that user is in table */
      id = user_get_id(cid, nick);
   }

   if(id != 0)
      user_set_seen(cid, nick);

   return id;
}

static void server_msg(int cid, char *msg)
{
   char *nick, *user, *host, *target, *command, *p, *param, *reply;
   char str[64];
   int direct, uid;

   str[0] = 0;

   target = strtok(msg, " ");
   nick = strtok(NULL, " ");
   user = strtok(NULL, " ");
   host = strtok(NULL, " ");
   p = strtok(NULL, "");

   /* get user id, level if possible */
   uid = user_status(cid, 1, nick, user, host);

   /* write to log */
   if(ischan(target))
   {
      log_add(cid, target, LOGTYPE_PRIVMSG, "<%s!%s@%s> %s", 
         nick, user, host, p);
   }

   /* call _msg */
   script_exec(cid, "_msg", NULL,
      "Snick", nick,
      "Suser", user,
      "Shost", host,
      "Starget", target,
      "Sparam", p,
      "Sbotnick", conn[cid].nick,
      "Iuid", uid,
      NULL);

   if(strcasecmp(target, conn[cid].nick) == 0)
      direct = 1;
   else
      direct = 0;

   command = NULL;
   param = NULL;

   if(p[0] == conn[cid].prefix)
   {
      command = strtok(p, " ");
      param = strtok(NULL, "");

      if(command)
         command++;
   }
   else if(direct)
   {
      command = strtok(p, " ");
      param = strtok(NULL, "");
   }

   if(direct == 1)
      reply = nick;
   else
      reply = target;

   if(command)
   {
      if(command[0] == '_')
      {
         /* reserved for system commands */
         send_to_child(cid, "OK MSG");
         return;
      }

      if(param == NULL)
         param = str;

      /* call specific script */
      script_exec(cid, command, nick,
         "Snick", nick,
         "Suser", user,
         "Shost", host,
         "Starget", target,
         "Sreply", reply,
         "Sparam", param,
         "Sbotnick", conn[cid].nick,
         "Iuid", uid,
         "Ccp", conn[cid].prefix,
         NULL);
   }

   send_to_child(cid, "OK MSG");
}

static void server_ctcp(int cid, char *msg)
{
   char *nick, *user, *host, *target, *ctcp, *param;
   int uid;

   target = strtok(msg, " ");
   nick = strtok(NULL, " ");
   user = strtok(NULL, " ");
   host = strtok(NULL, " ");
   ctcp = strtok(NULL, " ");
   param = strtok(NULL, "");

   if(ischan(target))
   {
      log_add(cid, target, LOGTYPE_CTCP, "*%s!%s@%s* %s",
         nick, user, host, param);
   }

   uid = user_status(cid, 1, nick, user, host);
   script_exec(cid, "_ctcp", NULL,
      "Snick", nick,
      "Suser", user,
      "Shost", host,
      "Starget", target,
      "Sctcp", ctcp,
      "Sparam", param,
      "Sbotnick", conn[cid].nick,
      "Iuid", uid,
      NULL);

   send_to_child(cid, "OK CTCP");
}

static void server_notice(int cid, char *msg)
{
   char *nick, *user, *host, *target, *param;

   target = strtok(msg, " ");
   nick = strtok(NULL, " ");
   user = strtok(NULL, " ");
   host = strtok(NULL, " ");
   param = strtok(NULL, "");

   if(ischan(target))
   {
      log_add(cid, target, LOGTYPE_NOTICE, "-%s!%s@%s- %s",
         nick, user, host, param);
   }

   script_exec(cid, "_notice", NULL,
      "Snick", nick,
      "Suser", user,
      "Shost", host,
      "Starget", target,
      "Sparam", param,
      "Sbotnick", conn[cid].nick,
      NULL);

   send_to_child(cid, "OK NOTICE");
}

static void server_snotice(int cid, char *msg)
{
   char *server, *param;

   server = strtok(msg, " ");
   param = strtok(NULL, "");

   script_exec(cid, "_server_notice", NULL,
      "Sserver", server,
      "Sparam", param,
      "Sbotnick", conn[cid].nick,
      NULL);

   send_to_child(cid, "OK SNOTICE");
}

static void server_ctcpr(int cid, char *msg)
{
   char *nick, *user, *host, *ctcp, *param;

   nick = strtok(msg, " ");
   user = strtok(NULL, " ");
   host = strtok(NULL, " ");
   ctcp = strtok(NULL, " ");
   param = strtok(NULL, "");

   script_exec(cid, "_ctcp_reply", NULL,
      "Snick", nick,
      "Suser", user,
      "Shost", host,
      "Sctcp", ctcp,
      "Sparam", param,
      "Sbotnick", conn[cid].nick,
      NULL);

   send_to_child(cid, "OK CTCPR");
}

static void server_join(int cid, char *p)
{
   char *target, *nick, *user, *host, *type;
   int uid;

   target = strtok(p, " ");
   nick = strtok(NULL, " ");
   user = strtok(NULL, " ");
   host = strtok(NULL, " ");
   type = strtok(NULL, " ");

   user_join(cid, target, nick, user, host);

   if((strcmp(type, "JOIN") == 0) &&
      (strcmp(nick, conn[cid].nick) == 0))
   {
      char filename[256];

      /* set logging for this channel */
      sprintf(filename, "%s-%s.log", conn[cid].name, target);
      log_set(cid, target, filename, LOGTYPE_VERBOSE);
   }

   /* ignore on who replies */
   if(strcmp(type, "WHO") != 0)
   {
      uid = user_status(cid, 0, nick, user, host);

      log_add(cid, target, LOGTYPE_JOIN, "-- Join: %s!%s@%s (#%d) --",
         nick, user, host, uid);

      script_exec(cid, "_join", NULL,
         "Schan", target,
         "Snick", nick,
         "Suser", user,
         "Shost", host,
         "Sbotnick", conn[cid].nick,
         "Ccp", conn[cid].prefix,
         "Iuid", uid,
         NULL);
   }

   send_to_child(cid, "OK JOIN");
}

static void server_part(int cid, char *p)
{
   char *target, *nick;
   int uid;

   target = strtok(p, " ");
   nick = strtok(NULL, " ");

   user_part(cid, target, nick);

   log_add(cid, target, LOGTYPE_PART, "-- Part: %s --", nick);

   uid = user_status(cid, 0, nick, NULL, NULL);
   script_exec(cid, "_part", NULL,
      "Schan", target,
      "Snick", nick,
      "Sbotnick", conn[cid].nick,
      "Iuid", uid,
      NULL);

   send_to_child(cid, "OK PART");
}

static void server_nick(int cid, char *p)
{
   char *new, *old;

   new = strtok(p, " ");
   old = strtok(NULL, " ");

   user_nick(cid, new, old);
   send_to_child(cid, "OK NICK");
}

static void server_quit(int cid, char *p)
{
   char *nick;

   nick = strtok(p, " ");

   user_part(cid, NULL, nick);
   send_to_child(cid, "OK QUIT");
}

static void server_connect(int cid, char *p)
{
   char *server;

   server = strtok(p, " ");

   /* this might be a reconnection, so clear out users */
   user_part(cid, NULL, NULL);

   script_exec(cid, "_connect", NULL,
      "Sserver", server,
      "Sbotnick", conn[cid].nick,
      NULL);
   send_to_child(cid, "OK CONNECT");
}

static void server_update(int cid, char *p)
{
   int i;

   /* check for users to purge */
   user_update(purge_delay);

   /* check message queue */
   for(i = 0; i < 64; i++)
   {
      if((conn[cid].queue[i].qid != -1) && 
         (conn[cid].queue[i].time <= time(0)))
      {
         send_to_child(cid, "%s", queue[conn[cid].queue[i].qid]);
         queue[i][0] = 0;
         conn[cid].queue[i].qid = -1;
      }
   }

   /* change nick if needed */
   if(conn[cid].new_nick[0] != 0)
   {
      send_to_child(cid, "NICK %s", conn[cid].new_nick);

      strncpy(conn[cid].nick, conn[cid].new_nick, 32);
      conn[cid].new_nick[0] = 0;
   }

   send_to_child(cid, "OK UPDATE");
}

static void server_send(int cid, char *p)
{
   char *target, *msg;
   int i;

   target = strtok(p, " ");
   msg = strtok(NULL, "");

   /* enqueue `msg' */
   for(i = 0; i < QUEUE_SIZE; i++)
   {
      if(queue[i][0] == 0)
      {
         snprintf(queue[i], 512, "%d %s", atoi(target), msg);
         send_to_child(cid, "OK SEND");
      }
   }

   log_warning("Message queue is full.  Discarding message (cid=%d, "
      "`%s').", atoi(target), msg);
   send_to_child(cid, "FAIL SEND");
}

static void server_topic(int cid, char *p)
{
   char *nick, *user, *host, *target, *param;
   int uid;

   nick = strtok(p, " ");
   user = strtok(NULL, " ");
   host = strtok(NULL, " ");
   target = strtok(NULL, " ");
   param = strtok(NULL, "");

   uid = user_status(cid, 0, nick, user, host);

   log_add(cid, target, LOGTYPE_TOPIC, "-- Topic: <%s!%s@%s> %s --",
      nick, user, host, param);

   script_exec(cid, "_topic", NULL,
      "Snick", nick,
      "Suser", user,
      "Shost", host,
      "Starget", target,
      "Sparam", param,
      "Sbotnick", conn[cid].nick,
      "Iuid", uid,
      NULL);
   send_to_child(cid, "OK TOPIC");
}

static void server_note(int cid, char *p)
{
   char *type;

   type = strtok(p, " ");

   if(strcmp(type, "NICKINUSE") == 0)
   {
      if(strlen(conn[cid].nick) < 31)
      {
         strcat(conn[cid].nick, "_");
         send_to_child(cid, "NICK %s", conn[cid].nick);
      }
   }
   else if(strcmp(type, "JOINFAILURE") == 0)
   {
      char *chan, *reason;

      chan = strtok(NULL, " ");
      reason = strtok(NULL, " ");

      if(chan && reason)
      {
         switch(atoi(reason))
         {
            case 471:
               log_debug("Cannot join `%s': channel is full.", chan);
               break;
            case 473:
               log_debug("Cannot join `%s': invite only channel.", chan);
               break;
            case 474:
               log_debug("Cannot join `%s': banned from channel.", chan);
               break;
            case 475:
               log_debug("Cannot join `%s': bad channel key.", chan);
               break;
         }
      }
   }
   else
   {
      log_debug("Garbage note input: unknown type `%s'.", type);
      send_to_child(cid, "FAIL NOTE");
      return;
   }

   send_to_child(cid, "OK NOTE");
}

static void server_dcc(int cid, char *p)
{
   char *cmd, *param;

   cmd = strtok(p, " ");
   param = strtok(NULL, "");

   if(cmd)
   {
      script_exec(cid, cmd, NULL,
         "Snick", "=dcc",
         "Suser", "=dcc",
         "Shost", conn[cid].addr,
         "Starget", "",
         "Sparam", param,
         "Sbotnick", "=dcc",
         "Iuid", conn[cid].uid,
         NULL);
   }

   send_to_child(cid, "OK DCC");
}

static void server_invite(int cid, char *p)
{
   char *target, *nick, *user, *host;
   int uid;

   target = strtok(p, " ");
   nick = strtok(NULL, " ");
   user = strtok(NULL, " ");
   host = strtok(NULL, " ");

   uid = user_status(cid, 1, nick, user, host);

   script_exec(cid, "_invite", NULL,
      "Snick", nick,
      "Suser", user,
      "Shost", host,
      "Starget", target,
      "Sbotnick", conn[cid].nick,
      "Iuid", uid,
      NULL);

   send_to_child(cid, "OK INVITE");
}

static void child_msg(char *p)
{
   char *target, *msg;

   target = strtok(p, " ");
   msg = strtok(NULL, "");

   irc_privmsg(target, msg);
}

static void child_ctcp(char *p)
{
   char *target, *msg;

   target = strtok(p, " ");
   msg = strtok(NULL, "");

   irc_ctcp(target, msg);
}

static void child_notice(char *p)
{
   char *target, *msg;

   target = strtok(p, " ");
   msg = strtok(NULL, "");

   irc_notice(target, msg);
}

static void child_join(char *p)
{
   char *chan, *key;

   chan = strtok(p, " ");
   key = strtok(NULL, "");

   irc_join(chan, key);
}

static void child_part(char *p)
{
   char *chan, *message;

   chan = strtok(p, " ");
   message = strtok(NULL, "");

   irc_part(chan, message);
}

static void child_topic(char *p)
{
   char *chan, *topic;

   chan = strtok(p, " ");
   topic = strtok(NULL, "");

   irc_topic(chan, topic);
}

static void child_mode(char *p)
{
   char *chan, *mode, *param;

   chan = strtok(p, " ");
   mode = strtok(NULL, " ");
   param = strtok(NULL, "");

   irc_mode(chan, mode, param);
}

static void child_kick(char *p)
{
   char *chan, *nick, *param;

   chan = strtok(p, " ");
   nick = strtok(NULL, " ");
   param = strtok(NULL, "");

   irc_kick(chan, nick, param);
}

static void child_irc(char *p)
{
   irc_quote(p);
}

static void child_disconnect(char *p)
{
   irc_disconnect(p);
}

static void child_reconnect(char *p)
{
   irc_reconnect(p);
}

static void child_nick(char *p)
{
   char *nick;

   nick = strtok(p, " ");

   irc_setnick(nick);
}

static RETSIGTYPE sigchild(int s)
{
   connection_close(wait(0));
}

static RETSIGTYPE sigint(int s)
{
   char reason[512];

   var_get_def(0, "msg:sigint", reason, 512, "SIGINT");
   connection_shutdown(reason);
}

static RETSIGTYPE sigterm(int s)
{
   char reason[512];

   var_get_def(0, "msg:sigterm", reason, 512, "SIGTERM");
   connection_shutdown(reason);
}

static RETSIGTYPE sigterm_child(int s)
{
   /* ignore first term, assume parent will handle it. */
   /* try to disconnect on second term. */
   /* give up on third. */

   term_count++;

   if(term_count > 2)
      _exit(-1);
}

