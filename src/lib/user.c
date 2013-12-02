/*
 * src/lib/user.c
 *
 * Copyright 2001-2002 Colin O'Leary
 *
 * Tcl commands corresponding to src/user.c and src/db/(type)/userdb.c
 *
 */

#include <string.h>
#include <tcl.h>

#include "connection.h"
#include "lib.h"
#include "mx3bot.h"
#include "userdb.h"
#include "user.h"

/* src/userdb.c */
COMMAND_DECL(tcl_user);
COMMAND_DECL(tcl_user_getid);
COMMAND_DECL(tcl_user_getname);
COMMAND_DECL(tcl_user_setname);
COMMAND_DECL(tcl_user_haspass);
COMMAND_DECL(tcl_user_checkpass);
COMMAND_DECL(tcl_user_setpass);
COMMAND_DECL(tcl_user_add);
COMMAND_DECL(tcl_user_del);
COMMAND_DECL(tcl_user_addmask);
COMMAND_DECL(tcl_user_delmask);
COMMAND_DECL(tcl_user_getmasklist);
COMMAND_DECL(tcl_user_getmask);
COMMAND_DECL(tcl_user_count);

/* src/user.c */
COMMAND_DECL(tcl_getuser);
COMMAND_DECL(tcl_setauth);
COMMAND_DECL(tcl_user_debug);
COMMAND_DECL(tcl_getident);

int _lib_user_init(Tcl_Interp *interp)
{
   COMMAND_ADD(tcl_user, "user");
   COMMAND_ADD(tcl_getuser, "getuser");
   COMMAND_ADD(tcl_setauth, "setauth");
   COMMAND_ADD(tcl_user_debug, "user_debug");
   COMMAND_ADD(tcl_getident, "getident");

   return 0;
}

COMMAND(tcl_user)
{
   char cmd[64];

   if(objc < 2)
   {
      USAGE(1, "command ?args ...?");
      return TCL_ERROR;
   }

   strncpy(cmd, STR(1), 64);

   if(strcmp(cmd, "getid") == 0)
      return COMMAND_EXEC(tcl_user_getid);
   if(strcmp(cmd, "getname") == 0)
      return COMMAND_EXEC(tcl_user_getname);
   if(strcmp(cmd, "setname") == 0)
      return COMMAND_EXEC(tcl_user_setname);
   if(strcmp(cmd, "haspass") == 0)
      return COMMAND_EXEC(tcl_user_haspass);
   if(strcmp(cmd, "checkpass") == 0)
      return COMMAND_EXEC(tcl_user_checkpass);
   if(strcmp(cmd, "setpass") == 0)
      return COMMAND_EXEC(tcl_user_setpass);
   if(strcmp(cmd, "add") == 0)
      return COMMAND_EXEC(tcl_user_add);
   if(strcmp(cmd, "del") == 0)
      return COMMAND_EXEC(tcl_user_del);
   if(strcmp(cmd, "addmask") == 0)
      return COMMAND_EXEC(tcl_user_addmask);
   if(strcmp(cmd, "delmask") == 0)
      return COMMAND_EXEC(tcl_user_delmask);
   if(strcmp(cmd, "getmasklist") == 0)
      return COMMAND_EXEC(tcl_user_getmasklist);
   if(strcmp(cmd, "getmask") == 0)
      return COMMAND_EXEC(tcl_user_getmask);
   if(strcmp(cmd, "count") == 0)
      return COMMAND_EXEC(tcl_user_count);

   ERROR("user: invalid command");
   return TCL_ERROR;
}

COMMAND(tcl_user_getid)
{
   if(objc == 3)
   {
      R_INT(userdb_find_byname(STR(2)));
      return TCL_OK;
   }
   else if(objc == 4)
   {
      if(strcmp(STR(2), "-ident") == 0)
      {
         R_INT(userdb_find_bymask(STR(3)));
         return TCL_OK;
      }
      else
      {
         ERROR("user getid: invalid flag");
         return TCL_ERROR;
      }
   }

   USAGE(2, "handle");
   return TCL_ERROR;
}

COMMAND(tcl_user_getname)
{
   char *name;

   if(objc == 3)
   {
      name = userdb_get_name(INT(2));
      R_STR(name ? name : "");
      return TCL_OK;
   }

   USAGE(2, "userid");
   return TCL_ERROR;
}

COMMAND(tcl_user_setname)
{
   if(objc == 4)
   {
      R_INT(userdb_set_name(INT(2), STR(3)));
      return TCL_OK;
   }

   USAGE(2, "userid name");
   return TCL_ERROR;
}

/* user haspass uid */
COMMAND(tcl_user_haspass)
{
   if(objc == 3)
   {
      R_INT(userdb_has_password(INT(2)));
      return TCL_OK;
   }

   USAGE(2, "userid");
   return TCL_ERROR;
}

COMMAND(tcl_user_checkpass)
{
   if(objc == 4)
   {
      R_INT(userdb_check_password(INT(2), STR(3)));
      return TCL_OK;
   }

   USAGE(2, "userid pass");
   return TCL_ERROR;
}

COMMAND(tcl_user_setpass)
{
   char *pass;

   if(objc == 4)
   {
      pass = STR(3);

      if(pass[0] == '-' && pass[1] == 0)
         R_INT(userdb_set_password(INT(2), NULL));
      else
         R_INT(userdb_set_password(INT(2), pass));

      /* necessary? */
      user_update_id(INT(2));
      return TCL_OK;
   }

   USAGE(2, "userid pass");
   return TCL_ERROR;
}

COMMAND(tcl_user_add)
{
   if(objc == 4)
   {
      int id;
      char *mask;

      mask = STR(3);
      id = userdb_add(STR(2), mask);

      R_INT(id);
      user_update_add(id, mask);

      return TCL_OK;
   }

   USAGE(2, "name mask");
   return TCL_ERROR;
}

COMMAND(tcl_user_del)
{
   if(objc == 3)
   {
      int id = INT(2);

      R_INT(userdb_del(id));
      user_update_id(id);
      return TCL_OK;
   }

   USAGE(2, "userid");
   return TCL_ERROR;
}

COMMAND(tcl_user_addmask)
{
   if(objc == 4)
   {
      int id;
      char *mask;

      id = INT(2);
      mask = STR(3);

      R_INT(userdb_addmask(id, mask));
      user_update_add(id, mask);

      return TCL_OK;
   }

   USAGE(2, "userid mask");
   return TCL_ERROR;
}

COMMAND(tcl_user_delmask)
{
   if(objc == 4)
   {
      int id = INT(2);

      R_INT(userdb_delmask(id, STR(3))); 
      user_update_id(id);
      return TCL_OK;
   }

   USAGE(2, "userid mask");
   return TCL_ERROR;
}

COMMAND(tcl_user_getmasklist)
{
   if(objc == 3)
   {
      R_STR(userdb_getmasklist(INT(2)));
      return TCL_OK;
   }

   USAGE(2, "userid");
   return TCL_ERROR;
}

COMMAND(tcl_user_getmask)
{
   if(objc == 3)
   {
      R_STR(userdb_getmask(INT(2)));
      return TCL_OK;
   }

   USAGE(2, "maskid");
   return TCL_ERROR;
}

COMMAND(tcl_getuser)
{
   int uid;

   if(objc == 2)
   {
      uid = user_get_id(connection_get_current(), STR(1));
      R_INT(uid);

      return TCL_OK;
   }

   USAGE(1, "nick");
   return TCL_ERROR;
}

/* setauth nick uid */
COMMAND(tcl_setauth)
{
   if(objc == 3)
   {
      if(user_set_id(connection_get_current(), STR(1), INT(2)) == 0)
         R_INT(1);
      else
         R_INT(0);
      return TCL_OK;
   }

   USAGE(1, "nick uid");
   return TCL_ERROR;
}

COMMAND(tcl_user_count)
{
   if(objc == 2)
   {
      R_INT(userdb_count());
      return TCL_OK;
   }

   USAGE(2, "");
   return TCL_ERROR;
}

COMMAND(tcl_user_debug)
{
   if(objc == 1)
   {
      user_debug(-1);
      return TCL_OK;
   }
   else if(objc == 2)
   {
      user_debug(INT(1));
      return TCL_OK;
   }

   USAGE(1, "cid");
   return TCL_ERROR;
}

/* getident nick */
COMMAND(tcl_getident)
{
   if(objc == 2)
   {
      R_STR(user_get_ident(connection_get_current(),
         STR(1)));
      return TCL_OK;
   }

   USAGE(1, "nick");
   return TCL_ERROR;
}
