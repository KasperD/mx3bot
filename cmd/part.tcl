# part [chan] [message]
# Part a channel

   rmode

   if {$args == 0} {
      if {![string match {[!#&+]*} $target]} {
         notice $nick "Please specify a channel."
         return
      }

      set chan $target
      set msg ""
   } else {
      set chan [lindex $param 0]

      if {[expr $args > 0] && [string match {[!#&+]*} $chan]} {
         set msg [join [lrange $param 1 end]]
      } else {
         if {![string match {[!#&+]*} $target]} {
            notice $nick "Please specify a channel."
            return
         } else {
            set chan $target
            set msg [join $param]
         }
      }
   }

   if {![checkmode $uid $chan ajw]} {
      rmode aw
   }

   if {[string length $msg] > 0} {
      partchan $chan $msg
   } else {
      partchan $chan
   }
