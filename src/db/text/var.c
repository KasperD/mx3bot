/*
 * src/db/text/var.c
 *
 * Copyright 2002 Colin O'Leary
 *
 * Get/set variables.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "var.h"

static char cfgname[1024];

void text_var_setconfig(const char *filename)
{
   strncpy(cfgname, filename, 1024);
   cfgname[1023] = 0;
}

int text_var_set(int _uid, char *_name, char *val)
{
   char row[512];
   int r, line;

   if(_name == NULL)
      return -1;

   r = config_get_row(cfgname, "vars", NULL, 0);

   while(r == CONFIG_OK)
   {
      char *uid, *name;

      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         break;

      uid = strtok(row, " \t");
      name = strtok(NULL, " \t");

      if((!uid) || (!name))
         continue;

      if((atoi(uid) == _uid) && (strcasecmp(name, _name) == 0))
      {
         line = config_get_line();
         config_get_row(NULL, NULL, NULL, 0);

         if(val)
         {
            snprintf(row, 512, "%d\t%s\t%s", _uid, _name, val);
            val = row;
         }

         if(config_set_row(cfgname, line, val) != CONFIG_OK)
            return -1;

         return 0;
      }
   }

   snprintf(row, 512, "%d\t%s\t%s", _uid, _name, val);

   if(config_add_row(cfgname, "vars", row) != CONFIG_OK)
      return -1;

   return 0;
}

int text_var_get(int uid, char *name, char *val, int len)
{
   char row[512];
   int r;

   r = config_get_row(cfgname, "vars", NULL, 0);

   while(r == CONFIG_OK)
   {
      if((r = config_get_row(NULL, NULL, row, 512)) == CONFIG_OK)
      {
         char *u, *n, *v;

         u = strtok(row, " \t");
         n = strtok(NULL, " \t");
         v = strtok(NULL, "");

         if(v && (atoi(u) == uid) && (strcasecmp(name, n) == 0))
         {
            strncpy(val, v, len);
            val[len - 1] = 0;
            return 0;
         }
      }
   }

   return -1;
}

int text_var_get_def(int uid, char *name, char *val, int len, char *def)
{
   if(var_get(uid, name, val, len) != 0)
   {
      strncpy(val, def, len);
      val[len - 1] = 0;
   }

   return 0;
}

int text_var_set_unique(int uid, char *val)
{
   char name[32];
   int found, r, num;

   found = 1;

   while(found == 1)
   {
      num = rand() & 0x7fffffff;
      sprintf(name, "%d", num);

      r = config_get_str(cfgname, "vars", name, NULL, 0);

      if(r == CONFIG_ERROR)
         return 0;

      if(r == CONFIG_NOTFOUND)
         found = 0;
   }

   var_set(uid, name, val);
   return num;
}

int text_var_deluser(int uid)
{
   char idstr[16];

   sprintf(idstr, "%d", uid);

   while(config_set_str(cfgname, "vars", idstr, NULL) == CONFIG_OK)
      ;

   return 0;
}
