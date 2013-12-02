/*
 * src/irc/irc_channel.c
 *
 * Copyright 2001-2002 Colin O'Leary
 *
 * Maintains list of channels in which client resides, handles
 * task of joining a channel.
 *
 */

#include <string.h>
#include <time.h>

#include "irc_channel.h"

static struct
{
   char name[32];
   char key[32];
   int status;
   time_t attempt;
} channel[N_CHANNELS];

static int rejoin_delay, failure_delay, initial_delay;

/* _irc_chan_init() : initialize internal structures in channel.c
                         call before using any _irc_channel_* functions
   returns negative number on error
*/
int _irc_chan_init(void)
{
   int i;

   for(i = 0; i < N_CHANNELS; i++)
   {
      channel[i].name[0] = 0;
      channel[i].key[0] = 0;
      channel[i].status = 0;
      channel[i].attempt = 0;
   }

   rejoin_delay = 5;
   failure_delay = 15;
   initial_delay = 60;

   return 0;
}


/* _irc_chan_find() : returns index/handle of channel
   name   : name of channel
   returns -1 if not found
*/
int _irc_chan_find(char *name)
{
   int i;

   for(i = 0; i < N_CHANNELS; i++)
   {
      if(strcasecmp(channel[i].name, name) == 0)
         return i;
   }

   return -1;
}


/* _irc_chan_add() : adds channel to list
   returns -1 on error (table full)
*/
int _irc_chan_add(char *name, char *key)
{
   int i;

   i = _irc_chan_find(name);
   if(i != -1)
      return i;

   for(i = 0; i < N_CHANNELS; i++)
   {
      if(channel[i].name[0] == 0)
      {
         strncpy(channel[i].name, name, 32);

         if(key)
            strncpy(channel[i].key, key, 32);
         else
            channel[i].key[0] = 0;

         return i;
      }
   }

   return -1;
}


/* _irc_chan_del() : removes channel from list
   returns -1 if not found
*/
int _irc_chan_del(char *chan)
{
   int c;

   c = _irc_chan_find(chan);
   if(c == -1)
      return -1;

   channel[c].name[0] = 0;
   channel[c].key[0] = 0;
   channel[c].status = IRC_CHAN_NULL;
   channel[c].attempt = 0;

   return 0;
}

   
/* _irc_chan_getstatus() : returns status value of chan
   returns -1 if not found
*/
int _irc_chan_getstatus(char *chan)
{
   int i;

   i = _irc_chan_find(chan);

   if(i == -1)
      return i;
   else
      return channel[i].status;
}


/* _irc_chan_setstatus() : sets status value of chan
   returns -1 on error
*/
int _irc_chan_setstatus(char *chan, int status)
{
   int c;

   c = _irc_chan_find(chan);
   if(c == -1)
      c = _irc_chan_add(chan, NULL);
   if(c == -1)
      return -1;

   channel[c].status = status;

   return 0;
}


int _irc_chan_rejoin(void)
{
   int i, count;

   count = 0;

   for(i = 0; i < N_CHANNELS; i++)
   {
      if(channel[i].name[0] != 0)
      {
         channel[i].status = IRC_CHAN_REJOIN;
         channel[i].attempt = 0;
         count++;
      }
   }

   return count;
}

int _irc_chan_join(int (*join_func)(char *chan, char *key))
{
   int i, result;

   for(i = 0; i < N_CHANNELS; i++)
   {
      if(channel[i].name[0] != 0)
      {
         if(channel[i].status == IRC_CHAN_OK)
            continue;

         if((channel[i].status == IRC_CHAN_REJOIN) &&
            ((time(0) - channel[i].attempt) < rejoin_delay))
            continue;

         if((channel[i].status == IRC_CHAN_FAIL) &&
            ((time(0) - channel[i].attempt) < failure_delay))
            continue;

         if(channel[i].status == IRC_CHAN_JOIN)
         {
            if((time(0) - channel[i].attempt) < initial_delay)
               continue;

            if(channel[i].attempt != 0)
               channel[i].status = IRC_CHAN_REJOIN;
         }

         channel[i].attempt = time(0);

         if(channel[i].key[0])
            result = join_func(channel[i].name, channel[i].key);
         else
            result = join_func(channel[i].name, NULL);

         if(result != 0)
            return result;
      }
   }

   return 0;
}
