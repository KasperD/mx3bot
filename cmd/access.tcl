# access [handle] [channel]
# Display modes that a user holds in a channel

   if {$args > 2} {
      notice $nick {Usage: access [handle] [channel]}
      return
   }

   # no parameters: use current user, channel
   if {$args == 0} {
      rmode

      set id $uid
      set name $nick

      if {![string match {[!#&+]*} $target]} {
         notice $nick "Please specify a channel."
         return
      }
      set chan $target
   }

   set p [lindex $param 0]

   if {[expr $args == 1]} {
      if [string match {[!#&+]*} $p] {
         # one parameter (channel)
         rmode

         set id $uid
         set name $nick
         set chan $p
      } else {
         # one parameter (handle)
         if {![string match {[!#&+]*} $target]} {
            notice $nick "Please specify a channel."
            return
         }

         set id [user getid $p]
         set name $p
         set chan $target
      }
   }

   if {$args == 2} {
      set name [lindex $param 0]
      set id [user getid $name]
      set chan [lindex $param 1]
   }

   if {$id == 0} {
      notice $nick "User `$name' was not found."
      return
   }

   set mode [getmode $id $chan]

   if {[string length $mode] > 0} {
      notice $nick "`[user getname $id]' has +$mode on `$chan'."
   } else {
      notice $nick "`[user getname $id]' has no modes set on `$chan'."
   }

