// File Name: src/lib/fxsite.c
// Copyright: Colin O'Leary, 2001
// Description: TCL Commands to access fxSite dataase

#include "mx3bot.h"

#ifdef ENABLE_FXSITE

#include <string.h>
#include <tcl.h>

#include "lib.h"
#include "misc.h"
#include "sql.h"

COMMAND_DECL(tcl_fxsite);
COMMAND_DECL(tcl_fxsite_connect);
COMMAND_DECL(tcl_fxsite_disconnect);
COMMAND_DECL(tcl_fxsite_getnews);

static char error_msg[256];
static void error_handler(const char *msg)
{
   strncpy(error_msg, msg, 256);
}

int _lib_fxsite_init(Tcl_Interp *interp)
{
   COMMAND_ADD(tcl_fxsite, "fxsite");

   return 0;
}

/* fxsite command ?args? */
COMMAND(tcl_fxsite)
{
   char cmd[64];

   if(objc < 2)
   {
      USAGE(1, "command ?arg ...?");
      return TCL_ERROR;
   }

   strncpy(cmd, STR(1), 64);

   if(strcmp(cmd, "connect") == 0)
      return COMMAND_EXEC(tcl_fxsite_connect);
   if(strcmp(cmd, "disconnect") == 0)
      return COMMAND_EXEC(tcl_fxsite_disconnect);
   if(strcmp(cmd, "getnews") == 0)
      return COMMAND_EXEC(tcl_fxsite_getnews);

   ERROR("fxsite: invalid command");
   return TCL_ERROR;
}

/* fxsite connect host user pass name */
COMMAND(tcl_fxsite_connect)
{
   if(objc == 6)
   {
      if(db_connect(DB_FXSITE, STR(2), STR(3), STR(4), STR(5),
         error_handler) != 0)
         R_INT(0);
      else
         R_INT(1);

      error_msg[0] = 0;

      return TCL_OK; 
   }

   USAGE(2, "host user pass name");
   return TCL_ERROR;
}

/* fxsite disconnect */
COMMAND(tcl_fxsite_disconnect)
{
   if(objc == 2)
   {
      db_disconnect(DB_FXSITE);
      return TCL_OK;
   }

   USAGE(2, "");
   return TCL_ERROR;
}

/* fxsite getnews sectionid */
COMMAND(tcl_fxsite_getnews)
{
   char str[16384];
   DB_RESULT result;
   DB_ROW row;

   if(objc == 3)
   {
      /* Talk about invalid SQL */

      snprintf(str, 16384,
         "SELECT message.subject, message.time, message.body, user.name, "
         "       message.article "
         "FROM message, article, user "
         "WHERE article.section = %d "
         "AND message.article = article.id "
         "AND user.id = message.author "
         "GROUP BY message.article "
         "ORDER BY message.time DESC "
         "LIMIT 1", INT(2));

      result = db_query(DB_FXSITE, str);

      if(db_rows(result) != 1)
      {
         R_STR("0");
      }
      else
      {
         Tcl_Obj *objv[7], *list;

         row = db_fetch(result);

         /* remove newlines */
         strncpy(str, row[2], 16384);
         str[16383] = 0;
         nl2sp(str);

         /* article id */
         objv[0] = Tcl_NewIntObj(db_int(row[4]));

         /* author */
         objv[1] = Tcl_NewStringObj(row[3], -1);

         /* subject */
         objv[2] = Tcl_NewStringObj(row[0], -1);

         /* date */
         objv[3] = Tcl_NewStringObj(row[1], -1);

         /* number of comments */
         objv[4] = Tcl_NewIntObj(0);

         /* text */
         objv[5] = Tcl_NewStringObj(str, -1);

         list = Tcl_NewListObj(6, objv);
         Tcl_SetObjResult(interp, list);
      }

      db_free(result);
      return TCL_OK;
   }

   USAGE(2, "sectionid");
   return TCL_ERROR;
}

#endif /* ENABLE_FXSITE */
