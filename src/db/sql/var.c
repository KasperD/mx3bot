/*
 * src/db/sql/var.c
 *
 * Copyright 2001 Colin O'Leary
 *
 * Get/set variables.
 *
 */

#include "mx3bot.h"

#ifdef HAVE_SQL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"
#include "sql.h"
#include "var.h"

int sql_var_set(int uid, char *name, char *val)
{
   char name_buf[64], val_buf[1024];
   DB_RESULT result;
   DB_ROW row;

   if(name == NULL || name[0] == 0)
      return 1;

   db_escape(name_buf, name, 32);

   if(val == NULL || val[0] == 0)
   {
      result = db_query(0, "DELETE FROM bot_var WHERE uid = %d AND "
         "name = '%s'", uid, name_buf);
   }
   else
   {
      db_escape(val_buf, val, 512);

      result = db_query(0, "SELECT COUNT(*) FROM bot_var WHERE uid = %d "
         "AND name = '%s'", uid, name_buf);
      row = db_fetch(result);

      if(db_int(row[0]) < 1)
      {
         db_query(0, "INSERT INTO bot_var (uid, name, val) "
            "VALUES(%d, '%s', '%s')", uid, name_buf, val_buf);
      }
      else
      {
         db_query(0, "UPDATE bot_var SET val = '%s', name = '%s' "
            "WHERE uid = %d AND name = '%s'", val_buf, name_buf, uid,
            name_buf);
      }
   }

   db_free(result);
   return 0;
}

int sql_var_get(int uid, char *name, char *val, int len)
{
   char name_buf[64];
   DB_RESULT result;
   DB_ROW row;

   if(name == NULL || name[0] == 0)
      return 1;

   db_escape(name_buf, name, 32);

   result = db_query(0, "SELECT val FROM bot_var WHERE uid = %d AND "
      "name = '%s'", uid, name_buf);

   if(db_rows(result) < 1)
   {
      db_free(result);
      return 1;
   }

   row = db_fetch(result);
   strncpy(val, row[0], len);

   db_free(result);
   return 0;
}

int sql_var_get_def(int uid, char *name, char *val, int len, char *def)
{
   if(var_get(uid, name, val, len) != 0)
      strncpy(val, def, len);

   return 0;
}

int sql_var_set_unique(int uid, char *val)
{
   DB_RESULT result;
   DB_ROW row;
   int id;
   char name[64];

   do
   {
      id = rand();
      if(id == 0)
         continue;

      result = db_query(0, "SELECT COUNT(*) FROM bot_var WHERE uid = %d "
         "AND name = '%d'", uid, id);
      row = db_fetch(result);

      if(db_int(row[0]) > 0)
         id = 0;
   }
   while(id == 0);

   db_free(result);

   sprintf(name, "%d", id);

   if(var_set(uid, name, val) == 1)
      return 0;
   else
      return id;
}

int sql_var_deluser(int uid)
{
   db_query(0, "DELETE FROM bot_var WHERE uid = %d", uid);

   return 0;
}

#endif
