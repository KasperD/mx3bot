# addmask <handle> <mask>
# Adds a mask to an existing user

   rmode aw

   if {$args != 2} {
      notice $nick "Usage: addmask <handle> <mask>"
      return
   }

   set handle [lindex $param 0]
   set mask   [lindex $param 1]

   set id [user getid $handle]

   if {$id == 0} {
      notice $nick "User `$handle' was not found."
      return
   }

   if {[checkmode $id - aw]} {
      rmode w
   }

   if [user addmask $id $mask] {
      notice $nick "Could not add `$mask' to `$handle'."
   } else {
      notice $nick "Added mask `$mask' to `$handle'."
   }

