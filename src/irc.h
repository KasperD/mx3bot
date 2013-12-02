/*
 * src/irc.h
 *
 * Copyright 2001 Colin O'Leary
 *
 * Definitions for public functions in src/irc/
 */

#ifndef _irc_h
#define _irc_h

/* do not change without modifying tables in src/irc/irc_handler.c */
#define IRC_CMD_UNKNOWN		0
#define IRC_CMD_PRIVMSG		1
#define IRC_CMD_CTCP		2
#define IRC_CMD_NOTICE		3
#define IRC_CMD_JOIN		4
#define IRC_CMD_PART		5
#define IRC_CMD_NICK		6
#define IRC_CMD_MODE		7
#define IRC_CMD_KICK		8
#define IRC_CMD_QUIT		9
#define IRC_CMD_PING		10
#define IRC_CMD_ERROR		11
#define IRC_CMD_KILL		12
#define IRC_CMD_NUMERIC		13
#define IRC_CMD_TIMER		14
#define IRC_CMD_CONNECT		15
#define IRC_CMD_TOPIC		16
#define IRC_CMD_DCC		17
#define IRC_CMD_PONG		18
#define IRC_CMD_INVITE		19
#define IRC_CMD_LOG		20

#define IRC_CMD_MAX		24

#define IRC_OK			0

struct irc_handler_info
{
   int type;
   int server;
   char nick[32];
   char user[32];
   char host[128];
   char target[32];
   char param[512];
   char param2[512];
};

int irc_init(void);
int irc_client(int sid, char *server, int port, char *nick,
   char *username, char *realname);
int irc_dcc(int sid, char *addr, int port);

int irc_add_handler(int type, int (*h)(struct irc_handler_info *info));

int irc_privmsg(char *target, char *text);
int irc_notice(char *target, char *text);
int irc_ctcp(char *target, char *text);
int irc_quote(char *text);
int irc_join(char *chan, char *key);
int irc_part(char *chan, char *msg);
int irc_mode(char *chan, char *mode, char *param);
int irc_kick(char *chan, char *nick, char *msg);
char *irc_getnick(void);
int irc_setnick(char *chan);
int irc_disconnect(char *message);
int irc_reconnect(char *message);
int irc_topic(char *chan, char *topic);
int irc_who(char *target);

#endif
