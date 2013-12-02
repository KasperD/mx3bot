/*
 * src/db/text/userdb.c
 *
 * Copyright 2001-2002 Colin O'Leary
 *
 */

#include <crypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ban.h"
#include "config.h"
#include "misc.h"
#include "mode.h"
#include "mx3bot.h"
#include "note.h"
#include "userdb.h"
#include "var.h"

static char cfgname[1024];

void text_userdb_setconfig(const char *filename)
{
   strncpy(cfgname, filename, 1024);
   cfgname[1023] = 0;
}

int text_userdb_add(char *name, char *mask)
{
   char row[512], idstr[16], *id;
   int r, uid;

   uid = 0;

   r = config_get_row(cfgname, "users", NULL, 0);

   while(r == CONFIG_OK)
   {
      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         break;

      id = strtok(row, " \t");

      if(atoi(id) > uid)
         uid = atoi(id);
   }

   uid++;

   sprintf(idstr, "%d", uid);
   snprintf(row, 512, "%s\t-\t-", name);

   if(config_set_str(cfgname, "users", idstr, row) != CONFIG_OK)
      return 0;

   userdb_addmask(uid, mask);

   return uid;
}

int text_userdb_del(int id)
{
   char row[512], idstr[16];
   int r;

   if(id < 1)
      return -1;

   ban_deluser(id);
   mode_deluser(id);
   note_deluser(id);
   var_deluser(id);

   /* delete masks */
   r = config_get_row(cfgname, "masks", NULL, 0);

   while(r == CONFIG_OK)
   {
      char *mid, *uid;

      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         break;

      mid = strtok(row, " \t");
      uid = strtok(NULL, " \t");

      if((!mid) || (!uid))
         continue;

      if(atoi(uid) == id)
      {
         int line = config_get_line();

         config_get_row(NULL, NULL, NULL, 0);
         config_set_row(cfgname, line, NULL);
         r = config_get_row(cfgname, "masks", NULL, 0);
      }
   }

   /* finally, remove user */
   sprintf(idstr, "%d", id);
   if(config_set_str(cfgname, "users", idstr, NULL) != CONFIG_OK)
      return -1;

   return 0;
}

int text_userdb_find_byname(char *name)
{
   char row[512];
   int r;

   r = config_get_row(cfgname, "users", NULL, 0);

   while(r == CONFIG_OK)
   {
      char *id, *n;

      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         return 0;

      id = strtok(row, " \t");
      n = strtok(NULL, " \t");

      if(n && (strcasecmp(n, name) == 0))
         return atoi(id);
   }

   return 0;
}

int text_userdb_find_bymask3(char *nick, char *user, char *host)
{
   char ident[256];

   snprintf(ident, 256, "%s!%s@%s", nick, user, host);
   return userdb_find_bymask(ident);
}

int text_userdb_find_bymask(char *ident)
{
   char row[512];
   int r;

   r = config_get_row(cfgname, "masks", NULL, 0);

   while(r == CONFIG_OK)
   {
      char *mid, *uid, *mask;

      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         return 0;

      mid = strtok(row, " \t");
      uid = strtok(NULL, " \t");
      mask = strtok(NULL, " \t");

      if(mask && mask_match(mask, ident))
         return atoi(uid);
   }

   return 0;
}

char *text_userdb_get_name(int id)
{
   static char name[256];
   char idstr[16];

   sprintf(idstr, "%d", id);

   if(config_get_str(cfgname, "users", idstr, name, 256) == CONFIG_OK)
   {
      strtok(name, " \t");
      return name;
   }

   return NULL;
}

int text_userdb_set_name(int id, char *name)
{
   char row[512], str[512], idstr[16];

   sprintf(idstr, "%d", id);

   if(config_get_str(cfgname, "users", idstr, row, 512) == CONFIG_OK)
   {
      char *uid, *pass;

      uid = strtok(row, " \t");
      strtok(NULL, " \t");
      strtok(NULL, " \t");
      pass = strtok(NULL, " \t");

      if((!uid) || (!pass))
         return -1;

      snprintf(str, 512, "%s\t-\t%s", name, pass);

      if(config_set_str(cfgname, "users", uid, str) == CONFIG_OK)
         return 0;
   }

   return -1;
}

int text_userdb_has_password(int id)
{
   char idstr[16];
   char row[512];

   sprintf(idstr, "%d", id);

   if(config_get_str(cfgname, "users", idstr, row, 512) == CONFIG_OK)
   {
      char *name, *level, *pass;

      name = strtok(row, " \t");
      level = strtok(NULL, " \t");
      pass = strtok(NULL, " \t");

      if(pass && (strcmp(pass, "-") != 0))
         return 1;
   }

   return 0;
}

int text_userdb_check_password(int id, char *pass)
{
   char row[512];
   char idstr[16];
   char hash[64];

   sprintf(idstr, "%d", id);

   if(config_get_str(cfgname, "users", idstr, row, 512) == CONFIG_OK)
   {
      char *name, *level, *p;

      name = strtok(row, " \t");
      level = strtok(NULL, " \t");
      p = strtok(NULL, " \t");

      /* succeed if no password is set */
      if((p == NULL) || (p[0] == 0) || (strcmp(p, "-") == 0))
         return 1;

      /* password is set, but none was supplied: fail */
      if(pass == NULL || pass[0] == 0)
         return 0;

      strncpy(hash, crypt(pass, p), 64);
      hash[63] = 0;

      /* match: success */
      if(strcmp(hash, p) == 0)
         return 1;
   }

   /* no match, or user not found: fail */
   return 0;
}

int text_userdb_set_password(int id, char *pass)
{
   char salt[] = "$1$--------";
   char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz./";
   char hash[64];
   char row[512], str[512], idstr[16];
   int i;

   for(i = 0; i < 8; i++)
      salt[i + 3] = chars[rand() % 54];

   if(pass == NULL || *pass == 0)
      strcpy(hash, "-");
   else
   {
      strncpy(hash, crypt(pass, salt), 64);
      hash[63] = 0;
   }

   sprintf(idstr, "%d", id);

   if(config_get_str(cfgname, "users", idstr, row, 512) == CONFIG_OK)
   {
      char *name, *level;

      name = strtok(row, " \t");
      level = strtok(NULL, " \t");

      if((!name) || (!level))
         return -1;

      snprintf(str, 512, "%s\t%s\t%s", name, level, hash);

      if(config_set_str(cfgname, "users", idstr, str) == CONFIG_OK)
         return 0;
   }

   return -1;
}

int text_userdb_addmask(int uid, char *mask)
{
   char row[512], idstr[16], *id;
   int r, mid;

   mid = 0;

   r = config_get_row(cfgname, "masks", NULL, 0);

   while(r == CONFIG_OK)
   {
      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         break;

      id = strtok(row, " \t");

      if(atoi(id) > mid)
         mid = atoi(id);
   }

   mid++;

   sprintf(idstr, "%d", mid);
   snprintf(row, 512, "%d\t%s", uid, mask);

   if(config_set_str(cfgname, "masks", idstr, row) != CONFIG_OK)
      return -1;

   return 0;
}

int text_userdb_delmask(int user, char *mask)
{
   char row[512];
   int r;

   r = config_get_row(cfgname, "masks", NULL, 0);

   while(r == CONFIG_OK)
   {
      char *mid, *uid;

      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         break;

      mid = strtok(row, " \t");
      uid = strtok(NULL, " \t");

      if((!mid) || (!uid))
         continue;

      if(atoi(uid) == user)
      {
         int line = config_get_line();

         config_get_row(NULL, NULL, NULL, 0);
         config_set_row(cfgname, line, NULL);
         r = config_get_row(cfgname, "masks", NULL, 0);
      }
   }

   return 0;
}

char *text_userdb_getmask(int mid)
{
   static char mask[256];
   char idstr[16];
   char row[512];

   sprintf(idstr, "%d", mid);

   if(config_get_str(cfgname, "masks", idstr, row, 512) == CONFIG_OK)
   {
      char *uid, *m;

      uid = strtok(row, " \t");
      m = strtok(NULL, " \t");

      if(mask)
      {
         strncpy(mask, m, 256);
         mask[255] = 0;
         return mask;
      }
   }

   return NULL;
}

char *text_userdb_getmasklist(int uid)
{
   static char list[1024];
   int len, r;

   list[0] = 0;
   len = 0;

   r = config_get_row(cfgname, "masks", NULL, 0);

   while(r == CONFIG_OK)
   {
      char row[512], *mid, *user;

      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         break;

      mid = strtok(row, " \t");
      user = strtok(NULL, " \t");

      if(user && (atoi(user) == uid))
      {
         if(len + strlen(mid) >= 1020)
            break;

         strcat(list, mid);
         len += strlen(mid);
         strcat(list, " ");
         len++;
      }
   }

   if(len)
      len--;

   list[len] = 0;

   return list;
}

int text_userdb_count(void)
{
   int count, r;
   char row[1];

   count = 0;

   r = config_get_row(cfgname, "users", NULL, 0);

   while(config_get_row(NULL, NULL, row, 1) == CONFIG_OK)
      count++;

   return count;
}
