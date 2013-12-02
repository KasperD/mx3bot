/*
 * src/db/text/ban.c
 *
 * Copyright 2002 Colin O'Leary
 *
 * Implements functions to add/delete/check/get information on bans
 *   based on network, channel, and idents.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ban.h"
#include "config.h"
#include "misc.h"

static char cfgname[1024];

void text_ban_setconfig(const char *filename)
{
   strncpy(cfgname, filename, 1024);
   cfgname[1023] = 0;
}

char *text_ban_get_list(char *_net, char *_chan, char *ident)
{
   static char list[1024];
   char row[512];
   int len, r;

   list[0] = 0;
   len = 0;

   r = config_get_row(cfgname, "bans", NULL, 0);

   while(r == CONFIG_OK)
   {
      char *bid, *uid, *net, *chan, *mask;
      int netok, chanok, identok;

      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         break;

      bid = strtok(row, " \t");
      uid = strtok(NULL, " \t");
      net = strtok(NULL, " \t");
      chan = strtok(NULL, " \t");
      mask = strtok(NULL, " \t");

      if((!bid) || (!uid) || (!net) || (!chan) || (!mask))
         continue;

      netok = 0;
      chanok = 0;
      identok = 0;

      if((_net == NULL) || (strcasecmp(net, _net) == 0))
         netok = 1;
      if((_chan == NULL) || (strcasecmp(chan, _chan) == 0))
         chanok = 1;
      if((ident == NULL) || mask_match(mask, ident))
         identok = 1;

      if(netok && chanok && identok)
      {
         if(len + strlen(bid) > 1020)
            break;

         strcat(list, bid);
         len += strlen(bid);
         strcat(list, " ");
         len++;
      }
   }

   if(len)
      len--;

   list[len] = 0;

   return list;
}

int text_ban_get_id_bymask(char *_net, char *_chan, char *_mask)
{
   char row[512];
   int r;

   if(_net == NULL || _chan == NULL || _mask == NULL)
      return 0;

   r = config_get_row(cfgname, "bans", NULL, 0);

   while(r == CONFIG_OK)
   {
      char *bid, *uid, *net, *chan, *mask;

      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         break;

      bid = strtok(row, " \t");
      uid = strtok(NULL, " \t");
      net = strtok(NULL, " \t");
      chan = strtok(NULL, " \t");
      mask = strtok(NULL, " \t");

      if((!bid) || (!uid) || (!net) || (!chan) || (!mask))
         continue;

      if((strcasecmp(net, _net) == 0) &&
         (strcasecmp(chan, _chan) == 0) &&
         (strcasecmp(mask, _mask) == 0))
      {
         return atoi(bid);
      }
   }

   return 0;
}

int text_ban_get_id(char *_net, char *_chan, char *ident)
{
   char row[512];
   int r;

   if(_net == NULL || _chan == NULL)
      return 0;

   r = config_get_row(cfgname, "bans", NULL, 0);

   while(r == CONFIG_OK)
   {
      char *bid, *uid, *net, *chan, *mask;

      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         break;

      bid = strtok(row, " \t");
      uid = strtok(NULL, " \t");
      net = strtok(NULL, " \t");
      chan = strtok(NULL, " \t");
      mask = strtok(NULL, " \t");

      if((!bid) || (!uid) || (!net) || (!chan) || (!mask))
         continue;

      if((strcasecmp(net, _net) == 0) &&
         (strcasecmp(chan, _chan) == 0) &&
         mask_match(mask, ident))
      {
         return atoi(bid);
      }
   }

   return 0;
}

int text_ban_add(char *network, char *chan, char *mask, int uid,
   char *note)
{
   char row[512], idstr[16], *id;
   int bid, r;

   if(network == NULL || chan == NULL || mask == NULL)
      return 0;

   bid = ban_get_id_bymask(network, chan, mask);

   if(bid == 0)
   {
      r = config_get_row(cfgname, "bans", NULL, 0);

      while(r == CONFIG_OK)
      {
         if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
            break;

         id = strtok(row, " \t");

         if(atoi(id) > bid)
            bid = atoi(id);
      }

      bid++;
   }

   sprintf(idstr, "%d", bid);
   snprintf(row, 512, "%d\t%s\t%s\t%s\t%s", uid, network, chan, mask, note);

   if(config_set_str(cfgname, "bans", idstr, row) != CONFIG_OK)
      return 0;

   return bid;
}

int text_ban_del(int bid)
{
   char idstr[16];

   sprintf(idstr, "%d", bid);

   if(config_set_str(cfgname, "bans", idstr, NULL) != CONFIG_OK)
      return -1;

   return 0;
}

int text_ban_get(int bid, int *_uid, char *_mask, int mask_len,
   char *_note, int note_len)
{
   char row[512], idstr[16];

   sprintf(idstr, "%d", bid);

   if(config_get_str(cfgname, "bans", idstr, row, 512) == CONFIG_OK)
   {
      char *uid, *net, *chan, *mask, *note;

      uid = strtok(row, " \t");
      net = strtok(NULL, " \t");
      chan = strtok(NULL, " \t");
      mask = strtok(NULL, " \t");
      note = strtok(NULL, "");

      if((!uid) || (!net) || (!chan) || (!mask))
         return -1;

      if(_uid)
         *_uid = atoi(uid);
      if(_mask)
      {
         strncpy(_mask, mask, mask_len);
         _mask[mask_len - 1] = 0;
      }
      if(_note)
      {
         strncpy(_note, note, note_len);
         _note[note_len - 1] = 0;
      }

      return 0;
   }

   return -1;
}

int text_ban_deluser(int _uid)
{
   char row[512];
   int r;

   r = config_get_row(cfgname, "bans", NULL, 0);

   while(r == CONFIG_OK)
   {
      char *bid, *uid;

      if((r = config_get_row(NULL, NULL, row, 512)) != CONFIG_OK)
         break;

      bid = strtok(row, " \t");
      uid = strtok(NULL, " \t");

      if((!bid) || (!uid))
         continue;

      if(atoi(uid) == _uid)
      {
         int line = config_get_line();

         config_get_row(NULL, NULL, NULL, 0);
         config_set_row(cfgname, line, NULL);
         r = config_get_row(cfgname, "bans", NULL, 0);
      }
   }

   return 0;
}
