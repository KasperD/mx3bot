/*
 * src/lib/irc.c
 *
 * Copyright 2001-2002 Colin O'Leary
 *
 * Tcl commands to send data to IRC server.
 *
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <tcl.h>

#include "connection.h"
#include "misc.h"
#include "mx3bot.h"
#include "lib.h"
#include "log.h"

COMMAND_DECL(tcl_privmsg);
COMMAND_DECL(tcl_notice);
COMMAND_DECL(tcl_ctcp);
COMMAND_DECL(tcl_mode);
COMMAND_DECL(tcl_kick);
COMMAND_DECL(tcl_joinchan);
COMMAND_DECL(tcl_partchan);
COMMAND_DECL(tcl_topic);
COMMAND_DECL(tcl_reconnect);
COMMAND_DECL(tcl_disconnect);
COMMAND_DECL(tcl_irc);
COMMAND_DECL(tcl_connect);
COMMAND_DECL(tcl_connectdcc);
COMMAND_DECL(tcl_irctarget);

static char target[32];

#define SEND(msg...)				\
   do						\
   {						\
      if(target[0] == 0)			\
         connection_send(msg);			\
      else					\
         connection_queue(target, 0, msg);	\
   } while(0);

int _lib_irc_init(Tcl_Interp *interp)
{
   COMMAND_ADD(tcl_privmsg, "privmsg");
   COMMAND_ADD(tcl_notice, "notice");
   COMMAND_ADD(tcl_ctcp, "ctcp");
   COMMAND_ADD(tcl_mode, "mode");
   COMMAND_ADD(tcl_kick, "kick");
   COMMAND_ADD(tcl_joinchan, "joinchan");
   COMMAND_ADD(tcl_partchan, "partchan");
   COMMAND_ADD(tcl_topic, "topic");
   COMMAND_ADD(tcl_reconnect, "reconnect");
   COMMAND_ADD(tcl_disconnect, "disconnect");
   COMMAND_ADD(tcl_irc, "irc");
   COMMAND_ADD(tcl_connect, "connect");
   COMMAND_ADD(tcl_connectdcc, "connectdcc");
   COMMAND_ADD(tcl_irctarget, "irctarget");

   target[0] = 0;

   return 0;
}

COMMAND(tcl_privmsg)
{
   if(objc == 3)
   {
      SEND("MSG %s %s", STR(1), STR(2));
      return TCL_OK;
   }
   else
   {
      USAGE(1, "target message");
      return TCL_ERROR;
   }
}

COMMAND(tcl_notice)
{
   if(objc == 3)
   {
      SEND("NOTICE %s %s", STR(1), STR(2));
      return TCL_OK;
   }
   else
   {
      USAGE(1, "target message");
      return TCL_ERROR;
   }
}

COMMAND(tcl_ctcp)
{
   if(objc == 3)
   {
      SEND("CTCP %s %s", STR(1), STR(2));
      return TCL_OK;
   }
   else
   {
      USAGE(1, "target message");
      return TCL_ERROR;
   }
}

COMMAND(tcl_joinchan)
{
   switch(objc)
   {
      case 2:
         SEND("JOIN %s", STR(1));
         return TCL_OK;
      case 3:
         SEND("JOIN %s %s", STR(1), STR(2));
         return TCL_OK;
      default:
         USAGE(1, "chan ?key?");
         return TCL_ERROR;
   }
}

COMMAND(tcl_partchan)
{
   switch(objc)
   {
      case 2:
         SEND("PART %s", STR(1));
         return TCL_OK;
      case 3:
         SEND("PART %s %s", STR(1), STR(2));
         return TCL_OK;
      default:
         USAGE(1, "chan ?reason?");
         return TCL_ERROR;
   }
}          

COMMAND(tcl_topic)
{
   if(objc == 3)
   {
      SEND("TOPIC %s %s", STR(1), STR(2));
      return TCL_OK;
   }

   USAGE(1, "chan topic");
   return TCL_ERROR;
}

COMMAND(tcl_mode)
{
   switch(objc)
   {
      case 2:
         SEND("MODE %s", STR(1));
         return TCL_OK;
      case 3:
         SEND("MODE %s %s", STR(1), STR(2));
         return TCL_OK;
      case 4:
         SEND("MODE %s %s %s", STR(1), STR(2), STR(3));
         return TCL_OK;
      default:
         USAGE(1, "chan mode ?param?");
         return TCL_ERROR;
   }
}

/* kick chan nick ?message? */
COMMAND(tcl_kick)
{
   switch(objc)
   {
      case 3:
         SEND("KICK %s %s", STR(1), STR(2));
         return TCL_OK;
      case 4:
         SEND("KICK %s %s %s", STR(1), STR(2), STR(3));
         return TCL_OK;
      default:
         USAGE(1, "chan nick ?reason?");
         return TCL_ERROR;
   }
}

COMMAND(tcl_reconnect)
{
   if(objc == 1)
   {
      SEND("RECONNECT");
      return TCL_OK;
   }
   else if(objc == 2)
   {
      SEND("RECONNECT %s", STR(1));
      return TCL_OK;
   }

   USAGE(1, "?message?");
   return TCL_ERROR;
}

COMMAND(tcl_disconnect)
{
   if(objc == 1)
   {
      SEND("DISCONNECT");
      return TCL_OK;
   }
   else if(objc == 2)
   {
      SEND("DISCONNECT %s", STR(1));
      return TCL_OK;
   }

   USAGE(1, "?message?");
   return TCL_ERROR;
}

COMMAND(tcl_irc)
{
   if(objc == 2)
   {
      SEND("IRC %s", STR(1));
      return TCL_OK;
   }

   USAGE(1, "text");
   return TCL_ERROR;
}

/* connectdcc addr port uid */
COMMAND(tcl_connectdcc)
{
   char *addr, addrstr[32];
   int port, uid, r;

   if(objc == 4)
   {
      unsigned int ip, a, b, c, d;

      addr = STR(1);
      port = INT(2);
      uid = INT(3);

      if(port == 0)
      {
         ERROR("connectdcc: invalid port");
         return TCL_ERROR;
      }

      ip = strtoul(addr, NULL, 10);

      a = ip & 0xff000000;
      b = ip & 0x00ff0000;
      c = ip & 0x0000ff00;
      d = ip & 0x000000ff;

      a >>= 24;
      b >>= 16;
      c >>= 8;

      snprintf(addrstr, 32, "%u.%u.%u.%u", a, b, c, d);

      r = connection_open_dcc(connection_get_current(), addr, port, uid);

      if(r == -1)
         R_INT(0);
      else
         R_INT(1);
      return TCL_OK;
   }

   USAGE(1, "addr port userid");
   return TCL_ERROR;
}

/* connect name addr port nick [username] [realname] */
COMMAND(tcl_connect)
{
   char *name, *addr, *nick, *username, *realname;
   int port, r;

   switch(objc)
   {
      case 5:
         name = STR(1);
         addr = STR(2);
         port = INT(3);
         nick = STR(4);
         username = default_username;
         realname = default_realname;
         break;
      case 6:
         name = STR(1);
         addr = STR(2);
         port = INT(3);
         nick = STR(4);
         username = STR(5);
         realname = default_realname;
         break;
      case 7:
         name = STR(1);
         addr = STR(2);
         port = INT(3);
         nick = STR(4);
         username = STR(5);
         realname = STR(6);
         break;
      default:
         USAGE(1, "name addr port nick ?username realname?");
         return TCL_ERROR;
   }

   r = connection_open(name, addr, port, nick, username, realname,
      connection_get_prefix(-1));

   if(r == -1)
      R_INT(0);
   else
      R_INT(1);

   return TCL_OK;
}

COMMAND(tcl_irctarget)
{
   if(objc == 1)
   {
      target[0] = 0;
      return TCL_OK;
   }
   else
   {
      strncpy(target, STR(1), 32);
      return TCL_OK;
   }

   USAGE(1, "?target?");
   return TCL_ERROR;
}

