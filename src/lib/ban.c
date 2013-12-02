/*          
 * src/lib/ban.c
 *          
 * Copyright 2001 Colin O'Leary
 *         
 * Tcl functions corresponding to src/ban.c 
 */         
            
#include <string.h>
#include <tcl.h>

#include "ban.h"
#include "connection.h"
#include "lib.h"

COMMAND_DECL(tcl_getbanlist);
COMMAND_DECL(tcl_getbanid);
COMMAND_DECL(tcl_getban);
COMMAND_DECL(tcl_addban);
COMMAND_DECL(tcl_delban);

int _lib_ban_init(Tcl_Interp *interp)
{
   COMMAND_ADD(tcl_getbanlist, "getbanlist");
   COMMAND_ADD(tcl_getbanid, "getbanid");
   COMMAND_ADD(tcl_getban, "getban");
   COMMAND_ADD(tcl_addban, "addban");
   COMMAND_ADD(tcl_delban, "delban");

   return 0;
}

/* getbanlist chan ident */
COMMAND(tcl_getbanlist)
{
   if(objc == 2)
   {
      R_STR(ban_get_list(connection_get_name(-1),
         STR(1), NULL));
      return TCL_OK;
   }
   else if(objc == 3)
   {
      R_STR(ban_get_list(connection_get_name(-1), 
         STR(1), STR(2)));
      return TCL_OK;
   }

   USAGE(1, "chan ident");
   return TCL_ERROR;
}

/* getbanid -mask chan mask
   getbanid chan ident
*/
COMMAND(tcl_getbanid)
{
   if(objc == 4)
   {
      if(strcmp(STR(1), "-mask") != 0)
      {
         ERROR("getbanid: invalid flag");
         return TCL_ERROR;
      }

      R_INT(ban_get_id_bymask(connection_get_name(-1),
         STR(2), STR(3)));
      return TCL_OK;
   }
   else if(objc == 3)
   {
      R_INT(ban_get_id(connection_get_name(-1),
         STR(1), STR(2)));
      return TCL_OK;
   }

   USAGE(1, "?-mask? chan ident|mask");
   return TCL_ERROR;
}

/* getban bid */
COMMAND(tcl_getban)
{
   if(objc == 2)
   {
      int uid;
      char mask[256], note[256], str[512];

      if(ban_get(INT(1), &uid, mask, 256, note, 256) == -1)
         str[0] = 0;
      else
         snprintf(str, 512, "%d %s %s", uid, mask, note);

      R_STR(str);
      return TCL_OK;
   }

   USAGE(1, "banid");
   return TCL_ERROR;
}

/* addban chan mask uid [note] */
COMMAND(tcl_addban)
{
   if(objc == 4 || objc == 5)
   {
      char *net, *chan, *mask, *note;
      int uid;

      net = connection_get_name(-1);
      chan = STR(1);
      mask = STR(2);
      uid = INT(3);

      if(objc == 4)
         note = "";
      else
         note = STR(4);

      R_INT(ban_add(net, chan, mask, uid, note));
      return TCL_OK;
   }

   USAGE(1, "chan mask userid ?note?");
   return TCL_ERROR;
}

/* delban bid */
COMMAND(tcl_delban)
{
   if(objc == 2)
   {
      ban_del(INT(1));
      return TCL_OK;
   }

   USAGE(1, "banid");
   return TCL_ERROR;
}

