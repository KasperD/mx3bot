/*          
 * src/connection.h
 *          
 * Copyright 2001-2002 Colin O'Leary
 *          
 */         
            
#ifndef _connection_h
#define _connection_h

int connection_init(char *fifo);
int connection_shutdown(char *reason);
int connection_open(char *name, char *addr, int port, char *nick, 
   char *username, char *realname, char prefix);
int connection_open_dcc(int parent, char *addr, int port, int uid);
int connection_close(int pid);
int connection_server(int delay);

int connection_send(char *format, ...);
int connection_queue(char *cname, int delay, char *format, ...);

void connection_set_target(int cid);
void connection_set_prefix(int cid, int prefix);
void connection_set_nick(int cid, char *nick);
int connection_get_prefix(int cid);
char *connection_get_name(int cid);
char *connection_get_nick(int cid);
int connection_get_current(void);

#endif

