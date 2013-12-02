/*
 * src/db/sql/setup.c
 *
 * Copyright 2002 Colin O'Leary
 *
 * User friendly installation.
 *
 */

#include "mx3bot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "db.h"
#include "misc.h"
#include "mode.h"
#include "setup.h"
#include "userdb.h"
#include "var.h"

static int setup_make_config(int type, const char *dir, const char *config,
   const char *dbhost, const char *dbuser, const char *dbpass,
   const char *dbname);

/*
 * setup_all() : Tries to do everything possible to setup database and
 *               config files for user.
 *
 * Returns negative if something was canceled by user or on error,
 * otherwise zero.
 *
 */
int setup_all(void)
{
   char dbhost[256], dbuser[256], dbpass[256], dbname[256], dbtype[64];
   char *home, mx3bot[512], config[512], temp[512], fifo[512], log[512];
   int typeno;

   printf("Mx3bot " VERSION " guided setup\n\n");

   printf("Mx3bot guided setup will attempt to create the neccesary\n");
   printf("directories and files needed for operation of the bot.\n\n");
   printf("A .mx3bot directory will be created in your home directory,\n");
   printf("and it will be filled with various configuration and special\n");
   printf("files.\n\n");

#ifdef HAVE_MYSQL
   printf("Setup will also try to connect to a MySQL server for you\n");
   printf("and create the needed tables (and user, if wanted).\n\n");

   printf("You have MySQL and text database support compiled.  Would you\n");
   printf("like to setup a MySQL Mx3bot installation or a text database\n");
   printf("installation?  Enter \"text\" or \"mysql\"\n\n");

   typeno = -1;

   while(typeno == -1)
   {
      text_input("Database type", dbtype, 64, "text");   

      if(strcmp(dbtype, "text") == 0)
         typeno = DBTYPE_TEXT;
      else if(strcmp(dbtype, "mysql") == 0)
         typeno = DBTYPE_MYSQL;
   }      
#else
   printf("To continue, press enter.  Otherwise hit Ctrl-C.\n");
   getchar();

   strcpy(dbtype, "text");
   typeno = DBTYPE_TEXT;
#endif

   /* look for ~/.mx3bot/ */
   home = getenv("HOME");

   if(home == NULL)
   {
      printf("Could not determine home directory.\n");
      printf("Please ensure that the HOME environment variable is set.\n");
      printf("\n");
      return -1;
   }

   snprintf(mx3bot, 512, "%s/.mx3bot", home);

   /* create ~/.mx3bot */
   if(access(mx3bot, R_OK) != 0)
   {
      if(mkdir(mx3bot, 0755) != 0)
      {
         printf("Could not create `%s'\n", mx3bot);
         perror("   ");
         return -1;
      }
      else
      {
         printf("Created directory `%s'.\n", mx3bot);
      }
   }
   else
   {
      printf("Mx3bot directory `%s' already exists.\n", mx3bot);
   }

   /* make temp directory */
   snprintf(temp, 512, "%s/temp", mx3bot);

   if(access(temp, R_OK) == 0)
   {
      printf("Temporary directory `%s' already exists.\n", temp);
   }
   else
   {
      if(mkdir(temp, 0755) != 0)
      {
         printf("Could not create `%s'.\n", temp);
         perror("   ");
         return -1;
      }
      else
      {
         printf("Created temporary directory `%s'.\n", temp);
      }
   }

   /* make log directory */
   snprintf(log, 512, "%s/log", mx3bot);

   if(access(log, R_OK) == 0)
   {
      printf("Log directory `%s' already exists.\n", log);
   }
   else
   {
      if(mkdir(log, 0755) != 0)
      {
         printf("Could not create `%s'.\n", log);
         perror("   ");
         return -1;
      }
      else
      {
         printf("Created log directory `%s'.\n", log);
      }
   }

   /* make server fifo */
   snprintf(fifo, 512, "%s/fifo", mx3bot);

   if(access(fifo, R_OK) == 0)
   {
      printf("Fifo `%s' already exists.\n", fifo);
   }
   else
   {
      if(mkfifo(fifo, 0600) != 0)
      {
         printf("Could not create fifo `%s'.\n", fifo);
         perror("   ");
         return -1;
      }
      else
      {
         printf("Created fifo `%s'.\n", fifo);
      }
   }

   /* write config */
   snprintf(config, 512, "%s/config", mx3bot);

   if(access(config, R_OK) == 0)
   {
      printf("Mx3bot config `%s' already exists.\n", config);

      if(typeno == DBTYPE_MYSQL)
      {
         config_get_str(config, "database", "host", dbhost, 256);
         config_get_str(config, "database", "user", dbuser, 256);
         config_get_str(config, "database", "pass", dbpass, 256);
         config_get_str(config, "database", "name", dbname, 256);
      }
   }
   else
   {
      /* get database info */
      if(typeno == DBTYPE_MYSQL)
      {
         putchar('\n');
         text_input("Database host", dbhost, 256, "localhost");
         text_input("Database user", dbuser, 256, "mx3bot");
         text_input("Database password", dbpass, 256, NULL);
         text_input("Database name", dbname, 256, "mx3bot");
      }

      if(setup_make_config(typeno, mx3bot, config, dbhost, dbuser, dbpass,
         dbname) != 0)
         return -1;
   }

   if((typeno == DBTYPE_MYSQL) &&
      ((dbhost[0] == 0) || (dbuser[0] == 0) || (dbname[0] == 0)))
   {
      printf("Database parameters were not set in config.\n");
      printf("Please fix your configuration file, or remove it\n");
      printf("  and rerun this setup.\n\n");
      return -1;
   }

   db_set_type(typeno);

   if(setup_database(mx3bot, dbhost, dbuser, dbpass, dbname) != 0)
      return -1;

   /* setup_database() must connect to/setup database */

   /* add default variables */
   printf("Adding default variables...");
   fflush(stdout);

   var_set(0, "default-umode", "n");
   var_set(0, "version", "http://bot.mx3.org/");
   var_set(0, "userinfo", "Mx3bot");
   var_set(0, "msg:sigint", "Interrupted");
   var_set(0, "msg:sighup", "Reconnecting");
   var_set(0, "msg:sigterm", "Terminated");
   var_set(0, "msg:dberror", "Lost connection to database");
   printf("done.\n");

   if(setup_add_owner() != 0)
      return -1;

   printf("Setup is complete.  Double check the values set in the \n");
   printf("config file (%s).  You will probably \n", config);
   printf("want to change at least the IRC server and nick.\n\n");
   printf("To add another owner, run `mx3bot --setup-owner'.\n\n");
   printf("Simply running `mx3bot' should now start your bot.\n\n");

   return 0;
}

/*
 * setup_add_owner() : Adds an owner to the user database.  The database
 *                     should already be connected to as handle 0.
 *
 * Returns negative if user canceled or on error, otherwise zero.
 *
 */
int setup_add_owner(void)
{
   char name[32], mask[256], pass[32];
   int id;

   putchar('\n');
   printf("This will add an owner to your database.\n\n");
   printf("It is important for security that you use the most specific\n");
   printf("mask possible.\n\n");
   printf("It is highly recommended that you set a password for this\n");
   printf("user.  Users with passwords will have to authenticate with\n");
   printf("the bot using the `auth' command before using any privileged\n");
   printf("commands.\n\n");

   text_input("Handle", name, 32, NULL);
   text_input("Mask", mask, 256, NULL);

   system("stty -echo");
   text_input("Password", pass, 32, NULL);
   system("stty echo");

   printf("\n\n");

   if((name[0] == 0) || (mask[0] == 0))
   {
      printf("Canceled.\n");
      return -1;
   }

   if(userdb_find_byname(name))
   {
      printf("User `%s' already exists.\n", name);
   }
   else
   {
      id = userdb_add(name, mask);

      if(id == 0)
      {
         printf("Could not add bot user `%s' to database.\n", name);
         return -1;
      }

      printf("Added owner `%s' with mask `%s'.\n", name, mask);

      mode_set(id, NULL, NULL, "+w", NULL);

      if(pass[0] != 0)
      {
         userdb_set_password(id, pass);
         printf("Set password.\n");
      }
   }

   return 0;
}

static int setup_make_config(int type, const char *dir, const char *filename,
   const char *host, const char *user, const char *pass, const char *name)
{
   FILE *cfg;

   cfg = fopen(filename, "w");

   if(cfg == NULL)
   {
      printf("Could not open `%s' for writing.\n", filename);
      perror("   ");
      return -1;
   }

   fprintf(cfg, "# Mx3bot Configuration\n");
   fprintf(cfg, "# Generated %s\n\n\n", timestr(time(0)));

   fprintf(cfg, "[database]\n");

   if(type == DBTYPE_MYSQL)
   {
      fprintf(cfg, "type\t\tmysql\n");
      fprintf(cfg, "host\t\t%s\n", host);
      fprintf(cfg, "name\t\t%s\n", name);
      fprintf(cfg, "user\t\t%s\n", user);
      fprintf(cfg, "pass\t\t%s\n\n\n", pass);
   }
   else
   {
      fprintf(cfg, "type\t\ttext\n");
      fprintf(cfg, "path\t\t%s\n\n\n", dir);
   }

   fprintf(cfg, "[settings]\n\n");

   fprintf(cfg, "# nick\n");
   fprintf(cfg, "#   This is the nick that the bot will attempt\n");
   fprintf(cfg, "#   to use upon connecting.\n");
   fprintf(cfg, "nick\t\tMx3bot\n\n");

   fprintf(cfg, "# network\n");
   fprintf(cfg, "#   A name for the default network to connect to.\n");
   fprintf(cfg, "#   This will be used as a key in the database,\n");
   fprintf(cfg, "#   so changing it after adding channels and modes\n");
   fprintf(cfg, "#   will cause loss of settings.\n");
   fprintf(cfg, "network\t\tdefault\n\n");

   fprintf(cfg, "# server\n");
   fprintf(cfg, "#   Server to connect to.  Don't include the port.\n");
   fprintf(cfg, "server\t\tirc.gamesnet.net\n\n");

   fprintf(cfg, "# port\n");
   fprintf(cfg, "#   This must be specified.\n");
   fprintf(cfg, "port\t\t6667\n\n");

   fprintf(cfg, "# fifo\n");
   fprintf(cfg, "#   The \"well-known\" fifo that child processes\n");
   fprintf(cfg, "#   will use to communicate with the parent process.\n");
   fprintf(cfg, "fifo\t\t%s/fifo\n\n", dir);

   fprintf(cfg, "# temp\n");
   fprintf(cfg, "#   The temporary directory used for child process\n");
   fprintf(cfg, "#   fifos.\n");
   fprintf(cfg, "temp\t\t%s/temp\n\n", dir);

   fprintf(cfg, "# scripts\n");
   fprintf(cfg, "#   The directory in which Tcl scripts are located.\n");
   fprintf(cfg, "scripts\t\t" CMDDIR "\n\n");

   fprintf(cfg, "# logpath\n");
   fprintf(cfg, "#   Directory in which to store logs.\n");
   fprintf(cfg, "logpath\t\t%s/log\n\n", dir);

   fprintf(cfg, "# prefix\n");
   fprintf(cfg, "#   Prefix for commands in public channels.\n");
   fprintf(cfg, "prefix\t\t!\n\n");

   fprintf(cfg, "# purge-delay\n");
   fprintf(cfg, "#   Time to wait between losing sight of a user and\n");
   fprintf(cfg, "#   removing him from the user table.  Keep this high\n");
   fprintf(cfg, "#   to make it easier to use nick based commands and\n");
   fprintf(cfg, "#   avoid having to re-authenticate.  Keep it lower to\n");
   fprintf(cfg, "#   make password authentication more secure.  Time is\n");
   fprintf(cfg, "#   in seconds.\n");
   fprintf(cfg, "purge-delay\t60\n\n");

   fprintf(cfg, "# background\n");
   fprintf(cfg, "#   If this is true, the bot will go into daemon mode\n");
   fprintf(cfg, "#   upon starting.\n");
   fprintf(cfg, "background\tfalse\n\n");

   fprintf(cfg, "# debug\n");
   fprintf(cfg, "#   Enables debug mode, which will print all kinds of\n");
   fprintf(cfg, "#   debug to the logs and screen (if not in daemon\n");
   fprintf(cfg, "#   mode.\n");
   fprintf(cfg, "debug\t\ttrue\n\n");

   fclose(cfg);

   return 0;
}
