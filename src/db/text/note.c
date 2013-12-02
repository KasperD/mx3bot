/*
 * src/db/text/note.c
 *
 * Copyright 2002 Colin O'Leary
 *
 * Routines to send and receive notes from/to users.
 *
 */  

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "note.h"

static char cfgname[1024];

void text_note_setconfig(const char *filename)
{
   strncpy(cfgname, filename, 1024);
   cfgname[1023] = 0;
}

char *text_note_get_list(int to, int from)
{
   static char list[1024];
   int len, r;

   list[0] = 0;
   len = 0;

   if((to == 0) && (from == 0))
      return list;

   r = config_get_row(cfgname, "notes", NULL, 0);

   while(r == CONFIG_OK)
   {
      char row[512], *nid, *t, *f;

      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         break;

      nid = strtok(row, " \t");
      f = strtok(NULL, " \t");
      t = strtok(NULL, " \t");

      if((!nid) || (!f) || (!t))
         continue;

      if(((to != 0) && (from == 0) && (to == atoi(t))) ||
         ((from != 0) && (to == 0) && (from == atoi(f))) ||
         ((to != 0) && (from != 0) && (to == atoi(t)) && (from ==
            atoi(f))))
      {
         if(len + strlen(nid) >= 1020)
            break;

         strcat(list, nid);
         len += strlen(nid);
         strcat(list, " ");
         len++;
      }
   }

   if(len)
      len--;

   list[len] = 0;

   return list;
}

int text_note_get(int nid, int *to, int *from, int *sent, char *note,
   int len)
{
   char row[512], idstr[16];

   sprintf(idstr, "%d", nid);

   if(config_get_str(cfgname, "notes", idstr, row, 512) == CONFIG_OK)
   {
      char *a, *b, *t, *msg;

      a = strtok(row, " \t");
      b = strtok(NULL, " \t");
      t = strtok(NULL, " \t");
      msg = strtok(NULL, "");

      if(msg)
      {
         if(from)
            *from = atoi(a);
         if(to)
            *to = atoi(b);
         if(sent)
            *sent = atoi(t);
         if(note)
         {
            strncpy(note, msg, len);
            note[len - 1] = 0;
         }

         return 1;
      }
   }

   return 0;
}

int text_note_del(int nid)
{
   char idstr[16];

   sprintf(idstr, "%d", nid);

   if(config_set_str(cfgname, "notes", idstr, 0) != CONFIG_OK)
      return -1;

   return 0;
}

int text_note_add(int to, int from, int sent, char *note)
{
   char row[512], idstr[16];
   int r, nid;

   nid = 0;

   r = config_get_row(cfgname, "notes", NULL, 0);

   while(r == CONFIG_OK)
   {
      char *id;

      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         break;

      id = strtok(row, " \t");

      if(!id)
         continue;

      if(atoi(id) > nid)
         nid = atoi(id);
   }

   nid++;

   sprintf(idstr, "%d", nid);
   snprintf(row, 512, "%d\t%d\t%d\t%s", from, to, sent, note);

   if(config_set_str(cfgname, "notes", idstr, row) != CONFIG_OK)
      return -1;

   return 0;
}

int text_note_deluser(int uid)
{
   char row[512];
   int r;

   r = config_get_row(cfgname, "notes", NULL, 0);

   while(r == CONFIG_OK)
   {
      char *nid, *from, *to;

      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         break;

      nid = strtok(row, " \t");
      from = strtok(NULL, " \t");
      to = strtok(NULL, " \t");

      if((!nid) || (!from) || (!to))
         continue;

      if((atoi(from) == uid) || (atoi(to) == uid))
      {
         int line = config_get_line();

         /* close config */
         config_get_row(NULL, NULL, NULL, 0);

         /* delete row */
         config_set_row(cfgname, line, NULL);

         /* reopen config */
         r = config_get_row(cfgname, "notes", NULL, 0);
      }
   }

   return 0;
}
