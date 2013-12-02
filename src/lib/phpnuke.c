/*          
 * src/lib/phpnuke.c
 *          
 * Copyright 2001 Colin O'Leary
 *          
 * Tcl commands to access a PHPNuke database.
 *
 */         

#include "mx3bot.h"

#ifdef ENABLE_PHPNUKE

#include <string.h>
#include <tcl.h>

#include "lib.h"
#include "misc.h"
#include "sql.h"

COMMAND_DECL(tcl_phpnuke);
COMMAND_DECL(tcl_phpnuke_connect);
COMMAND_DECL(tcl_phpnuke_disconnect);
COMMAND_DECL(tcl_phpnuke_getstory);

static char error_msg[256];
static void error_handler(const char *msg)
{
   strncpy(error_msg, msg, 256);
}

int _lib_phpnuke_init(Tcl_Interp *interp)
{
   COMMAND_ADD(tcl_phpnuke, "phpnuke");

   return 0;
}

/* phpnuke command ?args? */
COMMAND(tcl_phpnuke)
{
   char cmd[64];

   if(objc < 2)
   {
      USAGE(1, "command ?args ...?");
      return TCL_ERROR;
   }

   strncpy(cmd, STR(1), 64);

   if(strcmp(cmd, "connect") == 0)
      return COMMAND_EXEC(tcl_phpnuke_connect);
   if(strcmp(cmd, "disconnect") == 0)
      return COMMAND_EXEC(tcl_phpnuke_disconnect);
   if(strcmp(cmd, "getstory") == 0)
      return COMMAND_EXEC(tcl_phpnuke_getstory);

   ERROR("phpnuke: invalid command");
   return TCL_ERROR;
}

/* phpnuke connect host user pass name */
COMMAND(tcl_phpnuke_connect)
{
   if(objc == 6)
   {
      if(db_connect(DB_PHPNUKE, STR(2), STR(3), STR(4), STR(5),
         error_handler) != 0)
         R_INT(0);
      else
         R_INT(1);

      error_msg[0] = 0;

      return TCL_OK; 
   }

   USAGE(2, "host user name pass");
   return TCL_ERROR;
}

/* phpnuke disconnect */
COMMAND(tcl_phpnuke_disconnect)
{
   if(objc == 2)
   {
      db_disconnect(DB_PHPNUKE);
      return TCL_OK;
   }

   USAGE(2, "");
   return TCL_ERROR;
}

/* phpnuke getstory id */
COMMAND(tcl_phpnuke_getstory)
{
   char str[16384];
   DB_RESULT result;
   DB_ROW row;

   if(objc == 3)
   {
      if(strcmp(STR(2), "latest") == 0)
      {
         strncpy(str,
            "SELECT sid, aid, title, time, hometext, comments, counter "
            "FROM nuke_stories "
            "ORDER BY time DESC "
            "LIMIT 1", 256);
      }
      else
      {
         if(INT(2) < 1)
         {
            ERROR("phpnuke getstory: invalid id");
            return TCL_ERROR;
         }

         snprintf(str, 256,
            "SELECT sid, aid, title, time, hometext, comments, counter "
            "FROM nuke_stories "
            "WHERE sid = %d", INT(2));
      }

      result = db_query(DB_PHPNUKE, str);

      if(db_rows(result) != 1)
      {
         R_STR("0");
      }
      else
      {
         Tcl_Obj *objv[7], *list;

         row = db_fetch(result);

         /* remove newlines */
         strncpy(str, row[4], 16384);
         str[16383] = 0;
         nl2sp(str);

         /* sid */
         objv[0] = Tcl_NewIntObj(db_int(row[0]));

         /* author */
         objv[1] = Tcl_NewStringObj(row[1], -1);

         /* title */
         objv[2] = Tcl_NewStringObj(row[2], -1);

         /* date */
         objv[3] = Tcl_NewStringObj(row[3], -1);

         /* number of comments */
         objv[4] = Tcl_NewIntObj(db_int(row[5]));

         /* number of reads */
         objv[5] = Tcl_NewIntObj(db_int(row[6]));

         /* text */
         objv[6] = Tcl_NewStringObj(str, -1);

         list = Tcl_NewListObj(7, objv);
         Tcl_SetObjResult(interp, list);
      }

      db_free(result);
      return TCL_OK;
   }

   USAGE(2, "storyid|\"latest\"");
   return TCL_ERROR;
}

#endif /* ENABLE_PHPNUKE */
