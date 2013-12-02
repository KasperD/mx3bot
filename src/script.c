/*          
 * src/script.c
 *
 * Copyright 2001 Colin O'Leary
 *
 * Deals with Tcl.
 *
 */

#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <tcl.h>
#include <unistd.h>

#include "connection.h"
#include "lib.h"
#include "log.h"
#include "script.h"

static Tcl_Interp *interp;
static char script_path[256];

static char pre_exec[] =
   "set param [split [string trim $param]]\n"
   "set args [llength $param]\n";
static char post_exec[] =
   "\n";

int script_init(char *path)
{
   if(path == NULL)
      strcpy(script_path, ".");
   else
      strncpy(script_path, path, 256);

   if(script_path[strlen(script_path) - 1] == '/')
      script_path[strlen(script_path) - 1] = 0;

   interp = Tcl_CreateInterp();

   if(interp == NULL)
   {
      log_error("Could not create Tcl interpreter.");
      return -1;
   }   

   lib_init(interp);

   return 0;
}


int script_exec(int cid, char *name, char *reply, ...)
{
   int ret, junk, vars, i;
   char filename[256], errormsg[256], name_buf[256];
   char *var_name;
   va_list ap, ap2;

   if(strchr(name, '.'))
      return -1;
   if(strchr(name, '/'))
      return -1;

   strncpy(name_buf, name, 256);

   for(i = 0; name_buf[i] != 0; i++)
      name_buf[i] = tolower(name_buf[i]);

   snprintf(filename, 256, "%s/%s.tcl", script_path, name_buf);

   if(access(filename, R_OK) < 0)
      return -1;

   va_start(ap, reply);
   vars = 0;
   while(1)
   {
      char *str, buf[32];

      if((var_name = va_arg(ap, char *)) == NULL)
         break;

      vars++;

      switch(toupper(var_name[0]))
      {
         case 'S':
            str = va_arg(ap, char *);
            Tcl_SetVar(interp, var_name + 1, str, 0);
            break;
         case 'I':
            snprintf(buf, 32, "%d", va_arg(ap, int));
            Tcl_SetVar(interp, var_name + 1, buf, 0);
            break;
         case 'C':
            buf[0] = va_arg(ap, int);
            buf[1] = 0;
            Tcl_SetVar(interp, var_name + 1, buf, 0);
            break;
         default:
            log_warning("Invalid variable type `%c'.", var_name[0]);
            junk = va_arg(ap, int);
            break;
      }
   }
   va_end(ap);

   connection_set_target(cid);

   Tcl_Eval(interp, pre_exec);
   ret = Tcl_EvalFile(interp, filename);

   if(ret != TCL_OK)
   {
      snprintf(errormsg, 256, "Tcl: %s: %s", name,
         Tcl_GetStringResult(interp));
      log_warning("%s", errormsg);

      if(reply != NULL)
         connection_send("NOTICE %s %s", reply, errormsg);
   }

   Tcl_Eval(interp, post_exec);

   va_start(ap2, reply);
   for(i = 0; i < vars; i++)
   {
      Tcl_UnsetVar(interp, va_arg(ap2, char *), 0);
      junk = va_arg(ap2, int);
   }
   va_end(ap2);

   return ret;
}
