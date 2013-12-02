# delmask <handle> <mask>
# Remove a mask from a user

   rmode aw

   if {$args != 2} {
      notice $nick "Usage: delmask <handle> <mask>"
      return
   }

   set handle [lindex $param 0]
   set mask   [lindex $param 1]

   set id [user getid $handle]

   if {$id == 0} {
      notice $nick "User `$handle' was not found."
      return
   }

   if {[checkmode $uid - aw]} {
      rmode w
   }

   user delmask $id $mask

   notice $nick "Removed `$mask' from `$handle'."
