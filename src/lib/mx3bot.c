/*          
 * src/lib/mx3bot.c
 *          
 * Copyright 2001-2002 Colin O'Leary
 *          
 * Miscellaneous Tcl commands.
 *
 */         
            
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <tcl.h>
#include <time.h>

#include "connection.h"
#include "lib.h"
#include "log.h"
#include "misc.h"
#include "mode.h"
#include "mx3bot.h"
#include "sql.h"

COMMAND_DECL(tcl_debug);
COMMAND_DECL(tcl_match);
COMMAND_DECL(tcl_createmask);
COMMAND_DECL(tcl_setcp);
COMMAND_DECL(tcl_rmode);
COMMAND_DECL(tcl_dbqueries);
COMMAND_DECL(tcl_dbfetches);
COMMAND_DECL(tcl_getsec);
COMMAND_DECL(tcl_getusec);
COMMAND_DECL(tcl_getversion);
COMMAND_DECL(tcl_timestr);
COMMAND_DECL(tcl_shutdown);
COMMAND_DECL(tcl_getnetwork);
COMMAND_DECL(tcl_ischan);
COMMAND_DECL(tcl_setnick);

static Tcl_Obj *uid_name, *nick_name;

int _lib_mx3bot_init(Tcl_Interp *interp)
{
   COMMAND_ADD(tcl_debug, "debug");
   COMMAND_ADD(tcl_match, "match");
   COMMAND_ADD(tcl_createmask, "createmask");
   COMMAND_ADD(tcl_setcp, "setcp");
   COMMAND_ADD(tcl_rmode, "rmode");
   COMMAND_ADD(tcl_dbqueries, "dbqueries");
   COMMAND_ADD(tcl_dbfetches, "dbfetches");
   COMMAND_ADD(tcl_getsec, "getsec");
   COMMAND_ADD(tcl_getusec, "getusec");
   COMMAND_ADD(tcl_getversion, "getversion");
   COMMAND_ADD(tcl_timestr, "timestr");
   COMMAND_ADD(tcl_shutdown, "shutdown");
   COMMAND_ADD(tcl_getnetwork, "getnetwork");
   COMMAND_ADD(tcl_ischan, "ischan");
   COMMAND_ADD(tcl_setnick, "setnick");

   uid_name = Tcl_NewStringObj("uid", -1);
   nick_name = Tcl_NewStringObj("nick", -1);

   return 0;
}

COMMAND(tcl_debug)
{
   if(objc == 2)
   {
      log_debug("%s", STR(1));
      return TCL_OK;
   }

   USAGE(1, "text");
   return TCL_ERROR;
}

COMMAND(tcl_match)
{
   if(objc == 3)
   {
      R_INT(mask_match(STR(1), STR(2)));
      return TCL_OK;
   }

   USAGE(1, "mask ident");
   return TCL_OK;
}

/* createmask ident [depth] */
COMMAND(tcl_createmask)
{
   if(objc == 2 || objc == 3)
   {
      char mask[512];
      int d;

      if(objc == 2)
         d = 1;
      else
         d = INT(2);

      mask_create(STR(1), mask, 512, d);
      R_STR(mask);
      return TCL_OK;
   }

   USAGE(1, "ident");
   return TCL_ERROR;
}

COMMAND(tcl_setcp)
{
   char str[4];

   if(objc == 2)
   {
      strncpy(str, STR(1), 1);
      str[1] = 0;
      connection_set_prefix(-1, str[0]);
      R_STR(str);
      return TCL_OK;
   }

   USAGE(1, "prefix");
   return TCL_ERROR;
}

/* rmode mode ?chan? */
COMMAND(tcl_rmode)
{
   int uid;
   char *nick, *mode, *net, *chan;

   if(objc >= 1 || objc <= 3)
   {
      if(objc > 1)
         mode = STR(1);
      else
         mode = NULL;

      if(objc == 3)
         chan = STR(2);
      else
         chan = NULL;

      if(chan)
         net = connection_get_name(-1);
      else
         net = NULL;

      Tcl_GetIntFromObj(interp, 
         Tcl_ObjGetVar2(interp, uid_name, NULL, 0), &uid);

      nick = Tcl_GetStringFromObj(Tcl_ObjGetVar2(interp, nick_name, NULL,
         0), NULL);

      if(uid == 0)
      {
         connection_send("NOTICE %s I don't recognize you.  `/msg %s "
            "help user' for help.", nick, connection_get_nick(-1));
         return TCL_RETURN;
      }

      /* success if no mode is given */
      if(mode == NULL || mode[0] == 0 || strcmp(mode, "-") == 0)
         return TCL_OK;

      log_debug("mode=%s net=%s chan=%s uid=%d", mode, net, chan, uid);

      /* check each mode in mode argument */
      while(*mode != 0)
      {
         if(mode_check(uid, net, chan, *mode, NULL, 0))
            return TCL_OK;

         mode++;
      }

      connection_send("NOTICE %s Access Denied.", nick);
      return TCL_RETURN;
   }

   USAGE(1, "mode ?chan?");
   return TCL_ERROR;
}

COMMAND(tcl_dbqueries)
{
#ifdef HAVE_SQL
   int q;
#endif

   if(objc == 1)
   {
#ifdef HAVE_SQL
      db_stats(&q, NULL);
      R_INT(q);
#else
      R_INT(0);
#endif
      return TCL_OK;
   }

   USAGE(1, "");
   return TCL_ERROR;
}

COMMAND(tcl_dbfetches)
{
#ifdef HAVE_SQL
   int f;
#endif

   if(objc == 1)
   {
#ifdef HAVE_SQL
      db_stats(NULL, &f);
      R_INT(f);
#else
      R_INT(0);
#endif
      return TCL_OK;
   }

   USAGE(1, "");
   return TCL_ERROR;
}

COMMAND(tcl_getsec)
{
   if(objc == 1)
   {
      R_INT(time(0));
      return TCL_OK;
   }

   USAGE(1, "");
   return TCL_ERROR;
}

COMMAND(tcl_getusec)
{
   if(objc == 1)
   {
      struct timeval tv;

      gettimeofday(&tv, NULL);
      R_INT(tv.tv_usec);
      return TCL_OK;
   }

   USAGE(1, "");
   return TCL_ERROR;
}

COMMAND(tcl_getversion)
{
   if(objc == 1)
   {
      R_STR(VERSION " " __DATE__ " " __TIME__);
      return TCL_OK;
   }

   USAGE(1, "");
   return TCL_ERROR;
}

COMMAND(tcl_timestr)
{
   if(objc == 1)
   {
      R_STR(timestr(time(0)));
      return TCL_OK;
   }
   else if(objc == 2)
   {
      R_STR(timestr(INT(1)));
      return TCL_OK;
   }

   USAGE(1, "?seconds?");
   return TCL_ERROR;
}

/* shutdown ?reason? */
COMMAND(tcl_shutdown)
{
   if(objc == 1)
   {
      connection_shutdown("");
      return TCL_OK;
   }
   else if(objc == 2)
   {
      connection_shutdown(STR(1));
      return TCL_OK;
   }

   USAGE(1, "?reason?");
   return TCL_ERROR;
}

COMMAND(tcl_getnetwork)
{
   if(objc == 1)
   {
      R_STR(connection_get_name(-1));
      return TCL_OK;
   }

   USAGE(1, "");
   return TCL_ERROR;
}

COMMAND(tcl_ischan)
{
   if(objc == 2)
   {
      R_INT(ischan(STR(1)));
      return TCL_OK;
   }

   USAGE(1, "chan");
   return TCL_ERROR;
}

/* setnick nick */
COMMAND(tcl_setnick)
{
   if(objc == 2)
   {
      connection_set_nick(-1, STR(1));
      return TCL_OK;
   }

   USAGE(1, "nick");
   return TCL_ERROR;
}
