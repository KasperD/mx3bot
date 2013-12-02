# getu <handle> <name>
# Get the value of a user variable

   rmode aw

   if {$args != 2} {
      notice $nick "Usage: getu <handle> <name>"
      return
   }

   set handle [lindex $param 0]
   set uid [user getid $handle]

   if {$uid == 0} {
      notice $nick "`$handle' is not a valid user."
      return
   }

   set name [lindex $param 1]
   set val [getvar $uid $name]

   if {[llength $val] == 0} {
      notice $nick "$handle->`$name' is not set."
   } else {
      notice $nick "$handle->`$name' is set to `$val'."
   }

