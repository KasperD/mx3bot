# help <command>
# Get help on a specified topic
#
# Generated on Sat Jul 20 23:13:28 EDT 2002 by buildhelp.tcl
#

   set usage(access) {access [handle] [channel]}
   set  desc(access) {Display modes that a user holds in a channel}
   set usage(action) {action <target> <param>}
   set  desc(action) {Performs a CTCP Action (/me)}
   set usage(addban) {addban [chan] <nick|mask> [reason]}
   set  desc(addban) {Adds a permanent ban to a channel}
   set usage(addmask) {addmask <handle> <mask>}
   set  desc(addmask) {Adds a mask to an existing user}
   set usage(addme) {addme <name>}
   set  desc(addme) {Add yourself to the bot's user database}
   set usage(adduser) {adduser <handle> <mask>}
   set  desc(adduser) {Adds a user to the bot's database}
   set usage(auth) {auth <handle> <password>}
   set  desc(auth) {Authenticate to an account}
   set usage(away) {away [reason]}
   set  desc(away) {Set your status to `away'.}
   set usage(back) {back}
   set  desc(back) {Set your status to `around'.}
   set usage(banlist) {banlist [chan]}
   set  desc(banlist) {Get a listing of bans for a specific channel}
   set usage(cmode) {cmode [<channel> [mode]]}
   set  desc(cmode) {Set channel modes}
   set usage(connect) {connect <name> <server> <port> <nick> [username] [realname]}
   set  desc(connect) {Connect to another IRC server}
   set usage(ctcp) {ctcp <target> <text>}
   set  desc(ctcp) {Send a CTCP to a channel}
   set usage(delban) {delban [chan] <nick|mask>}
   set  desc(delban) {Remove a channel ban}
   set usage(delmask) {delmask <handle> <mask>}
   set  desc(delmask) {Remove a mask from a user}
   set usage(deluser) {deluser <handle>}
   set  desc(deluser) {Remove a user from the database}
   set usage(disconnect) {disconnect [message]}
   set  desc(disconnect) {Disconnect from the current server}
   set usage(fmsg) {fmsg <connection> <target> <text>}
   set  desc(fmsg) {Send a message to another server}
   set usage(getg) {getg <name>}
   set  desc(getg) {Get the value of a global variable}
   set usage(getu) {getu <handle> <name>}
   set  desc(getu) {Get the value of a user variable}
   set usage(inviteme) {inviteme <chan>}
   set  desc(inviteme) {Instruct bot to invite you to a channel}
   set usage(join) {join <channel> [key]}
   set  desc(join) {Join a new channel}
   set usage(mode) {mode <chan> <mode> [<param>]}
   set  desc(mode) {Perform an IRC mode command}
   set usage(msg) {msg <target> <text>}
   set  desc(msg) {Send a message to a channel or nick}
   set usage(note) {note <handle> <note>}
   set  desc(note) {Leave a note for a user}
   set usage(notes) {notes}
   set  desc(notes) {Read any notes waiting for you}
   set usage(notice) {notice <target> <text>}
   set  desc(notice) {Send a notice to a channel or nick}
   set usage(part) {part [chan] [message]}
   set  desc(part) {Part a channel}
   set usage(ping) {ping <nick>}
   set  desc(ping) {Check CTCP ping time between bot and another user}
   set usage(quit) {quit [message]}
   set  desc(quit) {Completely shutdown bot}
   set usage(quote) {quote <text>}
   set  desc(quote) {Send raw text to server}
   set usage(reconnect) {reconnect [reason]}
   set  desc(reconnect) {Disconnect and reconnect to server}
   set usage(resetpass) {resetpass <handle>}
   set  desc(resetpass) {Clear a user's password}
   set usage(setcmode) {setcmode <channel> <mode> [param]}
   set  desc(setcmode) {Set a channel mode}
   set usage(setcp) {setcp <prefix>}
   set  desc(setcp) {Set bot's public command prefix}
   set usage(setg) {setg <name> <val>}
   set  desc(setg) {Set a global variable}
   set usage(setinfo) {setinfo [chan] <text>}
   set  desc(setinfo) {Set an info line}
   set usage(setmode) {setmode <handle> <channel> <mode> [param]}
   set  desc(setmode) {Set a user/channel mode}
   set usage(setnick) {setnick <nick>}
   set  desc(setnick) {Set bot's nick}
   set usage(setpass) {setpass <password>}
   set  desc(setpass) {Set a password for your account}
   set usage(setu) {setu <handle> <name> <val>}
   set  desc(setu) {Set a user variable}
   set usage(setumode) {setumode <handle> <mode> [param]}
   set  desc(setumode) {Set a user mode}
   set usage(stats) {stats <type>}
   set  desc(stats) {Display various statistics about bot}
   set usage(status) {status [handle]}
   set  desc(status) {Display a user's current status}
   set usage(topic) {topic [chan] <topic>}
   set  desc(topic) {Set the topic in a channel}
   set usage(uinfo) {uinfo [handle|#id]}
   set  desc(uinfo) {Display detailed information about a given user}
   set usage(umode) {umode [handle]}
   set  desc(umode) {Display modes set for a user}
   set usage(unsetg) {unsetg <name>}
   set  desc(unsetg) {Unset a global variable}
   set usage(unsetu) {unsetu <handle> <name>}
   set  desc(unsetu) {Unset a user variable}
   set usage(whois) {whois <nick>}
   set  desc(whois) {Get a user handle from a nick}
# help_static.tcl: included in help.tcl by misc/buildhelp.tcl

   set usage(help) {help <command>}
   set  desc(help) {Get help on a specified command or topic}

   set  desc(user) "If you are a new user, try `/msg $botnick help addme'."
   append desc(user) "  If you've set a password, try `/msg $botnick "
   append desc(user) "help auth'."

   if {$args != 1} {
      notice $nick "Usage: help <command>"
      return
   }

   set topic [string tolower [lindex $param 0]]

   if [info exists desc($topic)] {
      notice $nick "$topic: $desc($topic)"

      if {[info exists usage($topic)]} {
         notice $nick "Usage: $usage($topic)"
      }
   } else {
      notice $nick "No help for `$topic'."
   }

