/*
 * src/ban.h
 *
 * Copyright 2001 Colin O'Leary
 *
 */

#ifndef _ban_h
#define _ban_h

char *ban_get_list(char *network, char *chan, char *ident);
int ban_get_id(char *network, char *chan, char *ident);
int ban_get_id_bymask(char *network, char *chan, char *mask);
int ban_add(char *network, char *chan, char *mask, int uid, char *note);
int ban_del(int bid);
int ban_get(int bid, int *uid, char *mask, int mask_len, char *note,
   int note_len);
void ban_setconfig(const char *filename);
int ban_deluser(int uid);

#endif

