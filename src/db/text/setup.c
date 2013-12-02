/*
 * src/db/text/setup.c
 *
 * Copyright 2002 Colin O'Leary
 *
 * Adds text database files.
 *
 */

#include <stdio.h>
#include <unistd.h>

#include "ban.h"
#include "mode.h"
#include "note.h"
#include "setup.h"
#include "userdb.h"
#include "var.h"

/*
 * setup_database() : Sets up empty database.
 *
 * Returns negative if user canceled or on error, otherwise zero.
 *
 */
int text_setup_database(const char *dir, const char *a, const char *b,
   const char *c, const char *d)
{
   FILE *cfg;
   char filename[1024];

   /* bans.conf */

   snprintf(filename, 1024, "%s/bans.conf", dir);
   ban_setconfig(filename);

   if(access(filename, R_OK) == 0)
   {
      printf("Ban file `%s' already exists.\n", filename);
   }
   else
   {
      if((cfg = fopen(filename, "w")) == NULL)
      {
         printf("Could not open `%s' for writing.\n", filename);
         perror("   ");
         return -1;
      }

      fprintf(cfg, "[bans]\n");
      fclose(cfg);

      printf("Created ban file `%s'.\n", filename);
   }

   /* modes.conf */

   snprintf(filename, 1024, "%s/modes.conf", dir);
   mode_setconfig(filename);

   if(access(filename, R_OK) == 0)
   {
      printf("Mode file `%s' already exists.\n", filename);
   }
   else
   {
      if((cfg = fopen(filename, "w")) == NULL)
      {
         printf("Could not open `%s' for writing.\n", filename);
         perror("   ");
         return -1;
      }

      fprintf(cfg, "[modes]\n");
      fclose(cfg);

      printf("Created mode file `%s'.\n", filename);
   }

   /* notes.conf */

   snprintf(filename, 1024, "%s/notes.conf", dir);
   note_setconfig(filename);

   if(access(filename, R_OK) == 0)
   {
      printf("Note file `%s' already exists.\n", filename);
   }
   else
   {
      if((cfg = fopen(filename, "w")) == NULL)
      {
         printf("Could not open `%s' for writing.\n", filename);
         perror("   ");
         return -1;
      }

      fprintf(cfg, "[notes]\n");
      fclose(cfg);

      printf("Created note file `%s'.\n", filename);
   }

   /* users.conf */

   snprintf(filename, 1024, "%s/users.conf", dir);
   userdb_setconfig(filename);

   if(access(filename, R_OK) == 0)
   {
      printf("User file `%s' already exists.\n", filename);
   }
   else
   {
      if((cfg = fopen(filename, "w")) == NULL)
      {
         printf("Could not open `%s' for writing.\n", filename);
         perror("   ");
         return -1;
      }

      fprintf(cfg, "[users]\n\n\n");
      fprintf(cfg, "[masks]\n");
      fclose(cfg);

      printf("Created user file `%s'.\n", filename);
   }

   /* vars.conf */

   snprintf(filename, 1024, "%s/vars.conf", dir);
   var_setconfig(filename);

   if(access(filename, R_OK) == 0)
   {
      printf("Variable file `%s' already exists.\n", filename);
   }
   else
   {
      if((cfg = fopen(filename, "w")) == NULL)
      {
         printf("Could not open `%s' for writing.\n", filename);
         perror("   ");
         return -1;
      }

      fprintf(cfg, "[vars]\n");
      fclose(cfg);

      printf("Created variable file `%s'.\n\n", filename);
   }

   return 0;
}
