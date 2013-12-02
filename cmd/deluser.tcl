# deluser <handle>
# Remove a user from the database

   rmode aw

   if {$args != 1} {
      notice $nick "Usage: deluser <handle>"
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

   notice $nick "Deleted user `[user getname $id]'."
   user del $id

