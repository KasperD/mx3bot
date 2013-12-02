/*          
 * src/mode.h
 *          
 * Copyright 2001 Colin O'Leary
 *          
 */         
            
#ifndef _mode_h
#define _mode_h

void mode_set(int user, char *net, char *chan, char *mode, char *p);
void mode_get(int user, char *net, char *chan, char *mode);
int mode_check(int user, char *net, char *chan, int mode, char *param, int len);
char *mode_get_chan_list(char *net, int mode);
void mode_setconfig(const char *filename);
int mode_deluser(int user);

#endif

