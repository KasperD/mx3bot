/*
 * src/db/sql/setup.c
 *
 * Copyright 2002 Colin O'Leary
 *
 * Sets up MySQL database.
 *
 */

#include "mx3bot.h"

#ifdef HAVE_MYSQL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "misc.h"
#include "mx3bot.h"
#include "setup.h"
#include "sql.h"

static int setup_create_user(const char *host, const char *user,
   const char *pass, const char *name);
static int setup_create_db(const char *host, const char *user,
   const char *pass, const char *name);
static int setup_create_tables(void);
static void setup_error(const char *str);

static int setup_err;

/*
 * setup_database() : Creates empty SQL tables.
 *
 * Returns negative if something was canceled by user or error, otherwise
 * zero.
 *
 */
int sql_setup_database(const char *dir, const char *dbhost,
   const char *dbuser, const char *dbpass, const char *dbname)
{
   int ret;
   DB_RESULT result;
   DB_ROW row;

   setup_err = 0;

   /* try connecting to server */
   ret = db_connect(0, dbhost, dbuser, dbpass, NULL, setup_error);

   if(ret == DB_ERR_INIT)
   {
      printf("Could not initialize database client library.\n");
      return -1;
   }

   if(ret == DB_ERR_CONNECT)
   {
      if(setup_create_user(dbhost, dbuser, dbpass, dbname) != 0)
         return -1;

      ret = db_connect(0, dbhost, dbuser, dbpass, NULL, setup_error);

      if(ret != 0)
      {
         printf("Could not connect to database server.\n\n");
         printf("Please manually add `%s' to the user table.\n", dbuser);
         return -1;
      }
   }

   /* try selecting database */
   ret = db_select(0, dbname);

   if(ret != 0)
   {
      if(setup_create_db(dbhost, dbuser, dbpass, dbname) != 0)
         return -1;
   }

   /* check for tables */
   result = db_query(0, "SELECT COUNT(*) FROM bot_user");

   if(setup_err)
   {
      if(setup_create_tables() != 0)
         return -1;
   }
   else
   {
      row = db_fetch(result);

      if(db_int(row[0]) > 0)
      {
         printf("It seems your bot is already setup!\n");
         printf("If you need to add an owner, use ");
         printf("mx3bot --setup-owner\n\n");
         return -1;
      }
   }

   return 0;
}

static int setup_create_user(const char *host, const char *user,
   const char *pass, const char *name)
{
   int ret, major, minor, patch;
   char root[256], user_buf[512], pass_buf[512], name_buf[512];
   char *p1, *p2, *p3;
   DB_RESULT result;
   DB_ROW row;

   printf("Setup will attempt to add this user to your database.\n\n");
   printf("This will only work if your bot resides on the same\n");
   printf("machines as your MySQL server.\n\n");
   printf("This involves connecting to your MySQL as root.\n\n");
   printf("Hit Ctrl-C if you wish to abort, or enter to confirm.\n");

   getchar();

   setup_err = 0;

   /* try to connect as root */
   ret = db_connect(0, host, "root", NULL, "mysql", setup_error);

   if(ret != 0)
   {
      printf("Your MySQL server has a root password set.\n");
      printf("Setup will use this to add a user to your database.\n\n");
      printf("Leave blank to abort.\n\n");

      system("stty -echo");
      text_input("Enter your MySQL root password", root, 256, NULL);
      system("stty echo");

      if(root[0] == 0)
         return -1;

      if(db_connect(0, host, "root", root, "mysql", setup_error) != 0)
      {
         printf("Setup could not connect to your database.\n\n");
         printf("Check in your config file that your database\n");
         printf("settings are correct.  Also try manually\n");
         printf("adding a MySQL user.\n");
         return -1;
      }
   }

   result = db_query(0, "SELECT VERSION()");
   row = db_fetch(result);

   p1 = strtok(row[0], ".-");
   p2 = strtok(NULL, ".-");
   p3 = strtok(NULL, ".-");

   if((p1 == NULL) || (p2 == NULL) || (p3 == NULL))
   {
      printf("Could not determine MySQL version.\n");
      return -1;
   }

   major = p1 ? atoi(p1) : 0;
   minor = p2 ? atoi(p2) : 0;
   patch = p3 ? atoi(p3) : 0;

   db_escape(user_buf, user, 256);
   db_escape(pass_buf, pass, 256);
   db_escape(name_buf, name, 256);

   if(((major >= 3) && (minor >= 23)) || (major > 3))
   {
      db_query(0, "GRANT ALL PRIVILEGES ON %s.* TO %s@localhost "
         "IDENTIFIED BY '%s' WITH GRANT OPTION", name_buf, user_buf,
         pass_buf);
   }
   else if((major >= 3) && (minor == 22) && (patch >= 11))
   {
      db_query(0, "INSERT INTO user (host, user, password) VALUES("
         "'localhost', '%s', PASSWORD('%s'))", user_buf, pass_buf);
      db_query(0, "INSERT INTO db VALUES('%%', '%s', '%s', 'Y', 'Y', "
         "'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y')", name_buf, user_buf);
      db_query(0, "FLUSH PRIVILEGES");
   }
   else
   {
      printf("Setup doesn't know how to deal with MySQL %d.%d.%d.\n\n",
         major, minor, patch);
      printf("You're on your own.\n");
      return -1;
   }

   db_disconnect(0);

   if(setup_err)
   {
      printf("Could not create a user.  You'll have to do it manually\n");
      return -1;
   }

   printf("Created user `%s' on `%s'.\n", user, host);

   return 0;
}

static int setup_create_db(const char *host, const char *user,
   const char *pass, const char *name)
{
   char name_buf[512];

   db_escape(name_buf, name, 256);
   db_query(0, "CREATE DATABASE %s", name_buf);

   if(setup_err)
   {
      printf("Could not create database.\n");
      return -1;
   }

   if(db_select(0, name) != 0)
   {
      printf("Could not select database.\n");
      return -1;
   }

   printf("Created database `%s'.\n", name);

   return 0;
}

static int setup_create_tables(void)
{
   setup_err = 0;

   /* evil awk voodoo:
      cat setup.mysql | awk 'BEGIN {ORS=""} /^.+$/' |
      awk 'BEGIN {RS=";"} {print "db_query(0, \"" $0 "\");"}'
    */
   #include "db/sql/schema.h"

   if(setup_err)
   {
      printf("Could not setup database tables.\n");
      return -1;
   }

   return 0;
}

static void setup_error(const char *msg)
{
   setup_err = 1;
}

#endif /* HAVE_MYSQL */
