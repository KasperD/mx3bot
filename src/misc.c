/*
 * src/misc.c
 *
 * Copyright 2001 Colin O'Leary
 *
 * Miscellaneous functions, mask creation and matching.
 *
 */

#include "mx3bot.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef HAVE_REGEX
#include <sys/types.h>
#include <regex.h>
#endif

#include "log.h"
#include "misc.h"

char *timestr(time_t t)
{
   static char str[64];
   struct tm *tm;

   tm = localtime(&t);

   strftime(str, 64, "%a %b %d %I:%M:%S%P", tm);

   return str;
}

void nl2sp(char *p)
{
   while(*p != 0)
   {
      if(*p == '\n' || *p == '\r') *p = ' ';
      p++;
   }
}

int mask_match(char *mask, char *val)
{
   const char *m, *v, *m2, *v2, *end;

   if(mask[0] == '$')
   {
#ifdef HAVE_REGEX
      regex_t reg;

      if(regcomp(&reg, mask + 1, REG_ICASE | REG_NOSUB | REG_EXTENDED)
         != 0)
         return 0;

      if(regexec(&reg, val, 0, NULL, 0) == 0)
      {
         regfree(&reg);
         return 1;
      }

      regfree(&reg);
#endif
      return 0;
   }

   m = mask;
   v = val;
   end = val + strlen(val);

   while(*m)
   {
      if(!*v)
         return 0;

      /* basic string compare */
      while(*m && *m != '*')
      {
         if(tolower(*m) != tolower(*v))
            return 0;

         m++;
         v++;
      }

      if(*m == 0)
         continue;

      /* move pointer past wildcard(s) */
      while(*m == '*')
         m++;

      /* wildcard matches end of input */
      if(!*m)
         return 1;

      /* m2: next wildcard in mask */
      m2 = m;

      while(*m2 && *m2 != '*')
         m2++;

      v2 = end;

      do
      {
         v2--;

         /* v2: occurence of *m2 in v */
         while((v2 >= v) && *v2 != *m)
            v2--;

         /* if not found, no match */
         if(v2 < v)
            return 0;
      }
      while(strncmp(m, v2, m2 - m) != 0);

      /* update v */
      v = v2;
   }

   if(*v)
      return 0;

   return 1;
}

int mask_match3(char *nick, char *user, char *host, char *val)
{
   char ident[512];

   snprintf(ident, 512, "%s!%s@%s", nick, user, host);
   return mask_match(ident, val);
}

char *mask_create(char *ident, char *mask, int maxlen, int depth)
{
   char *nick, *user, *host;

   nick = strtok(ident, "!");
   user = strtok(NULL, "@");
   host = strtok(NULL, "");

   if(user && host)
      return mask_create2(user, host, mask, maxlen, depth);
   else
      return "";
}

char *mask_create2(char *user, char *_host, char *mask, int maxlen,
   int depth)
{
   char host[256];
   char *p;

   if(depth != 1)
      log_warning("mask_create2: depth != 1 is not implemented.");

   strncpy(host, _host, 256);

   if(user[0] == '~')
      user++;

   p = host + strlen(host) - 1;

   if(isdigit(*p))
   {
      /* create 192.168.0.* type mask */

      while((p > host) && (*p != '.'))
         p--;
      *p = 0;

      snprintf(mask, maxlen, "*!*%s@%s.*", user, host);
   }
   else
   {
      /* create *.domain.org type mask */

      p = host;

      while((*p) && (*p != '.'))
         p++;

      if(strlen(p) <= 4)
         snprintf(mask, maxlen, "*!*%s@%s", user, host);
      else
         snprintf(mask, maxlen, "*!*%s@*%s", user, p);
   }

   return mask;
}

int ischan(char *chan)
{
   if(chan == NULL)
      return 0;

   if(chan[0] == '#' || chan[0] == '+' || chan[0] == '&' ||
      chan[0] == '!')
      return 1;
   else
      return 0;
}

int text_input(char *prompt, char *buf, int len, char *def)
{
   char *p;

   if(def)
      printf("%s [%s]: ", prompt, def);
   else
      printf("%s: ", prompt);

   fgets(buf, len, stdin);

   if((p = strchr(buf, '\n')) != NULL)
      *p = 0;

   if((def != NULL) && (buf[0] == 0))
   {
      strcpy(buf, def);
      return 0;
   }

   return 1;
}

