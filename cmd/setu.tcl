# setu <handle> <name> <val>
# Set a user variable

   rmode aw

   if {$args < 3} {
      notice $nick "Usage: setu <handle> <name> <val>"
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
   set val  [join [lrange $param 2 end]]

   setvar $id $name $val

   notice $nick "$handle->`$name' has been set to `$val'."

