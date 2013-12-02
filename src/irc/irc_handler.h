/*          
 * src/irc/irc_handler.h
 *          
 * Copyright 2001 Colin O'Leary
 *          
 */         
            
#ifndef _irc_handler_h
#define _irc_handler_h

#include "irc.h"

int _irc_handler_init(void);
int _irc_handler_exec(int type, struct irc_handler_info *info);

#endif

