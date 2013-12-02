/*
 * src/main.c
 *
 * Copyright 2001-2002 Colin O'Leary
 *
 * Main function, handles all command-line arguments, reads from
 *    config file, and starts a new connection.
 *
 */  
     
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "config.h"
#include "connection.h"
#include "db.h"
#include "log.h"
#include "misc.h"
#include "mx3bot.h"
#include "script.h"
#include "setup.h"
#include "user.h"
#include "userdb.h"
#include "var.h"

static void usage(FILE *output);
static void version(FILE *output);
static int background(void);

#define CONFIG_SETUP	1000
#define CONFIG_OWNER	1001
#define CONFIG_MYSQL	1002
#define CONFIG_TEXT	1003
#define CONFIG_NODEBUG	1004
#define CONFIG_NODAEMON	1005

#define SETUP_NONE	0
#define SETUP_ALL	1
#define SETUP_OWNER	2

static struct option long_options[] = 
{
   { "nick", 1, 0, 'n' },
   { "server", 1, 0, 's' },
   { "network", 1, 0, 'e' },
   { "debug", 0, 0, 'd' },
   { "no-debug", 0, 0, CONFIG_NODEBUG },
   { "port", 1, 0, 'p' },
   { "username", 1, 0, 'u' },
   { "realname", 1, 0, 'r' },
   { "command-prefix", 1, 0, 'c' },
   { "config-test", 1, 0, 'T' },
   { "dbhost", 1, 0, 'H' },
   { "dbuser", 1, 0, 'U' },
   { "dbname", 1, 0, 'N' },
   { "dbpass", 1, 0, 'P' },
   { "config-file", 1, 0, 'C' },
   { "script-path", 1, 0, 'S' },
   { "purge-delay", 1, 0, 'D' },
   { "help", 0, 0, 'h' },
   { "version", 0, 0, 'v' },
   { "fifo", 1, 0, 'f' },
   { "logpath", 1, 0, 'l' },
   { "background", 0, 0, 'b' },
   { "daemon", 0, 0, 'b' },
   { "no-background", 0, 0, CONFIG_NODAEMON },
   { "no-daemon", 0, 0, CONFIG_NODAEMON },
   { "setup", 0, 0, CONFIG_SETUP },
   { "create-owner", 0, 0, CONFIG_OWNER },
   { "setup-owner", 0, 0, CONFIG_OWNER },
#ifdef HAVE_MYSQL
   { "mysql", 0, 0, CONFIG_MYSQL },
#endif
   { "text", 0, 0, CONFIG_TEXT },
   { 0, 0, 0, 0 }
};

/* global variables */
int purge_delay;
char default_username[32], default_realname[80];

static char error_msg[256];

void db_error_handler(const char *msg)
{
   log_error("database: %s", msg);
   connection_shutdown(error_msg);
}

int main(int argc, char *argv[])
{
   char nick[32], network[32], server[128], username[32], realname[64],
      dbhost[64], dbname[64], dbuser[64], dbpass[64], dbpath[256],
      dbtype[32], config[256], script_path[256], prefix[4], fifo[256],
      logpath[256];
   int debug, port, config_test, daemon, setup_type, c, typeno;
   unsigned int logtype;
   struct timeval tv;

   /* start with all values empty */
   nick[0] = 0;
   network[0] = 0;
   server[0] = 0;
   username[0] = 0;
   realname[0] = 0;
   dbhost[0] = 0;
   dbname[0] = 0;
   dbuser[0] = 0;
   dbpass[0] = 0;
   dbpath[0] = 0;
   dbtype[0] = 0;
   config[0] = 0;
   script_path[0] = 0;
   prefix[0] = 0;
   fifo[0] = 0;
   logpath[0] = 0;
   debug = -1;
   port = -1;
   config_test = -1;
   purge_delay = -1;
   daemon = -1;
   setup_type = SETUP_NONE;

   /* check command line */
   c = 0;
   while(c != -1)
   {
      c = getopt_long(argc, argv, "n:s:dp:c:TH:U:P:N:D:hvf:e:wC:l:S:b",
         long_options, NULL);

      switch(c)
      {
         case 'n':
            strncpy(nick, optarg, 32);
            break;
         case 's':
            strncpy(server, optarg, 128);
            break;
         case 'e':
            strncpy(network, optarg, 32);
            break;
         case 'p':
            port = atoi(optarg);
            break;
         case 'd':
            debug = 1;
            break;
         case CONFIG_NODEBUG:
            debug = 0;
            break;
         case 'r':
            strncpy(realname, optarg, 64);
            break;
         case 'u':
            strncpy(username, optarg, 32);
            break;
         case 'c':
            strncpy(prefix, optarg, 4);
            break;
         case 'T':
            config_test = 1;
            break;
         case 'H':
            strncpy(dbhost, optarg, 64);
            break;
         case 'U':
            strncpy(dbuser, optarg, 64);
            break;
         case 'P':
            strncpy(dbpass, optarg, 64);
            break;
         case 'N':
            strncpy(dbname, optarg, 64);
            break;
         case 'S':
            strncpy(script_path, optarg, 256);
            break;
         case 'D':
            purge_delay = atoi(optarg);
            break;
         case 'h':
            usage(stdout);
            return 0;
         case 'v':
            version(stdout);
            return 0;
         case 'f':
            strncpy(fifo, optarg, 256);
            break;
         case 'C':
            strncpy(config, optarg, 256);
            break;
         case 'l':
            strncpy(logpath, optarg, 256);
            break;
         case 'b':
            daemon = 1;
            break;
         case CONFIG_NODAEMON:
            daemon = 0;
            break;
         case CONFIG_OWNER:
            setup_type = SETUP_OWNER;
            break;
         case CONFIG_SETUP:
            setup_type = SETUP_ALL;
            break;
         case CONFIG_TEXT:
            strcpy(dbtype, "text");
            break;
#ifdef HAVE_MYSQL
         case CONFIG_MYSQL:
            strcpy(dbtype, "mysql");
            break;
#endif
         case '?':
            fprintf(stderr, "Use `mx3bot --help' for help.\n");
            return 1;
            break;
      }
   }

   /* find config file if not specified */
   if(config[0] == 0)
   {
      char *home, *path;

      /* check MX3BOT_CONFIG */
      path = getenv("MX3BOT_CONFIG");
      if(path != NULL)
      {
         strncpy(config, path, 256);
         config[255] = 0;

         if(access(config, R_OK) == 0)
            goto found_config;
      }

      /* check ~/.mx3bot/config */
      home = getenv("HOME");
      if(home != NULL)
      {
         strncpy(config, home, 240);
         config[239] = 0;

         strcat(config, "/.mx3bot/config");

         if(access(config, R_OK) == 0)
            goto found_config;
      }

      /* check ./mx3bot.conf */
      strcpy(config, "./mx3bot.conf");
      if(access(config, R_OK) == 0)
         goto found_config;

      /* check /etc/mx3bot.conf */
      strcpy(config, "/etc/mx3bot.conf");
      if(access(config, R_OK) == 0)
         goto found_config;

      /* not found: assume user screwed up */
      if(setup_type != SETUP_ALL)
      {
         fprintf(stderr, "Could not find config file.\n");
         fprintf(stderr, "Try mx3bot -C - to use no config file.\n");
         fprintf(stderr, "Try mx3bot --setup to create one.\n");
         return 1;
      }
   }

   found_config:

   /* make sure config file is accessible */
   if((strcmp(config, "-") != 0) &&
      (access(config, R_OK) != 0))
   {
      fprintf(stderr, "Could not read config file `%s'.\n", config);
      return 1;
   }

   /* set dummy log support for log_debug/log_warning on init */
   logtype = LOGTYPE_WARNING | LOGTYPE_FATAL | LOGTYPE_LOGMSG |
             LOGTYPE_STDOUT;

   if(debug)
      logtype |= LOGTYPE_DEBUG;

   log_init(-1, -1);
   log_set(-1, NULL, NULL, logtype);

   /* run setup if wanted */
   if(setup_type == SETUP_ALL)
   {
      setup_all();
      return 0;
   }

   /* check config file for unset variables if using one */
   if(strcmp(config, "-") != 0)
   {
      #define CFG_STR(sect, name, var, size)			\
         if(var[0] == 0)					\
            config_get_str(config, sect, name, var, size);
      #define CFG_INT(sect, name, var)				\
         if(var == -1)						\
            config_get_int(config, sect, name, &var); 
      #define CFG_BOOL(sect, name, var)				\
         if(var == -1)						\
            config_get_bool(config, sect, name, &var);

      CFG_STR("settings", "nick", nick, 32);
      CFG_STR("settings", "server", server, 128);
      CFG_STR("settings", "network", network, 32);
      CFG_INT("settings", "port", port);
      CFG_STR("settings", "username", username, 32);
      CFG_STR("settings", "realname", realname, 64);
      CFG_STR("settings", "scripts", script_path, 256);
      CFG_STR("settings", "prefix", prefix, 4);
      CFG_INT("settings", "purge-delay", purge_delay);
      CFG_STR("settings", "fifo", fifo, 256);
      CFG_STR("settings", "logpath", logpath, 256);

      CFG_BOOL("settings", "background", daemon);
      CFG_BOOL("settings", "debug", debug);

      CFG_STR("database", "type", dbtype, 32);
      CFG_STR("database", "host", dbhost, 64);
      CFG_STR("database", "user", dbuser, 64);
      CFG_STR("database", "pass", dbpass, 64);
      CFG_STR("database", "name", dbname, 64);
      CFG_STR("database", "path", dbpath, 256);
   }

   /* use default values for unset variables */
   #define DEF_STR(var, val)					\
      if(var[0] == 0)						\
         strcpy(var, val);
   #define DEF_INT(var, val)					\
      if(var == -1)						\
         var = val;

   DEF_STR(nick, "mx3bot");
   DEF_STR(server, "localhost");
   DEF_STR(network, "default");
   DEF_INT(port, 6667);
   DEF_STR(username, "mx3bot");
   DEF_STR(realname, "Mx3bot " VERSION);
   DEF_STR(script_path, "cmd");
   DEF_STR(prefix, "!");
   DEF_INT(purge_delay, 120);
   DEF_STR(fifo, "./mx3bot.fifo");
   DEF_STR(logpath, "log");

   DEF_STR(dbtype, "text");
   DEF_STR(dbhost, "localhost");
   DEF_STR(dbuser, "mx3bot");
   DEF_STR(dbpass, "");
   DEF_STR(dbname, "mx3bot");
   DEF_STR(dbpath, ".");

   DEF_INT(debug, 0);
   DEF_INT(daemon, 0);

   if(config_test == 1)
   {
      printf("Config:           %s\n", config);
      printf("Database:\n");

      printf("   Type:          %s\n", dbtype);
      printf("   Path:          %s\n", dbpath);
      printf("   Host:          %s\n", dbhost);
      printf("   User:          %s\n", dbuser);
      printf("   Password:      %s\n", dbpass);
      printf("   Name:          %s\n", dbname);

      printf("Script Directory: %s\n", script_path);
      printf("Nick:             %s\n", nick);
      printf("Server:           %s:%d\n", server, port);
      printf("Username:         %s\n", username);
      printf("Realname:         %s\n", realname);
      printf("Command Prefix:   %c\n", prefix[0]);
      printf("Purge Delay:      %d seconds\n", purge_delay);
      printf("Fifo:             %s\n", fifo);
      printf("Log Path:         %s\n", logpath);
      printf("Debug mode:       %s\n", config_bool_str(debug));
      printf("Background mode:  %s\n", config_bool_str(daemon));
      return 0;
   }

   /* half-assed random seed */
   gettimeofday(&tv, NULL);   
   srand(tv.tv_sec ^ tv.tv_usec ^ getpid());

   /* logfile initialization */
   if(log_init(0, 0) < 0)
   {
      fprintf(stderr, "Could not initialize logfile table "
         "(Out of memory).");
      return -1;
   }

   /* set logfile path */
   if(logpath[0] != 0)
      log_setpath(logpath);

   /* default global logfile type */
   logtype = LOGTYPE_WARNING | LOGTYPE_FATAL | LOGTYPE_LOGMSG | 
             LOGTYPE_TIMESTAMP;

   /* debug mode */
   if(debug)
      logtype |= LOGTYPE_DEBUG;

   /* don't bother printing to screen if running as daemon */
   if(daemon == 0)
      logtype |= LOGTYPE_STDOUT;

   /* set global logfile */
   log_set(-1, NULL, "mx3bot.log", logtype);
   log_message("*** Logfile started %s ***", timestr(time(0)));

#ifdef HAVE_MYSQL
   if(strcasecmp(dbtype, "mysql") == 0)
   {
      log_debug("Connecting to database `%s' on `%s' as `%s'.",
         dbname, dbhost, dbuser);
      typeno = DBTYPE_MYSQL;
   }
   else
#endif
   if(strcasecmp(dbtype, "text") == 0)
   {
      log_debug("Reading database from `%s'.", dbpath);
      typeno = DBTYPE_TEXT;
   }
   else
   {
      log_error("Invalid database type `%s'.", dbtype);
      return 1;
   }

   if(db_init(typeno, dbpath, dbhost, dbuser, dbpass, dbname,
      db_error_handler) != 0)
   {
      return 1;
   }

   var_get_def(0, "msg:dberror", error_msg, 256, "Database error");

   script_init(script_path);

   if(user_init(0, 0) < 0)
   {
      log_error("Could not initialize user table (Out of memory).");
      return -1;
   }

   if(connection_init(fifo) < 0)
   {
      log_error("Could not open fifo `%s'.", fifo);
      return -1;
   }

   strncpy(default_username, username, 32);
   strncpy(default_realname, realname, 80);

   /* now that database is connected, try to add owner */
   if(setup_type == SETUP_OWNER)
   {
      setup_add_owner();
      return 0;
   }

   if(daemon == 1)
   {
      if(background() < 0)
      {
         log_error("Could not create daemon process.");
         return -1;
      }
   }

   connection_open(network, server, port, nick, username, realname,
      prefix[0]);

   while(connection_server(100) == 0)
      ;

   log_message("Closing logfile on %s.", timestr(time(0)));
   log_message("");
   log_set(-1, NULL, NULL, 0);

   return 0;
}

static void usage(FILE *out)
{
   fprintf(out, 

   "Mx3bot " VERSION "\n"
   "Usage: mx3bot [options]\n"
   "   [options] consists of the following:\n"
   "\n"
   " -n <nick>              Use specified nickname\n"
   "  --nick\n"
   " -s <server>            Connect to this server\n"
   "  --server\n"
   " -d                     Enable debug messages\n"
   "  --debug (or --no-debug)\n"
   " -p <port>              Connect to this port on server\n"
   "  --port\n"
   " -u <username>          Use this username on registering\n"
   "  --username\n"
   " -r <realname>          Use this realname on registering\n"
   "  --realname\n"
   " -c <prefix>            Prefix for bot commands\n"
   "  --command-prefix\n"
   " -T                     Configuration test: run and then exit\n"
   "  --config-test\n"
#ifdef HAVE_SQL
   " -H <hostname>          Database host\n"
   "  --dbhost\n"
   " -U <username>          Database user\n"
   "  --dbuser\n"
   " -N <name>              Database name\n"
   "  --dbname\n"
   " -P <password>          Database password\n"
   "  --dbpass\n"
#endif
   " -C <path>              Configuration file to use (use `-' for none)\n"
   "  --config-file\n"
   " -S <path>              Path in which to run scripts\n"
   "  --script-path\n"
   " -D <duration>          Time to keep unseen users cached (in seconds)\n"
   "  --purge-delay\n"
   " -h                     Display this help information\n"
   "  --help\n"
   " -v                     Display version information and exit\n"
   "  --version\n"
   " -f                     Path to fifo on which to listen\n"
   "  --fifo\n"
   " -l                     Path to prefix to logfiles\n"
   "  --logpath\n"
   " -b                     Go into background (daemon mode)\n"
   "  --background (or --no-background)\n"
   "  --daemon (or --no-daemon)\n"
   " --setup                Perform initial setup\n"
   " --setup-owner          Add an owner to the bot\n"
#ifdef HAVE_SQL
   " --mysql                Use MySQL user database\n"
#endif
   " --text                 Use text user database\n"
   "\n");
}

static void version(FILE *out)
{
   fprintf(out,

   "Mx3bot " VERSION " (" __DATE__ " " __TIME__ ")\n"
   "Copyright 2001-2002 Colin O'Leary\n"
   "See http://bot.mx3.org/ for more information.\n"
   "\n");
}

static int background(void)
{
   pid_t pid;

   pid = fork();

   if(pid < 0)
      return -1;
   if(pid > 0)
      exit(0);

   pid = fork();

   if(pid < 0)
      return -1;
   if(pid > 0)
      exit(0);

   setsid();
   chdir("/");
   umask(0);

   close(0);
   close(1);
   close(2);

   return 0;
}
