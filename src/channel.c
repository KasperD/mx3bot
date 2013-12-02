/*
 * src/channel.c
 *
 * Copyright 2001 Colin O'Leary
 *
 * Tracks channels that bot currently resides in.  Mainly
 *    for use by src/user.c
 *
 */  
     
#include <string.h>
#include <time.h>

#include "channel.h"

static struct
{
   char name[32];
   int server;
} channel[N_CHANNELS];


/* chan_init() : initialize internal structures in channel.c
                         call before using any _irc_channel_* functions
   returns negative number on error
*/
int chan_init(void)
{
   int i;

   for(i = 0; i < N_CHANNELS; i++)
   {
      channel[i].name[0] = 0;
   }

   return 0;
}


/* chan_find() : returns index/handle of channel
   name   : name of channel
   returns -1 if not found
*/
int chan_find(int server, char *name)
{
   int i;

   for(i = 0; i < N_CHANNELS; i++)
   {
      if(strcasecmp(channel[i].name, name) == 0)
         return i;
   }

   return -1;
}


/* chan_add() : adds channel to list
   returns -1 on error (table full)
*/
int chan_add(int server, char *name)
{
   int i;

   i = chan_find(server, name);
   if(i != -1)
      return i;

   for(i = 0; i < N_CHANNELS; i++)
   {
      if(channel[i].name[0] == 0)
      {
         strncpy(channel[i].name, name, 32);
         return i;
      }
   }

   return -1;
}


/* chan_del() : removes channel from list
   returns -1 if not found
*/
int chan_del(int server, char *chan)
{
   int c;

   c = chan_find(server, chan);
   if(c == -1)
      return -1;

   channel[c].name[0] = 0;

   return 0;
}

   
