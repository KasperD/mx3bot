/*          
 * src/lib/note.c
 *          
 * Copyright 2001 Colin O'Leary
 *          
 * Tcl commands corresponding to src/note.c
 *
 */         
            
#include <tcl.h>
#include <time.h>

#include "lib.h"
#include "note.h"

COMMAND_DECL(tcl_addnote);
COMMAND_DECL(tcl_delnote);
COMMAND_DECL(tcl_getnote);
COMMAND_DECL(tcl_getnotelist);

int _lib_note_init(Tcl_Interp *interp)
{
   COMMAND_ADD(tcl_addnote, "addnote");
   COMMAND_ADD(tcl_delnote, "delnote");
   COMMAND_ADD(tcl_getnote, "getnote");
   COMMAND_ADD(tcl_getnotelist, "getnotelist");

   return 0;
}

COMMAND(tcl_addnote)
{
   if(objc == 4)
   {
      R_INT(note_add(INT(1), INT(2), time(0), STR(3)));
      return TCL_OK;
   }

   USAGE(1, "target author note");
   return TCL_ERROR;
}

COMMAND(tcl_delnote)
{
   if(objc == 2)
   {
      R_INT(note_del(INT(1)));
      return TCL_OK;
   }

   USAGE(1, "noteid");
   return TCL_ERROR;
}

COMMAND(tcl_getnote)
{
   char note[256], buf[512];
   int to, from, sent;

   if(objc == 2)
   {
      if(note_get(INT(1), &to, &from, &sent, note, 256) == 1)
      {
         sprintf(buf, "%d %d %d %s", to, from, sent, note);
         R_STR(buf);
      }
      else
      {
         R_STR("");
      }
      return TCL_OK;
   }

   USAGE(1, "noteid");
   return TCL_ERROR;
}

COMMAND(tcl_getnotelist)
{
   if(objc == 3)
   {
      R_STR(note_get_list(INT(1), INT(2)));
      return TCL_OK;
   }

   USAGE(1, "target author");
   return TCL_ERROR;
}
