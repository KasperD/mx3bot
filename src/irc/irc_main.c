/*          
 * src/irc/irc_main.c
 *          
 * Copyright 2001 Colin O'Leary
 *          
 * Initiliazes various /src/irc/ modules.
 *
 */         
            
#include "irc_channel.h"
#include "irc_handler.h"

#include "irc.h"

int irc_init(void)
{
   if(_irc_chan_init() < 0)
      return -1;
   if(_irc_handler_init() < 0)
      return -1;

   return 0;
}
