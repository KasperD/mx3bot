# status [handle]
# Display a user's current status

   if {$args > 1} {
      notice $nick "Usage: status \[handle\]"
      return
   }

   if {$args == 1} {
      set handle [lindex $param 0]
      set id [user getid $handle]

      if {$id == 0} {
         notice $nick "`$handle' is not a valid user."
         return
      }
   } else {
      rmode

      set id $uid
      set handle [user getname $id]
   }

   set status [getvar $id "status"]
   set time [timestr [getvar $id "status-time"]]

   if {[string length $status] == 0} {
      notice $nick "`$handle' has not set a status."
   } else {
      notice $nick "Status: $status ($time)"
   }
