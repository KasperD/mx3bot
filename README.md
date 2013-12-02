mx3bot
======

IRC Bot written for the GameSurge network

   $Id: README,v 1.13 2002/07/21 03:08:03 influx Exp $

   Legal:

      Mx3bot: A MySQL/Tcl based IRC robot.
      Copyright 2001-2002 Colin O'Leary

      This program is free software; you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation; either version 2 of the License, or
      (at your option) any later version.

      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.

      You should have received a copy of the GNU General Public License
      along with this program; if not, write to the Free Software
      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

      A copy of the GNU GPL is in doc/GPL.


   Limitations:

      There is no protection scheme yet.

      Some commands could do some better type checking on input.
         Users may receive a Tcl error message with strange input.

      The PHP interface from the older versions will not work right
         now due to the way this version caches its user information
         in src/user.c.

      Not all user-modes/channel-modes/modes are implemented.
         Modes with a dot preceding them in doc/MODES are working.

      HOWEVER!  It should work!  Don't let all that discourage you
         from running the bot.


   Installation:

      Full installation instructions are available in doc/INSTALL.

      Quick Setup:

         $ tar zxvf mx3bot-0.8.x.tar.gz
         $ cd mx3bot-0.8.x/

         $ ./configure --with-mysql

         Don't include --with-mysql if you aren't interested in the
         MySQL database backend.

         $ make
         $ su
         # make install
         # exit
         $ mx3bot --setup

         Edit ~/.mx3bot/config to your liking.  Change `nick', `network',
         and `server'.

         $ mx3bot


   Quick Tutorial:

      You should be able to run the bot.  It will connect to the irc server
      specified in your ~/.mx3bot/config, and will join any channels that
      you have set a +a channel mode to (defaults to none).

      To get it to a useful state:

      (Assuming `!' is your command-prefix)

      (Do this if you have set a password)
      /msg mx3bot auth <username> <password>	Authenticate

      /join #lame
      /msg mx3bot join #lame

      !setpass something			Set yourself a password
      !setmode someguy #lame +o			Make yourself an op here

      Some commands:

      !setumode myhandle +d			Give yourself DCC access

      !adduser friend *!*friend@*.friend.com	Add some users
      !setmode friend #lame +o			Make them powerful

      !setcmode #lame +i			Allow !setinfo
      !setcmode #lame +m This is annoying!	Send this message to people
						when they join #lame

      !setcmode #lame +t Welcome to #lame  ::  $(topic)  ($(user))
      !setmode someguy #lame +t
      !setmode friend #lame +t			This will set the topic on #lame
      !topic #lame Hi.				to "Welcome to #lame  ::  Hi.  (someguy)"

      !note friend Hello friend.		Send a note to friend
      !note someguy Hello self.			Send a note to self
      !notes					Read your notes

      !setg version Lame version message.	Set CTCP VERSION response
      !setg userinfo Even lamer userinfo.	Set CTCP USERINFO response

      !away gone				Set your status to "gone"
      !back					Set your status to "around"
      !status someguy				Display your status

      !whois thatguy				Get the handle from a nick

      !msg #lame Hi.				Talk
      !action #lame waves.			/me
      !notice #lame ...				Notice a pattern?
      !ctcp #lame VERSION			Send a ctcp

      !ping friend				Get ping reply time of friend

      !connect networkname irc.someplace.org 6667 nick
						Connect to irc.someplace.org:6667
						as `nick'.
      !setcp .					Set the command prefix to `.'
						A different prefix may be used
						for each server connection.

      !setcmode <chan> [+-]<mode> [param]	Set a channel mode.
      !setumode <handle> [+-]<mode> [param]	Set a user mode.
      !setmode <handle> <chan> [+-]<mode> [param]  Set a mode.
						See doc/MODES

      All of the commands are in the cmd/ directory, and are called directly
      when that command is to be run.

      Most will give you usage instructions if no arguments are given.

      The !help command is half-implemented, and does not have help for
      most of the commands.


   Contact:

      It would be appreciated if you sent me a note if you're successfully
      (or unsuccesfully) using this program.

      Stop by #Mx3 on irc.gamesnet.net, and talk to influx or vodak.
      There's currently an old Mx3bot or two idling there, with the newer
      development versions usually running on a private ircd somewhere.

      Some kind of bug tracking forum/help forum should be going up on
      http://bot.mx3.org eventually.

      Feel free to email influx@users.sourceforge.net with any comments.
