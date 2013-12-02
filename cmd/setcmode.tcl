# setcmode <channel> <mode> [param]
# Set a channel mode

   rmode

   if {$args < 2} {
      notice $nick {Usage: setcmode <channel> <mode> [param]}
      return
   }

   set chan [lindex $param 0]
   set mode [lindex $param 1]
   set param [join [lrange $param 2 end]]

   if {![string match {[#&!+]*} $chan]} {
      notice $nick "Invalid channel `$chan'."
      return
   }

   if {![regexp {^[+\-][a-zA-Z]$} $mode]} {
      notice $nick "Mode must be in the form +m or -m."
      return
   }

   if {![checkmode 0 $chan aw]} {
      rmode aw
   }

   setmode 0 $chan $mode $param

   notice $nick "Set $mode to `$chan'."

