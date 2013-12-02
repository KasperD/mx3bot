/*          
 * src/channel.h
 *          
 * Copyright 2001 Colin O'Leary
 *          
 */         
            
#ifndef _channel_h
#define _channel_h

#define N_CHANNELS 256

int chan_init(void);
int chan_find(int server, char *name);
int chan_add(int server, char *name);
int chan_del(int server, char *name);
int chan_getstatus(int server, char *name);
int chan_setstatus(int server, char *name, int status);

#endif

