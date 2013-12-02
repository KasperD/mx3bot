# cmode [<channel> [mode]]
# Set channel modes

   if {$args > 2} {
      notice $nick {Usage: cmode [<channel> [mode]]}
      return
   }

   if {$args == 0} {
      set chan $target
      set mode 0

      if {![string match {[!#&+]*} $chan]} {
         notice $nick "Please specify a channel."
         return
      }
   } else {
      set chan [lindex $param 0]

      if {![string match {[!#&+]*} $chan]} {
         notice $nick "`$chan' is not a valid channel."
         return
      }

      if {$args == 2} {
         set mode [lindex $param 1]
      } else { 
         set mode 0
      }
   }

   if {$mode == 0} {
      set modes [getmode 0 $chan]

      if {[string length $modes] > 0} {
         notice $nick "`$chan' has +$modes."
      } else {
         notice $nick "`$chan' has no modes set."
      }
   } else {
      if {[string length $mode] != 1} {
         notice $nick "Invalid mode `$mode'."
         return
      }

      if {![checkmode 0 $chan $mode]} {
         notice $nick "Channel `$chan' does not have +$mode."
         return
      }

      set param [getmodeparam 0 $chan $mode]

      if {[string length $param] > 0} {
         notice $nick "`$chan' has +$mode: `$param'."
      } else {
         notice $nick "`$chan' has +$mode."
      }
   }

