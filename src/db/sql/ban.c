/*
 * src/db/sql/ban.c
 *
 * Copyright 2001 Colin O'Leary
 *
 * Implements functions to add/delete/check/get information on bans
 *   based on network, channel, and idents.
 *
 */

#include "mx3bot.h"

#ifdef HAVE_SQL

#include <stdio.h>
#include <string.h>

#include "ban.h"
#include "misc.h"
#include "sql.h"

char *sql_ban_get_list(char *network, char *chan, char *ident)
{
   static char list[1024];
   char sql[256], *mask;
   char network_buf[64], chan_buf[64];
   int len;
   DB_RESULT result;
   DB_ROW row;

   if(network)
      db_escape(network_buf, network, 32);
   if(chan)
      db_escape(chan_buf, chan, 32);

   if(ident)
      mask = ", mask ";
   else
      mask = "";

   if(network && chan)
      snprintf(sql, 256, "SELECT bid%s FROM bot_ban WHERE net = '%s' "
         "AND chan = '%s'", mask, network_buf, chan_buf);
   else if(network)
      snprintf(sql, 256, "SELECT bid%s FROM bot_ban WHERE net = '%s'",
         mask, network_buf);
   else if(chan)
      snprintf(sql, 256, "SELECT bid%s FROM bot_ban WHERE chan = '%s'",
         mask, chan_buf);
   else
      snprintf(sql, 256, "SELECT bid%s FROM bot_ban", mask);

   result = db_query(0, "%s", sql);

   list[0] = 0;
   len = 0;

   while((row = db_fetch(result)))
   {
      if((ident == NULL) || mask_match(row[1], ident))
      {
         char num[12];

         sprintf(num, "%d", db_int(row[0]));

         if(len + strlen(num) > 1020)
            break;

         strcat(list, num);
         len += strlen(num);
         strcat(list, " ");
         len++;
      }
   }

   if(len)
      len--;

   list[len] = 0;

   db_free(result);
   return list;
}

int sql_ban_get_id_bymask(char *network, char *chan, char *mask)
{
   char chan_buf[64];
   char network_buf[64];
   char mask_buf[512];
   DB_RESULT result;
   DB_ROW row;

   if(network == NULL || chan == NULL || mask == NULL)
      return 0;

   db_escape(network_buf, network, 32);
   db_escape(chan_buf, chan, 32);
   db_escape(mask_buf, mask, 256);

   result = db_query(0, "SELECT bid FROM bot_ban WHERE "
      "net = '%s' AND chan = '%s' AND mask = '%s'",
      network_buf, chan_buf, mask_buf);

   if(db_rows(result) < 1)
   {
      db_free(result); 
      return 0;
   }

   row = db_fetch(result);

   db_free(result);
   return db_int(row[0]);
}

int sql_ban_get_id(char *network, char *chan, char *ident)
{
   char chan_buf[64];
   char network_buf[64];
   DB_RESULT result;
   DB_ROW row;

   if(network == NULL || chan == NULL)
      return 0;

   db_escape(chan_buf, chan, 32);
   db_escape(network_buf, network, 32);

   result = db_query(0, "SELECT bid, mask FROM bot_ban WHERE "
      "net = '%s' AND chan = '%s'", network_buf, chan_buf);

   while((row = db_fetch(result)))
   {
      if(mask_match(row[1], ident))
      {
         db_free(result);
         return db_int(row[0]);
      }
   }

   db_free(result);
   return 0;
}

int sql_ban_add(char *network, char *chan, char *mask, int uid,
   char *note)
{
   char mask_buf[512], chan_buf[64], network_buf[64], note_buf[160];
   DB_RESULT result;
   DB_ROW row;
   int bid;

   if(network == NULL || chan == NULL)
      return 0;

   db_escape(mask_buf, mask, 256);
   db_escape(chan_buf, chan, 32);
   db_escape(network_buf, network, 32);

   if(note == NULL)
      note_buf[0] = 0;
   else
      db_escape(note_buf, note, 80);

   result = db_query(0, "SELECT bid FROM bot_ban WHERE mask = '%s' "
      "AND net = '%s' AND chan = '%s'", mask_buf, network_buf, chan_buf);

   if(db_rows(result) > 0)
   {
      row = db_fetch(result);
      bid = db_int(row[0]);

      db_query(0, "UPDATE bot_ban SET note = '%s', uid = %d "
         "WHERE bid = %d", note_buf, uid, bid);

      db_free(result);
      return bid;
   }

   db_query(0, "INSERT INTO bot_ban (mask, net, chan, uid, note) "
      "VALUES('%s', '%s', '%s', %d, '%s')", mask_buf, network_buf,
      chan_buf, uid, note_buf);

   bid = db_id(0, "bot_ban", "bid");

   db_free(result);
   return bid;
}

int sql_ban_del(int bid)
{
   db_query(0, "DELETE FROM bot_ban WHERE bid = %d", bid);

   return 0;
}

int sql_ban_get(int bid, int *uid, char *mask, int mask_len, char *note,
   int note_len)
{
   DB_RESULT result;
   DB_ROW row;

   result = db_query(0, "SELECT uid, mask, note FROM bot_ban WHERE "
      "bid = %d", bid);

   if(db_rows(result) < 1)
   {
      db_free(result);
      return -1;
   }

   row = db_fetch(result);

   if(uid)
      *uid = db_int(row[0]);
   if(mask)
      strncpy(mask, row[1], mask_len);
   if(note)
      strncpy(note, row[2], note_len);

   db_free(result);
   return 0;
}

int sql_ban_deluser(int uid)
{
   db_query(0, "DELETE FROM bot_ban WHERE uid = %d", uid);

   return 0;
}

#endif /* HAVE_SQL */
