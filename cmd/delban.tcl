# delban [chan] <nick|mask>
# Remove a channel ban

   rmode

   if {[expr $args < 1] || [expr $args > 2]} {
      notice $nick "Usage: delban \[chan\] <nick|mask>"
      return
   }

   set chan [lindex $param 0]

   if {[expr $args > 1] && [string match {[!#&+]*} $chan]} {
      set mask [lindex $param 1]
   } else {
      if {![string match {[!#&+]*} $target]} {
         notice $nick "Please specifiy a channel."
         return
      } else {
         set chan $target
         set mask [lindex $param 0]
      }
   }

   if [string match {[!#&+]*} $mask] {
      notice $nick "Usage: delban \[chan\] <nick|mask>"
      return
   }

   if {![checkmode $uid $chan abw]} {
      rmode aw
      return
   }

   if {[expr {[string first "!" $mask] != -1}] ||
       [expr {[string first "@" $mask] != -1}] ||
       [expr {[string first "*" $mask] != -1}]} {
      set bid [getbanid -mask $chan $mask]

      if {$bid == 0} {
         notice $nick "No bans matching `$mask' on `$chan'."
         return
      }

      delban $bid
      notice $nick "Removed ban `$mask' on `$chan'."

   } else {
      set ident [getident $mask]

      if {[string length $ident] == 0} {
         notice $nick "I don't know who `$mask' is."
         return
      }

      set bans [getbanlist $chan $ident]
      set len [llength $bans]

      if {$len == 0} {
         notice $nick "No bans matching `$ident' on `$chan'."
      } else {
         for {set i 0} {$i < $len} {incr i} {
            delban [lindex $bans $i]
         }

         if [string equal [getmodeparam 0 $chan "s"] "srvx"] {
            privmsg "ChanServ" "ub $chan $mask"
         }

         notice $nick "Removed all bans matching `$ident' on `$chan'."
      }
   }
