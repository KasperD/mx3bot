/*          
 * src/misc.h
 *          
 * Copyright 2001 Colin O'Leary
 *          
 */         
            
#ifndef _misc_h
#define _misc_h

#include <time.h>

char *timestr(time_t t);
void nl2sp(char *str);
int ischan(char *chan);

int mask_match(char *mask, char *val);
int mask_match3(char *nick, char *user, char *host, char *val);
char *mask_create(char *ident, char *mask, int maxlen, int depth);
char *mask_create2(char *user, char *host, char *mask, 
   int maxlen, int depth);
int text_input(char *prompt, char *buf, int len, char *def);

#endif
