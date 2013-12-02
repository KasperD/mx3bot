# adduser <handle> <mask>
# Adds a user to the bot's database

   rmode aw

   if {$args != 2} {
      notice $nick "Usage: adduser <handle> <mask>"
      return
   }

   set handle [lindex $param 0]
   set mask   [lindex $param 1]

   if [user getid $handle] {
      notice $nick "User `$handle' already exists."
      return
   }

   set id [user add $handle $mask]

   if {$id == 0} {
      notice $nick "Could not add user."
      return
   }

   set modes [string map {a "" w ""} [getvar 0 "default-umode" ""]]

   if {[string length $modes] > 0} {
      for {set i 0} {$i < [string length $modes]} {incr i} {
         setmode $id - +[string index $modes $i]
      }

      notice $nick "Added `$handle' with modes +$modes."
   } else {
      notice $nick "Added `$handle'."
   }

