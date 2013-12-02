/*
 * src/db/text/mode.c
 *
 * Copyright 2002 Colin O'Leary
 *
 * Handles setting/unsetting/checking of various modes.
 *
 */  

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "mode.h"

static char cfgname[1024];

void text_mode_setconfig(const char *filename)
{
   strncpy(cfgname, filename, 1024);
   cfgname[1023] = 0;
}

void text_mode_set(int _uid, char *_net, char *_chan, char *_mode,
   char *param)
{
   char row[512];
   int r;

   if(_mode[0] == 0 || _mode[1] == 0)
      return;

   if(_mode[0] != '+' && _mode[0] != '-')
      return;

   if(!isalpha(_mode[1]))
      return;

   /* look for this mode */
   r = config_get_row(cfgname, "modes", NULL, 0);

   while(r == CONFIG_OK)
   {
      char *uid, *net, *chan, *mode;
      int netok, chanok;

      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         break;

      uid = strtok(row, " \t");
      net = strtok(NULL, " \t");
      chan = strtok(NULL, " \t");
      mode = strtok(NULL, " \t");

      netok = 0;
      chanok = 0;

      if(_net && (strcasecmp(net, _net) == 0))
         netok = 1;
      if((_net == NULL) && (strcmp(net, "-") == 0))
         netok = 1;
      if(_chan && (strcasecmp(chan, _chan) == 0))
         chanok = 1;
      if((_chan == NULL) && (strcmp(chan, "-") == 0))
         chanok = 1;

      /* if found, remove */
      if((atoi(uid) == _uid) && netok && chanok && (_mode[1] == mode[0]))
      {
         int line = config_get_line();

         config_get_row(NULL, NULL, NULL, 0);
         config_set_row(cfgname, line, NULL);         
         break;
      }
   }

   /* add new mode */
   if(_mode[0] == '+')
   {
      if(_net == NULL)
         _net = "-";
      if(_chan == NULL)
         _chan = "-";
      if(param == NULL)
         param = "";

      snprintf(row, 512, "%d\t%s\t%s\t%c\t%s", _uid, _net, _chan,
         _mode[1], param);

      config_add_row(cfgname, "modes", row);
   }
}

void text_mode_get(int uid, char *network, char *chan, char *mode)
{
   char row[512], list[64];
   int i, r;

   i = 0;
   list[0] = 0;

   r = config_get_row(cfgname, "modes", NULL, 0);

   while(r == CONFIG_OK)
   {
      char *u, *n, *c, *m;
      int netok, chanok;

      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         break;

      u = strtok(row, " \t");
      n = strtok(NULL, " \t");
      c = strtok(NULL, " \t");
      m = strtok(NULL, " \t");

      if((!u) || (!n) || (!c) || (!m))
         continue;

      netok = 0;
      chanok = 0;

      if(network && (strcasecmp(network, n) == 0))
         netok = 1;
      if((network == NULL) && (strcmp(n, "-") == 0))
         netok = 1;
      if(chan && (strcasecmp(chan, c) == 0))
         chanok = 1;
      if((chan == NULL) && (strcmp(c, "-") == 0))
         chanok = 1;

      if(netok && chanok && (uid == atoi(u)))
      {
         list[i] = tolower(m[0]);
         i++;
         list[i] = 0;

         if(i >= 62)
            break;
      }
   }

   strncpy(mode, list, 64);
   list[63] = 0;
}

int text_mode_check(int _uid, char *_net, char *_chan, int _mode,
   char *_param, int len)
{
   char row[512];
   int r;

   r = config_get_row(cfgname, "modes", NULL, 0);

   while(r == CONFIG_OK)
   {
      char *uid, *net, *chan, *mode, *param;
      int netok, chanok;

      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         break;

      uid = strtok(row, " \t");
      net = strtok(NULL, " \t");
      chan = strtok(NULL, " \t");
      mode = strtok(NULL, " \t");
      param = strtok(NULL, "");

      if((!uid) || (!net) || (!chan) || (!mode))
         continue;

      netok = 0;
      chanok = 0;

      if(_net && (strcasecmp(net, _net) == 0))
         netok = 1;
      if((_net == NULL) && (strcmp(net, "-") == 0))
         netok = 1;
      if(_chan && (strcasecmp(chan, _chan) == 0))
         chanok = 1;
      if((_chan == NULL) && (strcmp(chan, "-") == 0))
         chanok = 1;

      if((atoi(uid) == _uid) && netok && chanok &&
         (tolower(mode[0]) == _mode))
      {
         if(param && _param)
         {
            strncpy(_param, param, len);
            _param[len - 1] = 0;
         }
         else if(_param)
            _param[0] = 0;

         return 1;
      }
   }

   return 0;
}

char *text_mode_get_chan_list(char *network, int mode)
{
   static char list[4096];
   char row[512];
   int len, r;

   list[0] = 0;
   len = 0;

   r = config_get_row(cfgname, "modes", NULL, 0);

   while(r == CONFIG_OK)
   {
      char *u, *n, *c, *m;

      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         break;

      u = strtok(row, " \t");
      n = strtok(NULL, " \t");
      c = strtok(NULL, " \t");
      m = strtok(NULL, " \t");

      if((!u) || (!n) || (!c) || (!m))
         continue;

      if((strcasecmp(network, n) == 0) &&
         (tolower(m[0]) == mode) &&
         (atoi(u) == 0))
      {
         if((len + strlen(c)) > 4092)
            break;

         strcat(list, c);
         len += strlen(c);
         strcat(list, " ");
         len++;
      }
   }

   if(len)
      len--;

   list[len] = 0;

   return list;
}

int text_mode_deluser(int uid)
{
   char idstr[16];

   sprintf(idstr, "%d", uid);

   while(config_set_str(cfgname, "modes", idstr, NULL) == CONFIG_OK)
      ;

   return 0;
}
