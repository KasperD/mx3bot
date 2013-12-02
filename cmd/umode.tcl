# umode [handle]
# Display modes set for a user

   if {$args > 1} {
      notice $nick {Usage: umode [handle]}
      return
   }

   if {$args == 0} {
      rmode

      set id $uid
   } else {
      set handle [lindex $param 0]
      set id [user getid $handle]

      if {$id == 0} {
         notice $nick "`$handle' is not a valid user."
         return
      }
   }

   set mode [getmode $id -]

   if {[string length $mode] > 0} {
      notice $nick "`[user getname $id]' has +$mode."
   } else {
      notice $nick "`[user getname $id]' has no modes set."
   }

