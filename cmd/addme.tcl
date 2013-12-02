# addme <name>
# Add yourself to the bot's user database

   # Remove these three lines to allow users to add themselves.
   notice $nick "Automatic user account creation is disabled."
   notice $nick "Please talk to this bot's owner for help."
   return

   if {$uid != 0} {
      set handle [user getname $uid]
      notice $nick "You are already on the userlist as `$handle'."
      return
   }

   if {$args != 1} {
      notice $nick "Usage: addme <username>"
      return
   }

   set handle [lindex $param 0]

   if {[user getid $handle] != 0} {
      notice $nick "The handle `$handle' is in use."
      return
   }

   set id [user add $handle [createmask "$nick!$user@$host"]]
   notice $nick "You have been added to the user database as `$handle'."

   set modes [string map {a "" w ""} [getvar 0 "default-umode" ""]]

   if {[string length $modes] > 0} {
      for {set i 0} {$i < [string length $modes]} {incr i} {
         setmode $id - +[string index $modes $i]
      }
   }
