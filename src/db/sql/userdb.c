/*
 * src/db/sql/userdb.c
 *
 * Copyright 2001 Colin O'Leary
 *
 * Gets user data from SQL database.
 *
 */

#include "mx3bot.h"

#ifdef HAVE_SQL

#include <crypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ban.h"
#include "misc.h"
#include "mode.h"
#include "note.h"
#include "mx3bot.h"
#include "sql.h"
#include "userdb.h"
#include "var.h"

int sql_userdb_add(char *name, char *mask)
{
   char name_buf[64], mask_buf[512];
   DB_RESULT result;
   int id;

   db_escape(name_buf, name, 32);
   db_escape(mask_buf, mask, 256);

   result = db_query(0, "INSERT INTO bot_user (name, level) "
      "VALUES('%s', 0)", name_buf);

   if((id = db_id(0, "bot_user", "id")))
   {
      db_query(0, "INSERT INTO bot_mask (uid, mask) "
         "VALUES(%d, '%s')", id, mask_buf);
   }

   db_free(result);

   return id;
}

int sql_userdb_del(int id)
{
   ban_deluser(id);
   mode_deluser(id);
   note_deluser(id);
   var_deluser(id);

   db_query(0, "DELETE FROM bot_mask WHERE uid = %d", id);
   db_query(0, "DELETE FROM bot_user WHERE uid = %d", id);

   return 0;
}

int sql_userdb_find_byname(char *name)
{
   DB_RESULT result;
   DB_ROW row;
   char name_buf[64];

   db_escape(name_buf, name, 32);

   result = db_query(0, "SELECT uid FROM bot_user WHERE name = '%s' ",
     name_buf);

   if(db_rows(result) < 1)
   {
      db_free(result);
      return 0;
   }

   row = db_fetch(result);

   db_free(result);
   return db_int(row[0]);
}

int sql_userdb_find_bymask3(char *nick, char *user, char *host)
{
   char ident[256];

   snprintf(ident, 256, "%s!%s@%s", nick, user, host);

   return userdb_find_bymask(ident);
}

int sql_userdb_find_bymask(char *ident)
{
   DB_RESULT result;
   DB_ROW row;

   result = db_query(0, "SELECT bot_user.uid, bot_mask.mask "
                        "FROM bot_user, bot_mask "
                        "WHERE bot_user.uid = bot_mask.uid ");

   if(db_rows(result) < 1)
   {
      db_free(result);
      return 0;
   }

   while((row = db_fetch(result)) != NULL)
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

char *sql_userdb_get_name(int id)
{
   DB_RESULT result;
   DB_ROW row;
   static char name[32];

   result = db_query(0, "SELECT name FROM bot_user WHERE uid = %d", id);

   if(db_rows(result) < 1)
   {
      db_free(result);
      return NULL;
   }

   row = db_fetch(result);
   strncpy(name, row[0], 32);

   db_free(result);
   return name;
}

int sql_userdb_set_name(int id, char *name)
{
   char name_buf[64];

   if(userdb_find_byname(name))
      return -1;

   db_escape(name_buf, name, 32);

   db_query(0, "UPDATE bot_user SET name = '%s' WHERE uid = %d",
      name_buf, id);

   return 0;
}

int sql_userdb_has_password(int id)
{
   DB_RESULT result;
   DB_ROW row;

   result = db_query(0, "SELECT pass FROM bot_user WHERE uid = %d", id);

   if(db_rows(result) < 1)
   {
      db_free(result); 
      return 0;
   }

   row = db_fetch(result);

   if(row == NULL || row[0] == NULL || row[0][0] == 0 || row[0][0] == '-')
   {
      db_free(result);
      return 0;
   }

   db_free(result);
   return 1;
}

int sql_userdb_check_password(int id, char *pass)
{
   DB_RESULT result;
   DB_ROW row;
   char hash[64];
   int ret;

   result = db_query(0, "SELECT pass FROM bot_user WHERE uid = %d", id);

   /* fail if user is not found */
   if(db_rows(result) < 1)
   {
      db_free(result);
      return 0;
   }

   row = db_fetch(result);

   /* succeed if no password is set */
   if(row == NULL || row[0] == 0 || row[0][0] == 0)
   {
      db_free(result);
      return 1;
   }

   /* password is set, but none was supplied: fail */
   if(pass == NULL || pass[0] == 0)
   {
      db_free(result);
      return 0;
   }

   strncpy(hash, crypt(pass, row[0]), 64);

   if(strcmp(hash, row[0]) == 0)
   {
      ret = 1;
   }
   else
   {
      ret = 0;
   }

   db_free(result);
   return ret;
}

int sql_userdb_set_password(int id, char *pass)
{
   char salt[] = "$1$--------";
   char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz./";
   char hash[64], hash_buf[128];
   int i;

   for(i = 0; i < 8; i++)
      salt[i + 3] = chars[rand() % 54];

   if(pass == NULL || *pass == 0)
   {
      hash_buf[0] = 0;
   }
   else
   {
      strncpy(hash, crypt(pass, salt), 64);
      db_escape(hash_buf, hash, 64);
   }

   db_query(0, "UPDATE bot_user SET pass = '%s' WHERE uid = %d",
      hash_buf, id);

   return 0;
}

int sql_userdb_addmask(int user, char *mask)
{
   DB_RESULT result;
   DB_ROW row;
   char mask_buf[512];

   db_escape(mask_buf, mask, 256);

   result = db_query(0, "SELECT COUNT(*) FROM bot_mask WHERE uid = %d "
      "AND mask = '%s'", user, mask_buf);

   row = db_fetch(result);

   if(db_int(row[0]) > 0)
   {
      db_free(result);
      return -1;
   }

   db_query(0, "INSERT INTO bot_mask (uid, mask) VALUES(%d, '%s')",
      user, mask_buf);
   db_free(result);

   return 0;
}

int sql_userdb_delmask(int user, char *mask)
{
   char mask_buf[512];

   db_escape(mask_buf, mask, 256);

   db_query(0, "DELETE FROM bot_mask WHERE uid = %d AND mask = '%s'",
      user, mask_buf);

   return 0;
}

char *sql_userdb_getmask(int mid)
{
   DB_RESULT result;
   DB_ROW row;
   static char mask[256];

   result = db_query(0, "SELECT mask FROM bot_mask WHERE mid = %d", mid);

   if(db_rows(result) < 1)
   {
      db_free(result);
      return NULL; 
   }

   row = db_fetch(result);
   strncpy(mask, row[0], 256);

   db_free(result);
   return mask;
}

char *sql_userdb_getmasklist(int uid)
{
   DB_RESULT result;
   DB_ROW row;
   static char list[1024];
   int len;

   list[0] = 0;
   len = 0;

   result = db_query(0, "SELECT mid FROM bot_mask WHERE uid = %d", uid);

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

int sql_userdb_count(void)
{
   DB_RESULT result;
   DB_ROW row;

   result = db_query(0, "SELECT COUNT(*) FROM bot_user");
   row = db_fetch(result);
   db_free(result);
   return db_int(row[0]);
}

#endif

