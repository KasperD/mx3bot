/*
 * src/db/sql/mode.c
 *
 * Copyright 2001-2002 Colin O'Leary
 *
 * Handles setting/unsetting/checking of various modes.
 *
 */  

#include "mx3bot.h"

#ifdef HAVE_SQL
   
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "mode.h"
#include "mx3bot.h"
#include "sql.h"

void sql_mode_set(int uid, char *network, char *chan, char *mode,
   char *param)
{
   char chan_buf[64], network_buf[64], param_buf[512];
   DB_RESULT result;
   DB_ROW row;

   if(mode[0] == 0 || mode[1] == 0)
      return;

   if(mode[0] != '+' && mode[0] != '-')
      return;

   if(!isalpha(mode[1]))
      return;

   if(chan)
      db_escape(chan_buf, chan, 32);
   else
      chan_buf[0] = 0;

   if(network)
      db_escape(network_buf, network, 32);
   else
      network_buf[0] = 0;

   if(param)
      db_escape(param_buf, param, 256);
   else
      param_buf[0] = 0;

   result = db_query(0, "SELECT COUNT(*) FROM bot_mode WHERE uid = %d "
      "AND net = '%s' AND chan = '%s' AND action = '%c'",
      uid, network_buf, chan_buf, mode[1]);

   row = db_fetch(result);

   if(db_int(row[0]) > 0)
   {
      if(mode[0] == '+')
      {
         db_query(0, "UPDATE bot_mode SET param = '%s' WHERE uid = %d "
            "AND net = '%s' AND chan = '%s' AND action = '%c'",
            param_buf, uid, network_buf, chan_buf, mode[1]);
      }
      else
      {
         db_query(0, "DELETE FROM bot_mode WHERE uid = %d AND net = '%s' "
            "AND chan = '%s' AND action = '%c'", uid, network_buf,
            chan_buf, mode[1]);
      }
   }
   else
   {
      if(mode[0] == '+')
      {
         db_query(0, "INSERT INTO bot_mode (uid, chan, net, action, param) "
            "VALUES(%d, '%s', '%s', '%c', '%s')", uid, chan_buf,
            network_buf, mode[1], param_buf);
      }
   }

   db_free(result);
}

void sql_mode_get(int uid, char *network, char *chan, char *mode)
{
   char chan_buf[64], network_buf[64];
   int i;
   DB_RESULT result;
   DB_ROW row;

   if(chan)
      db_escape(chan_buf, chan, 32);
   else
      chan_buf[0] = 0;

   if(network)
      db_escape(network_buf, network, 32);
   else
      network_buf[0] = 0;

   result = db_query(0, "SELECT action FROM bot_mode WHERE uid = %d "
      "AND net = '%s' AND chan = '%s' ORDER BY action",
      uid, network_buf, chan_buf);

   i = 0;

   while((row = db_fetch(result)))
   {
      mode[i] = row[0][0];
      i++;
   }

   mode[i] = 0;

   db_free(result);
}

int sql_mode_check(int uid, char *network, char *chan, int mode,
   char *param, int len)
{
   char chan_buf[64], network_buf[64];
   DB_RESULT result;
   DB_ROW row;

   if(chan)
      db_escape(chan_buf, chan, 32);
   else
      chan_buf[0] = 0;

   if(network)
      db_escape(network_buf, network, 32);
   else
      network_buf[0] = 0;

   result = db_query(0, "SELECT param FROM bot_mode WHERE uid = %d "
      "AND net = '%s' AND chan = '%s' AND action = '%c'", uid,
      network_buf, chan_buf, mode);
   row = db_fetch(result);

   if(db_rows(result) > 0)
   {
      if(param)
         strncpy(param, row[0], len);

      db_free(result);
      return 1;
   }
   else
   {
      db_free(result);
      return 0;
   }
}   

char *sql_mode_get_chan_list(char *network, int mode)
{
   char network_buf[64];
   static char list[4096];
   int len;
   DB_RESULT result;
   DB_ROW row;

   list[0] = 0;

   if(network == NULL)
      return list;

   db_escape(network_buf, network, 32);

   result = db_query(0, "SELECT chan FROM bot_mode WHERE net = '%s' "
      "AND action = '%c' AND uid = 0", network_buf, mode);

   if(db_rows(result) == 0)
   {
      db_free(result);
      return list;
   }

   len = 0;

   while((row = db_fetch(result)))
   {
      if((len + strlen(row[0])) > 4092)
         break;

      strcat(list, row[0]);
      len += strlen(row[0]);
      strcat(list, " ");
      len++;
   }         

   if(len)
      len--;

   list[len] = 0;

   db_free(result);
   return list;
}

int sql_mode_deluser(int uid)
{
   db_query(0, "DELETE FROM bot_mode WHERE uid = %d", uid);

   return 0;
}

#endif /* HAVE_SQL */
