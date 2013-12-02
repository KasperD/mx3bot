# setumode <handle> <mode> [param]
# Set a user mode

   rmode aw

   if {$args < 2} {
      notice $nick {Usage: setumode <user> <mode> [param]}
      return
   }

   set handle [lindex $param 0]
   set mode [lindex $param 1]
   set param [join [lrange $param 2 end]]

   set id [user getid $handle]

   if {$id == 0} {
      notice $nick "`$handle' is not a valid user."
      return
   }

   if {![regexp {^[+\-][a-zA-Z]$} $mode]} {
      notice $nick "Mode must be in the form +m or -m."
      return
   }

   if {[checkmode $id - w]} {
      rmode w
   }

   setmode $id - $mode $param

   notice $nick "Set $mode to `$handle'."
