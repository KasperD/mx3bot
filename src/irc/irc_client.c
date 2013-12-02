/*          
 * src/irc/irc_client.c	
 *          
 * Copyright 2001-2002 Colin O'Leary
 *          
 * Contains functions to perform basic IRC client activities.
 *
 * Contains irc_client() main loop.
 *
 */         
            
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "irc_channel.h"
#include "irc_client.h"
#include "irc_handler.h"
#include "irc_network.h"
#include "irc_parse.h"
#include "irc_queue.h"
#include "irc.h"

#define READ_DELAY 100
#define QUIT_DELAY 100000

#define QUIT_NONE		0
#define QUIT_RECONNECT		1
#define QUIT_DISCONNECT		2

#define CLIENT_IRC		1
#define CLIENT_DCC		2

static char botnick[32];
static char quit_message[128];
static int quit, type;

static int client(int sid, int ctype, char *addr, int port, char *nick,
   char *username, char *realname);

static int join(char *chan, char *key)
{
   if(key)
      _irc_queue_insert(5, "JOIN %s %s", chan, key);
   else
      _irc_queue_insert(5, "JOIN %s", chan);

   return 0;
}

int _irc_client_message(char *chan, char *msg_type, char *text,
   ...)
{
   char buffer[1024];
   va_list ap;

   if(type == CLIENT_IRC)
   {
      strcpy(buffer, msg_type);
      strcat(buffer, " ");
      strcat(buffer, chan);
      strcat(buffer, " :");
   }
   else
   {
      buffer[0] = 0;
   }

   va_start(ap, text);
   vsnprintf(buffer + strlen(buffer), 512, text, ap);
   va_end(ap);

   return _irc_queue_insert(4, "%s", buffer);
}

int irc_join(char *chan, char *key)
{
   if(chan == NULL)
      return -1;

   if(_irc_chan_add(chan, key) < 0)
      return -1;

   if(_irc_chan_setstatus(chan, IRC_CHAN_JOIN) < 0)
      return -1;

   return 0;
}

int irc_part(char *chan, char *msg)
{
   if(msg)
      _irc_queue_insert(6, "PART %s :%s", chan, msg);
   else
      _irc_queue_insert(6, "PART %s", chan);

   _irc_chan_del(chan);

   return 0;
}

int irc_topic(char *chan, char *topic)
{
   _irc_queue_insert(5, "TOPIC %s :%s", chan, topic);

   return 0;
}

int irc_setnick(char *nick)
{
   strncpy(botnick, nick, 32);
   _irc_queue_insert(6, "NICK %s", nick);

   return 0;
}

char *irc_getnick(void)
{
   return botnick;
}

int irc_who(char *chan)
{
   _irc_queue_insert(5, "WHO %s", chan);
   return 0;
}

int irc_mode(char *chan, char *mode, char *param)
{
   if(mode && param)
      _irc_queue_insert(5, "MODE %s %s %s", chan, mode, param);
   else if(mode)
      _irc_queue_insert(5, "MODE %s %s", chan, mode);
   else
      _irc_queue_insert(5, "MODE %s", chan);

   return 0;
}

int irc_kick(char *chan, char *nick, char *msg)
{
   if(msg)
      _irc_queue_insert(5, "KICK %s %s :%s", chan, nick, msg);
   else
      _irc_queue_insert(5, "KICK %s %s", chan, nick);

   return 0;
}

int irc_privmsg(char *target, char *text)
{
   return _irc_client_message(target, "PRIVMSG", "%s", text);
}

int irc_notice(char *target, char *text)
{
   return _irc_client_message(target, "NOTICE", "%s", text);
}

int irc_ctcp(char *target, char *text)
{
   return _irc_client_message(target, "PRIVMSG", "%c%s%c", 1, text, 1);
}

int irc_quote(char *text)
{
   return _irc_queue_insert(5, "%s", text);
}

int irc_disconnect(char *message)
{
   if(message)
      strncpy(quit_message, message, 128);
   else
      quit_message[0] = 0;

   quit = QUIT_DISCONNECT;

   return 0;
}

int irc_reconnect(char *message)
{
   if(message)
      strncpy(quit_message, message, 128);
   else
      quit_message[0] = 0;

   quit = QUIT_RECONNECT;

   return 0;
}

static void mini_handler(int sid, int type, char *msg)
{
   struct irc_handler_info hinfo;

   hinfo.server = sid;
   hinfo.type = type;
   hinfo.nick[0] = 0;
   hinfo.user[0] = 0;
   hinfo.host[0] = 0;
   hinfo.target[0] = 0;

   if(msg)
   {
      strncpy(hinfo.param, msg, 512);
      hinfo.param[511] = 0;
   }
   else
      hinfo.param[0] = 0;

   hinfo.param2[0] = 0;
   _irc_handler_exec(type, &hinfo);
}

int irc_dcc(int sid, char *addr, int port)
{
   /* disable output buffering */
   _irc_queue_init(-1);

   return client(sid, CLIENT_DCC, addr, port, NULL, NULL, NULL);
}

int irc_client(int sid, char *addr, int port, char *nick, char *username,
   char *realname)
{
   if(_irc_queue_init(0) != 0)
      return -1;

   return client(sid, CLIENT_IRC, addr, port, nick, username, realname);
}

static int client(int sid, int ctype, char *addr, int port, char *nick,
   char *username, char *realname)
{
   struct irc_line line;
   struct irc_handler_info hinfo;
   char buffer[512];
   int token, r, len, active;

   quit = 0;
   type = ctype;
   active = time(0);

   while(quit != QUIT_DISCONNECT)
   {
      quit = 0;

      /* connect */
      r = _irc_net_connect(addr, port);
      if(r < 0)
         return -1;

      _irc_queue_purge();

      if(ctype == CLIENT_IRC)
      {
         usleep(100000);
         _irc_queue_insert(9, "USER %s 0 %s :%s", username, username,
            realname);
         usleep(100000);
         irc_setnick(nick);

         _irc_chan_rejoin();

         mini_handler(sid, IRC_CMD_CONNECT, NULL);

         quit_message[0] = 0;
      }

      /* main loop */
      while(quit == 0)
      {
         len = _irc_net_readline(buffer, 512, READ_DELAY);

         if(len > 0)
         {
            if(ctype == CLIENT_IRC)
            {
               active = time(0);

               token = _irc_parseline(buffer, &line);

               hinfo.server = sid;
               strncpy(hinfo.nick, line.nick, 32);
               strncpy(hinfo.user, line.user, 32);
               strncpy(hinfo.host, line.host, 128);
               strncpy(hinfo.target, line.target, 32);
               strncpy(hinfo.param, line.param, 512);
               strncpy(hinfo.param2, line.param2, 512);

               if(token == IRC_CMD_NUMERIC)
                  hinfo.type = atoi(line.command);
               else
                  hinfo.type = token;

               _irc_handler_exec(token, &hinfo);
            }
            else if(ctype == CLIENT_DCC)
            {
               hinfo.type = IRC_CMD_DCC;
               hinfo.server = sid;
               strncpy(hinfo.param, buffer, 512);

               _irc_handler_exec(IRC_CMD_DCC, &hinfo);
            }
         }
         else if(len == -1)
         {
            /* if readline() returns negative, reconnect */
            if(type == CLIENT_IRC)
            {
               mini_handler(sid, IRC_CMD_LOG, "Read error: reconnecting.");
               quit = QUIT_RECONNECT;
            }
            else
            {
               mini_handler(sid, IRC_CMD_LOG, "Read error: disconnecting.");
               quit = QUIT_DISCONNECT;
            }

            quit_message[0] = 0;
         }
         else if(ctype == CLIENT_IRC)
         {
            int now = time(0);

            if((now - active) > 60)
               _irc_queue_insert(8, "PING %s", botnick);
            else if((now - active) > 120)
            {
               quit_message[0] = 0;
               quit = QUIT_RECONNECT;
            }
         }

         /* run timer handler */
         mini_handler(sid, IRC_CMD_TIMER, NULL);

         /* check channel status */
         if(type == CLIENT_IRC)
            _irc_chan_join(join);

         /* send any queued output */
         _irc_queue_update();

         if(quit != 0)
         {
            if(ctype == CLIENT_IRC)
            {
               if(quit_message[0] == 0)
                  _irc_queue_insert(9, "QUIT");
               else
                  _irc_queue_insert(9, "QUIT :%s", quit_message);
            }
            else
            {
               if(quit_message[0] == 0)
                  _irc_queue_insert(9, "Disconnecting.");
               else
                  _irc_queue_insert(9, "Disconnecting: %s", quit_message);
            }
         }
      }

      usleep(QUIT_DELAY);
      _irc_net_disconnect();
   }

   return 0;
}
