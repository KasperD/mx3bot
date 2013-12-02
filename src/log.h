/*
 * src/log.h
 *
 * Copyright 2001 Colin O'Leary
 *
 */

#ifndef _log_h
#define _log_h

#define LOGTYPE_NONE		0

/* bot types */
#define LOGTYPE_STDOUT		0x00000001
#define LOGTYPE_DEBUG		0x00000002
#define LOGTYPE_WARNING		0x00000004
#define LOGTYPE_FATAL		0x00000008
#define LOGTYPE_LOGMSG		0x00000010

/* options */
#define LOGTYPE_TIMESTAMP	0x00000100

/* irc types */
#define LOGTYPE_PRIVMSG		0x00010000
#define LOGTYPE_NOTICE		0x00020000
#define LOGTYPE_CTCP		0x00040000
#define LOGTYPE_JOIN		0x00080000
#define LOGTYPE_PART		0x00100000
#define LOGTYPE_TOPIC		0x00200000
#define LOGTYPE_QUIT		0x00400000
#define LOGTYPE_MODE		0x00800000
#define LOGTYPE_KICK		0x01000000
#define LOGTYPE_KILL		0x02000000
#define LOGTYPE_PING		0x04000000
#define LOGTYPE_ERROR		0x08000000

#define LOGTYPE_MESSAGES	(LOGTYPE_PRIVMSG | LOGTYPE_NOTICE |	\
				 LOGTYPE_CTCP)

#define LOGTYPE_VERBOSE		0xffffffff

int log_init(int initial_size, int grow_size);
void log_setpath(char *path);
int log_set(int cid, char *chan, char *filename, int type);
int log_add(int cid, char *chan, int type, char *format, ...);

#define log_debug(x...)				\
   log_add(-1, NULL, LOGTYPE_DEBUG, x)
#define log_warning(x...)			\
   log_add(-1, NULL, LOGTYPE_WARNING, x)
#define log_message(x...)			\
   log_add(-1, NULL, LOGTYPE_LOGMSG, x)
#define log_error(x...)				\
   log_add(-1, NULL, LOGTYPE_FATAL, x)

#endif
