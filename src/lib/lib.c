/*          
 * src/lib/lib.c
 *          
 * Copyright 2001 Colin O'Leary
 *          
 * Initialization, helper functions.
 *
 */         
            
#include "mx3bot.h"

#include <string.h>
#include <tcl.h>

#include "lib.h"

int _lib_irc_init(Tcl_Interp *i);
int _lib_mx3bot_init(Tcl_Interp *i);
int _lib_user_init(Tcl_Interp *i);
int _lib_var_init(Tcl_Interp *i);
int _lib_note_init(Tcl_Interp *i);
int _lib_mode_init(Tcl_Interp *i);
int _lib_ban_init(Tcl_Interp *i);

#ifdef ENABLE_PHPNUKE
int _lib_phpnuke_init(Tcl_Interp *i);
#endif

#ifdef ENABLE_FXSITE
int _lib_fxsite_init(Tcl_Interp *i);
#endif

int lib_init(Tcl_Interp *interp)
{
   if(_lib_mx3bot_init(interp) < 0)
      return -1;
   if(_lib_irc_init(interp) < 0)
      return -1;
   if(_lib_user_init(interp) < 0)
      return -1;
   if(_lib_var_init(interp) < 0)
      return -1;
   if(_lib_note_init(interp) < 0)
      return -1;
   if(_lib_mode_init(interp) < 0)
      return -1;
   if(_lib_ban_init(interp) < 0)
      return -1;

#ifdef ENABLE_PHPNUKE
   if(_lib_phpnuke_init(interp) < 0)
      return -1;
#endif

#ifdef ENABLE_FXSITE
   if(_lib_fxsite_init(interp) < 0)
      return -1;
#endif

   return 0;
}

int _lib_int_from_obj(Tcl_Interp *interp, Tcl_Obj *obj)
{
   int val;

   Tcl_GetIntFromObj(interp, obj, &val);
   return val;
}
