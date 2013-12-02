/*
 * src/user.h
 *
 * Copyright 2001-2002 Colin O'Leary
 *
 */

#ifndef _user_h
#define _user_h

int user_init(int n, int g);
int user_find(int server, char *nick);
int user_add(int server, char *nick, char *username, char *host);
int user_del(int i);
int user_join(int server, char *chan, char *n, char *u, char *h);
int user_part(int server, char *chan, char *nick);
int user_nick(int server, char *new, char *old);
int user_set_id(int server, char *nick, int id);
int user_get_id(int server, char *nick);
int user_auth(int server, char *nick, char *pass);
int user_update(int remove_delay);
int user_set_seen(int server, char *nick);
int user_update_add(int id, char *mask);
int user_update_id(int id);
char *user_get_ident(int server, char *nick);

void user_debug(int server);

#endif
