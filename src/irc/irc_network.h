/*          
 * src/irc/irc_network.h
 *          
 * Copyright 2001 Colin O'Leary
 *          
 */         
            
#ifndef _irc_network_h
#define _irc_network_h

int _irc_net_send(char *text);
int _irc_net_connect(char *addr, int port);
int _irc_net_disconnect(void);
int _irc_net_readstatus(int delay);
int _irc_net_readline(char *buf, int maxlen, int delay);

#endif

