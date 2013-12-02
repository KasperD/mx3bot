/*          
 * src/lib/mode.c
 *          
 * Copyright 2001 Colin O'Leary
 *          
 * Tcl commands corresponding to src/mode.c
 *
 */         
            
#include <string.h>
#include <tcl.h>
#include <string.h>

#include "connection.h"
#include "lib.h"
#include "mode.h"

COMMAND_DECL(tcl_getmode);
COMMAND_DECL(tcl_setmode);
COMMAND_DECL(tcl_checkmode);
COMMAND_DECL(tcl_getmodeparam);
COMMAND_DECL(tcl_getmodelist);

int _lib_mode_init(Tcl_Interp *interp)
{
   COMMAND_ADD(tcl_getmode, "getmode");
   COMMAND_ADD(tcl_setmode, "setmode");
   COMMAND_ADD(tcl_checkmode, "checkmode");
   COMMAND_ADD(tcl_getmodeparam, "getmodeparam");
   COMMAND_ADD(tcl_getmodelist, "getmodelist");

   return 0;
}

/* setmode uid channel mode [param] */
COMMAND(tcl_setmode)
{
   int uid;
   char *net, *chan, *mode, *p;

   if(objc == 4 || objc == 5)
   {
      uid = INT(1);
      net = connection_get_name(-1);
      chan = STR(2);
      mode = STR(3);

      if(objc == 4)
         p = NULL;
      else
         p = STR(4);

      if(strcmp(chan, "-") == 0)
      {
         net = NULL;
         chan = NULL;
      }

      mode_set(uid, net, chan, mode, p);
      return TCL_OK;
   }

   USAGE(1, "userid chan mode ?param?");
   return TCL_ERROR;
}

/* getmode uid channel */
COMMAND(tcl_getmode)
{
   char mode[64], *net, *chan;
   int uid;

   if(objc == 3)
   {
      uid = INT(1);
      net = connection_get_name(-1);
      chan = STR(2);

      if(strcmp(chan, "-") == 0)
      {
         chan = NULL;
         net = NULL;
      }

      mode_get(uid, net, chan, mode);
      R_STR(mode);
      return TCL_OK;
   }

   USAGE(1, "userid chan");
   return TCL_ERROR;
}

/* checkmode uid channel mode */
COMMAND(tcl_checkmode)
{
   char *mode, *net, *chan;
   int uid;

   if(objc == 4)
   {
      uid = INT(1);
      net = connection_get_name(-1);
      chan = STR(2);
      mode = STR(3);

      if(strcmp(chan, "-") == 0)
      {
         chan = NULL;
         net = NULL;
      }

      while(*mode != 0)
      {
         if(mode_check(uid, net, chan, *mode, NULL, 0))
         {
            R_INT(1);
            return TCL_OK;
         }

         mode++;
      }

      R_INT(0);
      return TCL_OK;
   }

   USAGE(1, "userid chan mode");
   return TCL_OK;
}

/* getmodeparam uid channel mode */
COMMAND(tcl_getmodeparam)
{
   char mode[64], param[256], *net, *chan;
   int uid;

   if(objc == 4)
   {
      uid = INT(1);
      net = connection_get_name(-1);
      chan = STR(2);

      strncpy(mode, STR(3), 64);
      if(strlen(mode) > 1)
      {
         ERROR("getmodeparam: mode must be one character");
         return TCL_ERROR;
      }

      if(strcmp(chan, "-") == 0)
      {
         chan = NULL;
         net = NULL;
      }

      param[0] = 0;
      mode_check(uid, net, chan, mode[0], param, 256);

      R_STR(param);
      return TCL_OK;
   }

   USAGE(1, "userid chan mode");
   return TCL_OK;
}

/* getmodelist mode */
COMMAND(tcl_getmodelist)
{
   char mode[64];

   if(objc == 2)
   {
      strncpy(mode, STR(1), 64);

      if(strlen(mode) != 1)
      {
         ERROR("getmodelist: mode must be one character");
         return TCL_ERROR;
      }

      R_STR(mode_get_chan_list(connection_get_name(-1),
         mode[0]));
      return TCL_OK;
   }

   USAGE(1, "mode");
   return TCL_ERROR;
}

