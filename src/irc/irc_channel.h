/*          
 * src/irc/irc_channel.h
 *          
 * Copyright 2001 Colin O'Leary
 *          
 */         
            
#ifndef _irc_channel_h
#define _irc_channel_h

#define N_CHANNELS 256

#define IRC_CHAN_NULL		0
#define IRC_CHAN_JOIN		1
#define IRC_CHAN_REJOIN		2
#define IRC_CHAN_FAIL		3
#define IRC_CHAN_OK		4

int _irc_chan_init(void);
int _irc_chan_find(char *name);
int _irc_chan_add(char *name, char *key);
int _irc_chan_del(char *name);
int _irc_chan_getstatus(char *name);
int _irc_chan_setstatus(char *name, int status);
int _irc_chan_rejoin(void);
int _irc_chan_join(int (*join_func)(char *chan, char *key));

#endif

