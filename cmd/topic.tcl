# topic [chan] <topic>
# Set the topic in a channel

   rmode

   if {$args < 1} {
      notice $nick "Usage: topic \[chan\] <topic>"
      return
   }

   set chan [lindex $param 0]

   if {[expr $args > 1] && [string match {[!#&+]*} $chan]} {
      set topic [join [lrange $param 1 end]]
   } else {
      if {![string match {[!#&+]*} $target]} {
         notice $nick "Please specifiy a channel."
         return
      } else {
         set chan $target
         set topic [join $param]
      }
   }

   if {![checkmode $uid $chan atw]} {
      rmode aw
   }

   if {![checkmode 0 $chan "t"]} {
      notice $nick "I don't know how to set a topic on `$chan'."
      return
   }

   set format [getmodeparam 0 $chan "t"]

   regsub {\$\(topic\)} $format $topic format
   regsub {\$\(user\)} $format [user getname $uid] format
   regsub {\$\(nick\)} $format $nick format
   regsub {\$\(time\)} $format [timestr] format

   if [string equal [getmodeparam 0 $chan "s"] "srvx"] {
      privmsg "ChanServ" "topic $chan $format"
   } else {
      topic $chan $format
   }
