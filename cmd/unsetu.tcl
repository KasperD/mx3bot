# unsetu <handle> <name>
# Unset a user variable

   rmode aw

   if {$args != 2} {
      notice $nick "Usage: unsetu <handle> <name>"
      return
   }

   set handle [lindex $param 0]
   set id [user getid $handle]

   if {$id == 0} {
      notice $nick "`$handle' is not a valid user."
      return
   }

   if {[checkmode $id - aw]} {
      rmode w
   }

   set name [lindex $param 1]

   unsetvar $id $name

   notice $nick "$handle->`$name' has been cleared."
