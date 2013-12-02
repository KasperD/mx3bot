/*          
 * src/irc/irc_parse.c
 *          
 * Copyright 2001 Colin O'Leary
 *          
 * Parses input from IRC server, returning data in
 * an irc_line structure.
 */         
            
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "irc.h"
#include "irc_parse.h"

/* _irc_parseline() : populates irc_line struct from a line of input from
                      the irc server
   _line : input
   info  : output
*/
int _irc_parseline(char *_line, struct irc_line *info)
{
   char line[512];
   char *p, *server, *ident, *command, *target, *param, *param2;
   char *nick, *user, *host;
   int type;

   /* default: not available */
   server = NULL;
   ident = NULL;
   command = NULL;
   target = NULL;
   param = NULL;
   param2 = NULL;
   nick = NULL;
   user = NULL;
   host = NULL;

   type = IRC_CMD_UNKNOWN;

   /* do not modify original */
   strncpy(line, _line, 512);

   /* strip ending \r\n */
   if((p = strrchr(line, '\r')))
      *p = 0;
   if((p = strrchr(line, '\n')))
      *p = 0;

   if(line[0] == ':')
   {
      /* server/user */
      ident = strtok(line, " ");
      ident++;

      if(!(command = strtok(NULL, " ")))
         command = "";

      /* numeric reply: server message */
      if(isdigit(*command))
      {
         target = strtok(NULL, " ");
         param = strtok(NULL, "");

         type = IRC_CMD_NUMERIC;
      }
      /* PRIVMSG (or CTCP) */
      else if(strcmp(command, "PRIVMSG") == 0)
      {
         target = strtok(NULL, " ");
         if((param = strtok(NULL, "")))
            param++;

         /* CTCP */
         if(*param == 1)
         {
            if((p = strrchr(param, 1)))
               *p = 0;
            param++;

            param = strtok(param, " ");
            param2 = strtok(NULL, "");

            type = IRC_CMD_CTCP;
         }
         else
         {
            type = IRC_CMD_PRIVMSG;
         }
      }
      /* NOTICE */
      else if(strcmp(command, "NOTICE") == 0)
      {
         target = strtok(NULL, " ");
         if((param = strtok(NULL, "")))
            param++;

         type = IRC_CMD_NOTICE;
      }
      /* PART */
      else if(strcmp(command, "PART") == 0)
      {
         target = strtok(NULL, " ");
         if((param = strtok(NULL, "")))
            param++;

         type = IRC_CMD_PART;
      }
      /* JOIN */
      else if(strcmp(command, "JOIN") == 0)
      {
         if((target = strtok(NULL, " ")))
            target++;
         type = IRC_CMD_JOIN;
      }
      /* NICK */
      else if(strcmp(command, "NICK") == 0)
      {
         if((param = strtok(NULL, " ")))
            param++;
         type = IRC_CMD_NICK;
      }
      /* QUIT */
      else if(strcmp(command, "QUIT") == 0)
      {
         if((param = strtok(NULL, " ")))
            param++;
         type = IRC_CMD_QUIT;
      }
      /* KICK */
      else if(strcmp(command, "KICK") == 0)
      {
         target = strtok(NULL, " ");
         param = strtok(NULL, " ");
         if((param2 = strtok(NULL, "")))
            param2++;
         type = IRC_CMD_KICK;
      }
      /* MODE */
      else if(strcmp(command, "MODE") == 0)
      {
         target = strtok(NULL, " ");
         param = strtok(NULL, " ");
         param2 = strtok(NULL, "");
         type = IRC_CMD_MODE;
      }
      /* TOPIC */
      else if(strcmp(command, "TOPIC") == 0)
      {
         target = strtok(NULL, " ");
         if((param = strtok(NULL, "")))
            param++;
         type = IRC_CMD_TOPIC;
      }
      /* KILL */
      else if(strcmp(command, "KILL") == 0)
      {
         target = strtok(NULL, " ");
         param = strtok(NULL, "");
         type = IRC_CMD_KILL;
      }
      else if(strcmp(command, "PONG") == 0)
      {
         target = strtok(NULL, " ");
         if((param = strtok(NULL, "")))
            param++;
         type = IRC_CMD_PONG;
      }
      else if(strcmp(command, "INVITE") == 0)
      {
         target = strtok(NULL, " ");
         if((param = strtok(NULL, "")));
            param++;
         type = IRC_CMD_INVITE;
      }
      else
      {
      }
   }
   else
   {
      command = strtok(line, " ");

      if(command == NULL)
         command = "";

      /* PING */
      if(strcmp(command, "PING") == 0)
      {
         if((param = strtok(NULL, "")))
            param++;
         type = IRC_CMD_PING;
      }
      /* ERROR */
      else if(strcmp(command, "ERROR") == 0)
      {
         param = strtok(NULL, "");
         type = IRC_CMD_ERROR;
      }
   }

   if(ident != NULL)
   {
      char temp[256];

      strncpy(temp, ident, 256);

      if((nick = strtok(temp, "!")))
      {
         user = strtok(NULL, "@");
         host = strtok(NULL, "");
      }
      else
      {
         server = ident;
         ident = NULL;
      }
   }

   strncpy(info->server, server ? server : "", 128);
   strncpy(info->command, command ? command : "", 128);
   strncpy(info->nick, nick ? nick : "", 32);
   strncpy(info->user, user ? user : "", 32);
   strncpy(info->host, host ? host : "", 128);
   strncpy(info->ident, ident ? ident : "", 128);
   strncpy(info->target, target ? target : "", 32);
   strncpy(info->param, param ? param : "", 512);
   strncpy(info->param2, param2 ? param2 : "", 512);

   return type;
}
