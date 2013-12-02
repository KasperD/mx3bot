/*
 * src/log.c
 *
 * Copyright 2001 Colin O'Leary
 *
 * Various logfile support functions.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "log.h"

struct logfile_t
{
   int cid;
   char chan[32];
   char filename[256];
   FILE *file;
   int type;
};

static struct logfile_t *log;
static int n_logfiles, n_grow;
static char logpath[256];
static int dummy_mode;

static void clearlog(i)
{
   log[i].cid = -1;
   log[i].chan[0] = 0;
   log[i].filename[0] = 0;
   log[i].file = NULL;
   log[i].type = 0;
}

int log_init(int n, int g)
{
   int i;

   /* dummy init */
   if(n == -1)
   {
      dummy_mode = -1;
      return 0;
   }

   dummy_mode = 0;

   if(n < 1)
      n_logfiles = 32;
   else
      n_logfiles = n;

   if(g < 1)
      n_grow = 8;
   else
      n_grow = g;

   log = malloc(n_logfiles * sizeof(struct logfile_t));
   if(log == NULL)
      return -1;

   for(i = 0; i < n_logfiles; i++)
      clearlog(i);

   logpath[0] = 0;

   return 0;
}

void log_setpath(char *path)
{
   if(path)
   {
      int end;

      strncpy(logpath, path, 256);

      /* strip trailing slashes */
      end = strlen(logpath) - 1;

      while((end > 0) && (logpath[end] == '/'))
      {
         logpath[end] = 0;
         end--;
      }
   }
   else
   {
      logpath[0] = 0;
   }
}

static int openlog(int i, int cid, char *chan, char *filename, int type)
{
   if(log[i].file != NULL)
      fclose(log[i].file);

   log[i].file = fopen(filename, "a");
   if(log[i].file == NULL)
      return -1;

   log[i].cid = cid;
   log[i].type = type;

   if(chan)
      strncpy(log[i].chan, chan, 32);
   else
      log[i].chan[0] = 0;

   strncpy(log[i].filename, filename, 256);

   return 0;
}

static void closelog(int i)
{
   if(log[i].file != NULL)
   {
      fclose(log[i].file);
      log[i].file = NULL;
   }
}

static int findlog(int cid, char *chan)
{
   int i;

   if((cid == -1) && (chan == NULL))
      return 0;

   for(i = 1; i < n_logfiles; i++)
   {
      if((log[i].cid == cid) &&
         (strcasecmp(log[i].chan, chan) == 0))
         return i;
   }

   return -1;
}

int log_set(int cid, char *chan, char *filename, int type)
{
   int i;
   char pathname[512];

   if(dummy_mode)
   {
      if(type != 0)
         dummy_mode = type;

      return 0;
   }

   /* unset log if filename is null */
   if(filename == NULL)
   {
      if((cid == -1) && (chan == NULL))
         closelog(0);

      i = findlog(cid, chan);
      if(i == -1)
         return -1;

      closelog(i);
      return 0;
   }

   /* adjust filename if logpath is set */
   if(logpath[0] != 0)
      snprintf(pathname, 256, "%s/%s", logpath, filename);

   /* global logfile is always index zero */
   if((cid == -1) && (chan == NULL))
      return openlog(0, -1, NULL, pathname, type);

   /* find empty slot */
   for(i = 1; i < n_logfiles; i++)
   {
      if((log[i].cid == -1) &&
         (log[i].chan[0] == 0))
      {
         return openlog(i, cid, chan, pathname, type);
      }
   }

   /* grow table */
   log = realloc(log, (n_logfiles + n_grow) * sizeof(struct logfile_t));
   if(log == NULL)
   {
      if(n_logfiles > 0)
         log_error("log_set: Out of memory");
      return -2;
   }

   i = n_logfiles;
   n_logfiles += n_grow;

   for(; i < n_logfiles; i++)
      clearlog(i);

   return 0;
}

int log_add(int cid, char *chan, int type, char *format, ...)
{
   char buffer[1024];
   va_list ap;
   int i = 0, len;

   if(dummy_mode == 0)
   {
      /* return if we are not logging this channel */
      i = findlog(cid, chan);
      if(i == -1)
         return 0;

      /* return if we are not logging this type of info */
      if((log[i].type & type) == 0)
         return 0;

      /* add timestamp */
      if(log[i].type & LOGTYPE_TIMESTAMP)
      {
         time_t t;
         struct tm *tm;
 
         t = time(0);
         tm = localtime(&t);

         sprintf(buffer, "[%04d-%02d-%02d %02d:%02d:%02d] ",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
            tm->tm_min, tm->tm_sec);

         len = 22;
      }
      else
         len = 0;
   }
   else
   {
      if((dummy_mode & type) == 0)
         return 0;

      len = 0;
   }

   va_start(ap, format);
   vsnprintf(buffer + len, 1024 - len, format, ap);
   va_end(ap);

   if(dummy_mode)
      return 0;

   if(log[i].file)
   {
      fputs(buffer, log[i].file);
      fputc('\n', log[i].file);
      fflush(log[i].file);
   }

   /* echo to stdout if desired */
   if(log[i].type & LOGTYPE_STDOUT)
      puts(buffer);

   return 0;
}

