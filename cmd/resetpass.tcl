# resetpass <handle>
# Clear a user's password

   rmode aw

   if {$args != 1} {
      notice $nick "Usage: resetpass <handle>"
      return
   }

   set handle [lindex $param 0]
   set id [user getid $handle]

   if {$id == 0} {
      notice $nick "User `$handle' was not found."
      return
   }

   if {[checkmode $id - aw]} {
      rmode w
   }

   if {[user setpass $id "-"]} {
      notice $nick "Could not clear password."
   } else {
      notice $nick "Password cleared."
   }
