# join <channel> [key]
# Join a new channel

   rmode

   if {[expr $args < 1] || [expr $args > 2]} {
      notice $nick "Usage: join <channel> \[key\]"
      return
   }

   set chan [lindex $param 0]

   if {![checkmode $uid $chan ajw]} {
      rmode ajw
   }

   if {$args == 1} {
      joinchan $chan
   } else {
      joinchan $chan [lindex $param 1]
   }
