# setmode <handle> <channel> <mode> [param]
# Set a user/channel mode

   rmode

   if {$args < 3} {
      notice $nick {Usage: setmode <user> <channel> <mode> [param]}
      return
   }

   set handle [lindex $param 0]
   set chan [lindex $param 1]
   set mode [lindex $param 2]
   set param [join [lrange $param 3 end]]

   set id [user getid $handle]

   if {$id == 0} {
      notice $nick "`$handle' is not a valid user."
      return
   }

   if {![string match {[#&!+]*} $chan]} {
      notice $nick "Invalid channel `$chan'."
      return
   }

   if {![regexp {^[+\-][a-zA-Z]$} $mode]} {
      notice $nick "Mode must be in the form +m or -m."
      return
   }

   if {![checkmode $uid $chan aw]} {
      rmode aw
   }

   if {[checkmode $id $chan aw]} {
      if {![checkmode $uid - aw]} {
         rmode w $chan
      }
   }

   setmode $id $chan $mode $param

   notice $nick "Set $mode to `$handle' on `$chan'."

