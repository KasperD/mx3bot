/*          
 * src/lib/var.c
 *          
 * Copyright 2001 Colin O'Leary
 *          
 * Tcl commands corresponding to src/var.c.
 *
 */         
            
#include <tcl.h>

#include "lib.h"
#include "var.h"

COMMAND_DECL(tcl_getvar);
COMMAND_DECL(tcl_setvar);
COMMAND_DECL(tcl_unsetvar);
COMMAND_DECL(tcl_setuvar);

int _lib_var_init(Tcl_Interp *interp)
{
   COMMAND_ADD(tcl_getvar, "getvar");
   COMMAND_ADD(tcl_setvar, "setvar");
   COMMAND_ADD(tcl_unsetvar, "unsetvar");
   COMMAND_ADD(tcl_setuvar, "setuvar");

   return 0;
}

/* getvar user name ?default? */
COMMAND(tcl_getvar)
{
   char buf[512];

   buf[0] = 0;

   if(objc == 3)
   {
      if(var_get(INT(1), STR(2), buf, 512) == 0)
         R_STR(buf);
      else
         R_STR("");
      return TCL_OK;
   }
   else if(objc == 4)
   {
      var_get_def(INT(1), STR(2), buf, 512, STR(3));
      R_STR(buf);
      return TCL_OK;
   }

   USAGE(1, "userid name ?default?");
   return TCL_ERROR;
}

/* setvar user name val */
COMMAND(tcl_setvar)
{
   if(objc == 4)
   {
      var_set(INT(1), STR(2), STR(3));
      R_STR(STR(3));
      return TCL_OK;
   }

   USAGE(1, "userid name value");
   return TCL_ERROR;
}

/* setuvar user val */
COMMAND(tcl_setuvar)
{
   if(objc == 3)
   {
      R_INT(var_set_unique(INT(1), STR(2)));
      return TCL_OK; 
   }

   USAGE(1, "userid value");
   return TCL_ERROR;
}

/* unsetvar user name */
COMMAND(tcl_unsetvar)
{
   if(objc == 3)
   {
      var_set(INT(1), STR(2), NULL);
      R_STR("");
      return TCL_OK;
   }

   USAGE(1, "userid name");
   return TCL_ERROR;
}

