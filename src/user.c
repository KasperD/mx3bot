/*
 * src/user.c
 *
 * Copyright 2001-2002 Colin O'Leary
 *
 * Holds information about all known users, caching select
 *    information from src/userdb.c.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "channel.h"
#include "log.h"
#include "misc.h"
#include "mx3bot.h"
#include "user.h"
#include "userdb.h"

struct user_t
{
   char nick[32];		/* current nick */
   char user[32];		/* username */
   char host[128];		/* hostname */
   int server;			/* server user resides on */
   char chan[N_CHANNELS];	/* reference to channels in channel.c */
   int id;			/* id from userdb_find_bymask() */
   int lost;			/* when did i last see this user? */
};

static struct user_t *user;
static int n_users, n_grow;

static int leave_channel(int server, char *chan);
static int check_user(int i);
static int lookup_user(int i);

/* user_init() : initializes internal structures
*/
int user_init(int users, int grow)
{
   int i, j;

   if(users > 0)
      n_users = users;
   else
      n_users = 256;

   if(grow > 0)
      n_grow = grow;
   else
      n_grow = 64;

   user = malloc(sizeof(struct user_t) * n_users);
   if(user == NULL)
      return -1;

   for(i = 0; i < n_users; i++)
   {
      user[i].nick[0] = 0;
      user[i].user[0] = 0;
      user[i].host[0] = 0;
      user[i].server = -1;

      for(j = 0; j < N_CHANNELS; j++)
         user[i].chan[j] = 0;
   }

   return 0;
}


/* user_find() : returns handle for user specified
   returns -1 if not found
*/
int user_find(int server, char *nick)
{
   int i;

   for(i = 0; i < n_users; i++)
   {
      if((user[i].server == server) &&
         (strcasecmp(user[i].nick, nick) == 0))
         return i;
   }

   return -1;
}


/* user_add() : adds user specified to tables
   returns handle of new user, or -1 on error
*/
int user_add(int server, char *nick, char *username, char *host)
{
   int i, j;

   i = user_find(server, nick);
   if(i != -1)
      return i;

   for(i = 0; i < n_users; i++)
   {
      if(user[i].nick[0] == 0)
      {
         int j;

         strncpy(user[i].nick, nick, 32);
         strncpy(user[i].user, username, 32);
         strncpy(user[i].host, host, 128);
         user[i].server = server;

         /* initially, user is not in sight */
         user[i].lost = time(0);

         for(j = 0; j < N_CHANNELS; j++)
            user[i].chan[j] = 0;

         /* check database for this user */
         lookup_user(i);

         log_debug("Added user `%s!%s@%s (#%d)", nick, username,
            host, user[i].id);

         return i;
      }
   }

   user = realloc(user, sizeof(struct user_t) * (n_users + n_grow));
   if(user == NULL)
   {
      log_error("Out of memory (user_add).");
      return -1;
   }

   i = n_users;
   n_users += n_grow;

   for(; i < n_users; i++)
   {
      user[i].nick[0] = 0;
      user[i].user[0] = 0;
      user[i].host[0] = 0;
      user[i].server = -1;

      for(j = 0; j < N_CHANNELS; j++)
         user[i].chan[j] = 0;
   }      

   return user_add(server, nick, username, host);
}

/* lookup_user() : searches database for user[i], and updates user[i].id.
   i is the index into the user[] array
*/
static int lookup_user(int i)
{
   int id;

   id = userdb_find_bymask3(user[i].nick, user[i].user, user[i].host);

   if((id > 0) && userdb_check_password(id, NULL))
      user[i].id = id;
   else
      user[i].id = 0;

   return id;
}

/* user_del() : removes specified users from tables
   returns -1 if not found
*/
int user_del(int i)
{
   user[i].nick[0] = 0;
   user[i].user[0] = 0;
   user[i].host[0] = 0;

   return 0;
}


/* user_join() : adds user to channel specified
   chan   : channel joined
   n      : user's nick
   u      : user's ident name
   h      : user's host name

   user_join(server, NULL, n, u, h) to add user to tables without having
      user in any of bot's channels.
   user_join(server, chan, NULL, NULL, NULL) is called to 
      when bot joins channel.
*/
int user_join(int server, char *chan, char *n, char *u, char *h)
{
   int c, i;

   /* find user, add if not found */
   i = user_find(server, n);
   if(i == -1)
      i = user_add(server, n, u, h);
   if(i == -1)
      return -1;

   if(chan != NULL)
   {
      /* find channel, add if not found */
      c = chan_find(server, chan);
      if(c == -1)
         c = chan_add(server, chan);
      if(c == -1)
         return -1;

      user[i].chan[c] = 1;

      /* user is in a channel, therefore is in sight */
      user[i].lost = 0;
   }

   log_debug("Join: `%s!%s@%s' %d/%s", n, u, h, server, chan);

   return 0;
}


/* user_part() : removes user from channel user list
   chan   : channel joined
   n      : user's nick
   u      : user's ident name
   h      : user's host name

   user_part(server, chan, NULL) is called when bot parts channel.
   user_part(server, NULL, nick) is called when user quits irc.
*/
int user_part(int server, char *chan, char *nick)
{
   int c, i;

   if(nick == NULL && chan == NULL)
   {
      for(i = 0; i < n_users; i++)
      {
         if(user[i].server == server)
            user_del(i);
      }

      log_debug("Purging all users from server (cid=%d).", server);
      return 0;
   }

   if(nick == NULL)
      return leave_channel(server, chan);

   i = user_find(server, nick);
   if(i == -1)
      return -1;

   if(chan == NULL)
   {
      log_debug("Quit: `%s'", nick);
      user[i].lost = 1;
   }
   else
   {
      log_debug("Part: `%s' %d/%s", nick, server, chan);

      c = chan_find(server, chan);
      if(c == -1)
      return -1;

      user[i].chan[c] = 0;
      check_user(i);
   }

   return 0;
}


/* check if user is still in any channels */
static int check_user(int u)
{
   int i;

   for(i = 0; i < N_CHANNELS; i++)
   {
      if(user[u].chan[i] > 0)
         return 1;
   }

   log_debug("Lost sight of `%s'.", user[u].nick);
   user[u].lost = time(0);

   return 0;
}

/* called when bot leaves channel:
      remove channel from list
      remove channel from all users' active channels
*/
static int leave_channel(int server, char *chan)
{
   int c, i;

   c = chan_find(server, chan);
   if(c == -1)
      return -1;

   for(i = 0; i < n_users; i++)
   {
      if((user[i].nick[0] != 0) && (user[i].chan[c] > 0))
      {
         user[i].chan[c] = 0;
         check_user(i);
      }
   }

   chan_del(server, chan);

   return 0;
}


/* user_nick() : updates a user (changes nick)
   new_nick : new nick
   old_nick : former nick

   returns -1 if not found
*/
int user_nick(int server, char *new_nick, char *old_nick)
{
   int i;

   log_debug("Nick: %s -> %s (cid=%d).", old_nick, new_nick, server);

   for(i = 0; i < n_users; i++)
   {
      if(strcasecmp(user[i].nick, old_nick) == 0)
      {
         strncpy(user[i].nick, new_nick, 32);
         return 0;
      }
   }

   return -1;
}

int user_set_id(int server, char *nick, int id)
{
   int i;

   i = user_find(server, nick);
   if(i == -1)
      return -1;

   user[i].id = id;

   return 0;
}

int user_get_id(int server, char *nick)
{
   int i;

   i = user_find(server, nick);
   if(i == -1)
      return 0;

   return user[i].id;
}

/* user_get_ident() : searchs user table for server/nick pair,
                      returning the user's ident if found.
   returns empty string if not found
*/
char *user_get_ident(int server, char *nick)
{
   int i;
   static char ident[256];

   ident[0] = 0;

   for(i = 0; i < n_users; i++)
   {
      if((user[i].server == server) &&
         (strcasecmp(user[i].nick, nick) == 0))
      {
         snprintf(ident, 256, "%s!%s@%s", user[i].nick,
            user[i].user, user[i].host);
         return ident;
      }
   }

   return ident;
}

/* user_auth() : attempts to authorize user.
 *    returns -1 on failure (user not found, password invalid)
 *    returns 0 on success
 */
int user_auth(int server, char *nick, char *pw)
{
   int i, id;

   i = user_find(server, nick);
   if(i == -1)
      return -1;

   id = userdb_find_bymask3(user[i].nick, user[i].user, user[i].host);

   if(id == 0)
      return -1;

   if(userdb_check_password(id, pw) == 1)
   {
      user[i].id = id;
      return 0;
   }

   return -1;
}

int user_update(int remove_delay)
{
   int i;

   for(i = 0; i < n_users; i++)
   {
      if((user[i].nick[0] != 0) &&
         (user[i].lost != 0) &&
         ((time(0) - user[i].lost) > remove_delay))
      {
         log_debug("Purging `%s'.", user[i].nick);
         user_del(i);
      }
   }

   return 0;
}

int user_set_seen(int server, char *nick)
{
   int i;

   i = user_find(server, nick);
   if(i == -1)
      return -1;

   if(user[i].lost != 0)
      user[i].lost = time(0);

   return 0;
}

void user_debug(int server)
{
}

int user_update_id(int id)
{
   int i;

   for(i = 0; i < n_users; i++)
   {
      if((user[i].nick[0] != 0) &&
         (user[i].id == id))
      {
         lookup_user(i);
      }
   }

   return 0;
}

int user_update_add(int id, char *mask)
{
   int i;

   if(id && mask)
   {
      for(i = 0; i < n_users; i++)
      {
         if((user[i].nick[0] != 0) &&
            (user[i].id == 0) ) // &&
//            (mask_match3(user[i].nick, user[i].user,
//               user[i].host, mask)))
         {
            lookup_user(i);
         }
      }
   }
   else if(id)
   {
      for(i = 0; i < n_users; i++)
      {
         if((user[i].nick[0] != 0) &&
            (user[i].id == 0))
         {
            lookup_user(i);
         }
      }
   }

   return 0;
}

