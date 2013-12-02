# uinfo [handle|#id]
# Display detailed information about a given user

   rmode

   if {$nick != "=dcc"} {
      notice $nick "You must DCC to use this command."
      return
   }

   if {$args > 1} {
      notice $nick "Usage: uinfo \[handle|#id\]"
      return
   }

   if {$args == 1} {
      rmode aw

      set handle [lindex $param 0]

      if {[string index $handle 0] == "#"} {
         set id [string range $handle 1 end]

         if {![string is integer $id]} {
            notice $nick "Invalid id `$handle'."
            return
         }
      } else {
         set id [user getid $handle]
      }

      if {$id == 0} {
         notice $nick "`$handle' is not a valid user."
         return
      }
   } else {
      set id $uid
   }

   notice $nick "Information for `[user getname $id]':"
   notice $nick "     Id: $id"

   set modes [getmode $id -]

   if {[string length $modes] > 0} {
      notice $nick "  Modes: +$modes"
   } else {
      notice $nick "  Modes: (none)"
   }

   if {[user haspass $id]} {
      notice $nick "   Pass: set"
   } else {
      notice $nick "   Pass: not set"
   }

   set masks [user getmasklist $id]

   notice $nick "  Masks: ([llength $masks])"

   foreach mid $masks {
      notice $nick "    [user getmask $mid]"
   }

   set in [llength [getnotelist $id 0]]
   set out [llength [getnotelist 0 $id]]

   notice $nick "  Notes: $in incoming, $out outgoing"

