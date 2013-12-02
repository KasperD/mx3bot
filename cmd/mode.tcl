# mode <chan> <mode> [<param>]
# Perform an IRC mode command

   rmode

   if {$args < 2} {
      notice $nick "Usage: mode <chan> <mode> <param>"
      return
   }

   set chan [lindex $param 0]
   set mode [lindex $param 1]
   set param [join [lrange $param 2 end]]

   if {![checkmode $uid $chan aw]} {
      rmode aw
   }

   mode $chan $mode $param

