/*          
 * src/irc/irc_handler.c
 *          
 * Copyright 2001 Colin O'Leary
 *          
 * Routines to handle user callbacks for IRC events.
 *
 */         
            
#include <string.h>

#include "irc_client.h"
#include "irc_channel.h"
#include "irc_handler.h"
#include "irc_queue.h"
#include "irc.h"

/* default handler functions */
static int handle_join(int type, struct irc_handler_info *info);
static int handle_ping(int type, struct irc_handler_info *info);
static int handle_error(int type, struct irc_handler_info *info);
static int handle_numeric(int type, struct irc_handler_info *info);
static int handle_kick(int type, struct irc_handler_info *info);

/* numeric handlers */
static int handle_join_failure(char *param, int n);

static int (*default_handler[IRC_CMD_MAX])
   (int type, struct irc_handler_info *info) =
{
   NULL,		/* unknown */
   NULL,		/* privmsg */
   NULL,		/* ctcp */
   NULL,		/* notice */
   handle_join,		/* join */
   NULL,		/* part */
   NULL,		/* nick */
   NULL,		/* mode */
   handle_kick,		/* kick */
   NULL,		/* quit */
   handle_ping,		/* ping */
   handle_error,	/* error */
   NULL,		/* kill */
   handle_numeric,	/* numeric */
   NULL,		/* timer */
   NULL,		/* dcc */
   NULL,		/* pong */
   NULL,		/* invite */
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL
};

static int (*handler[IRC_CMD_MAX])(struct irc_handler_info *);


/* irc_add_handler() : adds a handler for type, to be called each time
                       this command is received from the server.
                       types are defined in irc.h.
   type : IRC_CMD_* from irc.h
   func : function to be called, will be passed an irc_handler_info
   return negative number on failure
*/
int irc_add_handler(int type, int (*func)(struct irc_handler_info *info))
{
   if((type >= 0) && (type <= IRC_CMD_MAX))
   {
      handler[type] = func;
      return 0;
   }

   return 0;
}


/* _irc_handler_init() : initializes internal structures
   returns negative on failure
*/
int _irc_handler_init(void)
{
   int i;

   for(i = 0; i < IRC_CMD_MAX; i++)
      handler[i] = NULL;

   return 0;
}


/* _irc_handler_exec() : when called, will execute the default handler,
                         and then a supplied command handler, if one has
                         been registered.
   type : IRC_CMD_* from irc.h
   info : structure to pass to handler
*/
int _irc_handler_exec(int type, struct irc_handler_info *info)
{
   int r;

   if(default_handler[type] != NULL)
   {
      r = default_handler[type](type, info);

      if(r != IRC_OK)
         return r;
   }

   if(handler[type] != NULL)
      return (handler[type])(info);

   return IRC_OK;
}


static int handle_join(int type, struct irc_handler_info *info)
{
   /* if bot is joining channel, update chan status */
   if(strcasecmp(info->nick, irc_getnick()) == 0)
   {
      _irc_chan_setstatus(info->target, IRC_CHAN_OK);
   }

   return IRC_OK;
}

static int handle_ping(int type, struct irc_handler_info *info)
{
   _irc_queue_insert(8, "PONG %s", info->param);

   return IRC_OK;
}

static int handle_error(int type, struct irc_handler_info *info)
{
   irc_reconnect(NULL);

   return IRC_OK;
}

static int handle_kick(int type, struct irc_handler_info *info)
{
   if(strcasecmp(info->param, irc_getnick()) == 0)
   {
      _irc_chan_setstatus(info->target, IRC_CHAN_REJOIN);
   }

   return IRC_OK;
}

static int handle_numeric(int type, struct irc_handler_info *info)
{
   switch(info->type)
   {
      case 471:
      case 473:
      case 474:
      case 475:
         return handle_join_failure(info->param, info->type);
      default: break;
   }
 
   return IRC_OK;
}

static int handle_join_failure(char *param, int n)
{
   char *chan;

   chan = strtok(param, " ");
   _irc_chan_setstatus(chan, IRC_CHAN_FAIL);

   if(_irc_chan_getstatus(chan) == IRC_CHAN_JOIN)
   {
      /* repsonse to user join command;  give up */
      _irc_chan_del(chan);
   }
   else
   {
      /* rejoin attempt failed;  keep trying */
   }

   return IRC_OK;
}
