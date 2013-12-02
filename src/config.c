/*
 * src/config.c
 *
 * Copyright 2001-2002 Colin O'Leary
 *
 * Routines to read from a simple text configuration file.
 * 
 * Note: These routines suck.
 *
 */  

#include <ctype.h>     
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "log.h"

/* updated by config_get_row(), access through config_get_line() */
static int config_line;

static char *config_read_file(const char *filename, int *length);
static int config_write(const char *filename, const char *buf, int length,
   int start, int end, const char *row);

int config_get_str(const char *filename, const char *section,
   const char *name, char *buf, int maxlen)
{
   FILE *cfg;
   char buffer[1024];
   char *p;
   int sect = 0;

   cfg = fopen(filename, "r");
   if(cfg == NULL)
      return CONFIG_ERROR;

   while(fgets(buffer, 1024, cfg) != NULL)
   {
      p = strtok(buffer, " \t\r\n");

      if(p == NULL)
         continue;
      if(p[0] == '#')
         continue;

      if(p[0] == '[')
      {
         p++;
         p = strtok(p, "]");

         if((p != NULL) && (strcmp(p, section) == 0))
            sect = 1;
         else
            sect = 0;
      }
      else
      {
         if(sect && (strcmp(p, name) == 0))
         {
            p = strtok(NULL, "\r\n");

            while((*p) && (*p == ' ' || *p == '\t'))
               p++;

            if(p)
            {
               if(buf)
               {
                  strncpy(buf, p, maxlen);
                  buf[maxlen - 1] = 0;
               }

               fclose(cfg);
               return CONFIG_OK;
            }
         }
      }
   }

   fclose(cfg);

   return CONFIG_NOTFOUND;
}

int config_get_int(const char *filename, const char *section,
   const char *name, int *val)
{
   char buf[64];
   int r;

   if((r = config_get_str(filename, section, name, buf, 64)) != CONFIG_OK)
      return r;

   *val = atoi(buf);

   return CONFIG_OK;
}

int config_get_bool(const char *filename, const char *section,
   const char *name, int *val)
{
   char buf[64];
   int r;

   if((r = config_get_str(filename, section, name, buf, 64)) != CONFIG_OK)
      return r;

   if((strcasecmp(buf, "true") == 0) ||
      (strcasecmp(buf, "t") == 0) ||
      (strcasecmp(buf, "yes") == 0) ||
      (strcasecmp(buf, "y") == 0) ||
      (strcasecmp(buf, "on") == 0) ||
      (atoi(buf) == 1))
   {
      *val = 1;
      return CONFIG_OK;
   }

   if((strcasecmp(buf, "false") == 0) ||
      (strcasecmp(buf, "f") == 0) ||
      (strcasecmp(buf, "no") == 0) ||
      (strcasecmp(buf, "n") == 0) ||
      (strcasecmp(buf, "off") == 0) ||
      (isdigit(buf[0]) && (atoi(buf) == 0)))
   {
      *val = 0;
      return CONFIG_OK;
   }

   return CONFIG_ERROR;
}

const char *config_bool_str(int val)
{
   if(val == 0)
      return "false";
   else if(val == 1)
      return "true";
   else
      return "invalid";
}

int config_get_line(void)
{
   return config_line;
}

int config_get_row(const char *filename, const char *section,
   char *row, int len)
{
   static FILE *cfg;
   static int sect = 0;
   char buffer[1024], *p;

   if((filename == NULL) && (section == NULL) && (row == NULL))
   {
      fclose(cfg);
      config_line = -1;
      return CONFIG_OK;
   }

   if(filename && section)
   {
      cfg = fopen(filename, "r");
      if(cfg == NULL)
         return CONFIG_ERROR;
      sect = 0;
      config_line = 0;
   }
   else if(filename || section)
   {
      return CONFIG_ERROR;
   }
   else
   {
      if(cfg == NULL)
         return CONFIG_ERROR;
   }

   while(fgets(buffer, 1024, cfg) != NULL)
   {
      config_line++;

      p = strtok(buffer, "\r\n");

      if(p == NULL)
         continue;
      if(p[0] == '#')
         continue;

      if(p[0] == '[')
      {
         /* this section is over; no more rows */
         if(section == NULL)
            break;

         p++;
         p = strtok(p, "]");

         /* found section; will return rows (if any) on next call */
         if((p != NULL) && (strcmp(p, section) == 0))
         {
            sect = 1;
            return CONFIG_OK;
         }
      }
      else if(sect)
      {
         if(row)
         {
            strncpy(row, p, len);
            row[len - 1] = 0;
         }

         return CONFIG_OK;
      }
   }

   fclose(cfg);
   cfg = NULL;
   sect = 0;
   config_line = -1;

   return CONFIG_DONE;
}

int config_set_str(const char *filename, const char *section,
   const char *name, const char *val)
{
   char line[1024], *buf;
   int i, sect, replace_start, replace_end, length;

   buf = config_read_file(filename, &length);
   if(buf == NULL)
   {
      log_warning("config_set_str: Could not read `%s'.", filename);
      return CONFIG_ERROR;
   }

   i = 0;
   sect = -1;
   replace_start = -1;
   replace_end = -1;

   while(i < length)
   {
      char *p, *id;
      int start, j;

      j = 0;
      start = i;

      while((buf[i] != 0) && (buf[i] != '\r') && (buf[i] != '\n') &&
         (j < 1023))
      {
         line[j] = buf[i];
         i++;
         j++;
      }

      line[j] = 0;

      while((i < length) && ((buf[i] == '\r') || (buf[i] == '\n')))
         i++;

      /* parse line */
      p = line;

      /* skip leading whitespace */
      while((*p == ' ') || (*p == '\t'))
         p++;

      /* ignore comments and blank lines */
      if(*p == '#' || *p == 0)
         continue;

      /* get id */
      id = strtok(p, " \t");

      /* section? */
      if(*id == '[')
      {
         if(sect != -1)
            break;

         id++;
         id = strtok(id, "]");

         if(strcasecmp(id, section) == 0)
         {
            if(name)
            {
               /* set sect if this is the right section */
               sect = start + j;
            }
            else
            {
               /* add row in place */
               replace_start = start + j + 1;
               replace_end = replace_start;
               break;
            }
         }
      }
      else if(sect != -1)
      {
         if(strcasecmp(id, name) == 0)
         {
            /* found data */
            replace_start = start;
            replace_end = start + j + 1;
         }
      }
   }

   /* `name' was not found: insert at beginning of section */
   if(replace_start == -1)
   {
      /* if not found, delete fails */
      if(val == NULL)
      {
         free(buf);
         return CONFIG_ERROR;
      }

      /* no sections were defined: invalid config */
      if(sect == -1)
      {
         free(buf);
         log_warning("config_set_str: Invalid config `%s': no sections.",
            filename);
         return CONFIG_ERROR;
      }

      replace_start = sect + 1;
      replace_end = sect + 1;
   }

   if(name)
      snprintf(line, 1024, "%s\t%s", name, val);
   else
      snprintf(line, 1024, "%s", val);

   if(config_write(filename, buf, length, replace_start, replace_end,
      line) != CONFIG_OK)
   {
      log_warning("config_set_str: config_write(`%s', %p, %d, %d, %d, %d"
         ") failed.", filename, buf, length, replace_start, replace_end,
         line);
      free(buf);
      return CONFIG_ERROR;
   }

   free(buf);
   return CONFIG_OK;
}

int config_add_row(const char *filename, const char *section,
   const char *row)
{
   return config_set_str(filename, section, NULL, row);
}

int config_set_row(const char *filename, int rownum, const char *row)
{
   char *buf;
   int i, start, end, length, current;

   buf = config_read_file(filename, &length);
   if(buf == NULL)
      return CONFIG_ERROR;

   i = 0;
   current = 1;

   /* find start of line */
   while((i < length) && (current < rownum))
   {
      if(buf[i] == '\n')
         current++;

      i++;
   }

   /* row not found: invalid line was given */
   if(current != rownum)
   {
      free(buf);
      return CONFIG_ERROR;
   }

   start = i;

   /* find end of line */
   while((i < length) && (buf[i] != '\n'))
      i++;

   if(i < length)
      i++;

   end = i;

   if(config_write(filename, buf, length, start, end, row) != CONFIG_OK)
   {
      free(buf);
      return CONFIG_ERROR;
   }

   free(buf);
   return CONFIG_OK;
}

static int config_write(const char *filename, const char *buf, int length,
   int start, int end, const char *row)
{
   FILE *cfg;
   int a, b;

   cfg = fopen(filename, "w");
   if(cfg == NULL)
   {
      log_debug("config_write: Couldn't open `%s'.", filename);
      return CONFIG_ERROR;
   }

   a = fwrite(buf, 1, start, cfg);

   if(row)
      fprintf(cfg, "%s\n", row);

   b = fwrite(buf + end, 1, length - end, cfg);
   fclose(cfg);

   return CONFIG_OK;
}

static char *config_read_file(const char *filename, int *length)
{
   FILE *cfg;
   long len;
   char *buf;

   cfg = fopen(filename, "r");
   if(cfg == NULL)
      return NULL;

   fseek(cfg, 0L, SEEK_END);
   len = ftell(cfg);

   fseek(cfg, 0L, SEEK_SET);

   buf = malloc(len);
   if(buf == NULL)
   {
      fclose(cfg);
      return NULL;
   }

   fread(buf, len, 1, cfg);
   fclose(cfg);

   *length = len;
   return buf;
}

