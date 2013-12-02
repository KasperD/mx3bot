/*          
 * src/lib/lib.h
 *          
 * Copyright 2001 Colin O'Leary
 *          
 * Some defines to make libtcl slightly less irritating.
 *
 * Definitions for src/lib/ initialization functions.
 */         
            
#ifndef _lib_h
#define _lib_h

#include <string.h>
#include <tcl.h>

#define STR(x) Tcl_GetStringFromObj(objv[(x)], NULL)
#define INT(x) _lib_int_from_obj(interp, objv[(x)])

#define ERROR(x) Tcl_SetResult(interp, (x), TCL_STATIC)
#define USAGE(n, x) Tcl_WrongNumArgs(interp, (n), objv, (x))

#define R_INT(x) 						\
   Tcl_SetObjResult(interp, Tcl_NewIntObj((x)))

#define R_STR(x)						\
   Tcl_SetObjResult(interp, Tcl_NewStringObj((x), strlen((x))))

#define COMMAND(x)						\
   static int x(ClientData cd, Tcl_Interp *interp, int objc,	\
      Tcl_Obj *const objv[])

#define COMMAND_DECL(x)						\
   static Tcl_ObjCmdProc x

#define COMMAND_ADD(func, name)					\
   Tcl_CreateObjCommand(interp, (name), (func), 0, 0)

#define COMMAND_EXEC(func)					\
   func(cd, interp, objc, objv)

int _lib_int_from_obj(Tcl_Interp *interp, Tcl_Obj *obj);

int lib_init(Tcl_Interp *i);

#endif
