# setinfo [chan] <text>
# Set an info line

   rmode

   if {$args < 1} {
      notice $nick "Usage: setinfo \[channel\] <text>"
      return
   }

   set chan [lindex $param 0]

   if {[expr $args > 1] && [string match {[!#&+]*} $chan]} {
      set text [join [lrange $param 1 end]]
   } else {
      if {![string match {[!#&+]*} $target]} {
         notice $nick "Please specifiy a channel."
         return
      } else {
         set chan $target
         set text [join $param]
      }
   }

   if {![checkmode 0 $chan "i"]} {
      notice $nick "Info lines are not allowed in this channel."
      return
   }

   if [string equal $text "-"] {
      setmode $uid $chan "-i"
      notice $nick "Info remove from `$chan'."
   } else {
      setmode $uid $chan "+i" $text
      notice $nick "Info on `$chan' is `$text'."
   }

