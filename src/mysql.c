/*
 * src/mysql.c
 *
 * Copyright 2001-2002 Colin O'Leary
 *
 * Abstracts MySQL database calls.
 * 
 */  

#include "mx3bot.h"

#ifdef HAVE_MYSQL

#include "sql.h"

#include <mysql.h>
#include <errmsg.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct
{
   MYSQL *handle;
   void (*error_func)(const char *msg);
} db[16];

static int query_count, fetch_count;

static void handle_error(int i, const char *msg);

int db_connect(int i, const char *host, const char *user, const char *pass,
   const char *name, void (*func)(const char *msg))
{
   if(i == 0)
   {
      query_count = 0;
      fetch_count = 0;
   }

   db[i].handle = mysql_init(NULL);

   if(db[i].handle == NULL)
      return DB_ERR_INIT;

   db[i].handle = mysql_real_connect(db[i].handle, host, user, pass, NULL,
      0, NULL, 0);

   if(db[i].handle == NULL)
      return DB_ERR_CONNECT;

   if(name != NULL)
   {
      if(db_select(i, name) != 0)
         return DB_ERR_SELECT;
   }

   db[i].error_func = func;

   return 0;
}

int db_select(int i, const char *name)
{
   if(mysql_select_db(db[i].handle, name) != 0)
      return -1;

   return 0;
}

void db_disconnect(int i)
{
   mysql_close(db[i].handle);
}

MYSQL_RES *db_query(int i, const char *query, ...)
{
   static char buffer[1024];
   va_list ap;

   va_start(ap, query);
   vsnprintf(buffer, 1023, query, ap);
   va_end(ap);

   query_count++;
 
   if(mysql_query(db[i].handle, buffer) != 0)
   {
      int error = mysql_errno(db[i].handle);

      if(error == CR_SERVER_LOST)
      {
         error = db_connect(i, NULL, NULL, NULL, NULL, db[i].error_func);

         if(error != 0)
         {
            handle_error(i, "Lost connection to database.");
         }

         return db_query(i, "%s", buffer);
      }
      else
      {
         handle_error(i, mysql_error(db[i].handle));
      }
   }

   return mysql_store_result(db[i].handle);
}

MYSQL_ROW db_fetch(MYSQL_RES *result)
{
   fetch_count++;

   if(result)
      return mysql_fetch_row(result);
   else
      return NULL;
}

int db_rows(MYSQL_RES *result)
{
   if(result)
      return mysql_num_rows(result);
   else
      return 0;
}

void db_free(MYSQL_RES *result)
{
   mysql_free_result(result);
}

int db_int(const char *s)
{
   return atoi(s);
}

int db_escape(char *to, const char *from, int max)
{
   static char buffer[1024];

   mysql_escape_string(buffer, from, strlen(from));

   if(strlen(buffer) > max)
   {
      strncpy(to, buffer, max);
      to[max - 1] = 0;
      return 1;
   }
   else
   {
      strcpy(to, buffer);
      return 0;
   }
}

int db_id(int i, const char *table, const char *field)
{
   /* table, id for PostgreSQL compatability only */

   return mysql_insert_id(db[i].handle);
}

char *db_error(int i)
{
   return mysql_error(db[i].handle);
}

int db_affected(int i)
{
   return mysql_affected_rows(db[i].handle);
}

void db_stats(int *queries, int *fetches)
{
   if(queries)
      *queries = query_count;
   if(fetches)
      *fetches = fetch_count;
}

static void handle_error(int i, const char *msg)
{
   if(db[i].error_func)
   {
      db[i].error_func(msg);
   }
   else
   {
      fprintf(stderr, "db(%d): %s\n", i, msg);
   }
}

#endif /* HAVE_MYSQL */
