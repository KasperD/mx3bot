/*
 * src/db/sql/note.c
 *
 * Copyright 2001 Colin O'Leary
 *
 * Routines to send and receive notes from/to users.
 *
 */  

#include "mx3bot.h"

#ifdef HAVE_SQL

#include <stdio.h>
#include <string.h>

#include "mx3bot.h"
#include "note.h"
#include "sql.h"

char *sql_note_get_list(int to, int from)
{
   static char list[1024];
   int len;
   DB_RESULT result;
   DB_ROW row;

   list[0] = 0;
   len = 0;

   if(to != 0)
   {
      result = db_query(0, "SELECT nid FROM bot_note WHERE "
         "target = %d ORDER BY sent", to);
   }
   else if(from != 0)
   {
      result = db_query(0, "SELECT nid FROM bot_note WHERE "
         "author = %d ORDER BY sent", from);
   }
   else
   {
      return list;
   }

   if(db_rows(result) < 1)
   {
      db_free(result);
      return list;
   }

   while((row = db_fetch(result)))
   {
      char num[12];

      sprintf(num, "%d", db_int(row[0]));

      if(len + strlen(num) >= 1020)
         break;

      strcat(list, num);
      len += strlen(num);
      strcat(list, " ");
      len++;
   }

   if(len)
      len--;

   list[len] = 0;

   db_free(result);
   return list;
}

int sql_note_get(int nid, int *to, int *from, int *sent, char *note,
   int len)
{
   DB_RESULT result;
   DB_ROW row;

   result = db_query(0, "SELECT author, target, sent, note "
      "FROM bot_note WHERE nid = %d", nid);

   if(db_rows(result) < 1)
   {
      db_free(result);
      return 0;
   }

   row = db_fetch(result);

   if(from)
      *from = db_int(row[0]);
   if(to)
      *to = db_int(row[1]);
   if(sent)
      *sent = db_int(row[2]);
   if(note)
   {
      strncpy(note, row[3], len);
      note[len - 1] = 0;
   }

   db_free(result);
   return 1;
}

int sql_note_del(int nid)
{
   db_query(0, "DELETE FROM bot_note WHERE nid = %d", nid);

   return 0;
}

int sql_note_add(int to, int from, int sent, char *note)
{
   char note_buf[512];

   if(note == NULL || note[0] == 0)
      return -1;

   db_escape(note_buf, note, 256);
   
   db_query(0, "INSERT INTO bot_note (author, target, sent, note) "
      "VALUES(%d, %d, %d, '%s')", from, to, sent, note_buf);

   return 0;
}

int sql_note_deluser(int uid)
{
   db_query(0, "DELETE FROM bot_note WHERE author = %d", uid);
   db_query(0, "DELETE FROM bot_note WHERE target = %d", uid);

   return 0;
}

#endif /* HAVE_SQL */
