# addban [chan] <nick|mask> [reason]
# Adds a permanent ban to a channel

   rmode

   if {$args < 1} {
      notice $nick "Usage: addban \[chan\] <nick|mask> \[reason\]"
      return
   }

   set chan [lindex $param 0]

   if {[expr $args > 1] && [string match {[!#&+]*} $chan]} {
      set mask [lindex $param 1]
      set note [join [lrange $param 2 end]]
   } else {
      if {![string match {[!#&+]*} $target]} {
         notice $nick "Please specifiy a channel."
         return
      } else {
         set chan $target
         set mask [lindex $param 0]
         set note [join [lrange $param 1 end]]
      }
   }

   if [string match {[!#&+]*} $mask] {
      notice $nick "Usage: addban \[chan\] <mask> \[reason\]"
      return
   }

   if {![checkmode $uid $chan abw]} {
      rmode aw
   }

   if {[expr {[string first "!" $mask] != -1}] ||
       [expr {[string first "@" $mask] != -1}] ||
       [expr {[string first "*" $mask] != -1}]} {
      if {[string equal [getmodeparam 0 $chan "s"] "srvx"]} {
         privmsg "ChanServ" "kb $chan $mask $note"
      } else {
         mode $chan "+b" $mask
      }
   } else {
      set victim $mask
      set ident [getident $mask]

      if {[string length $ident] == 0} {
         notice $nick "I don't know who `$mask' is."
         return
      }

      set mask [createmask $ident]

      if [string equal [getmodeparam 0 $chan "s"] "srvx"] {
         privmsg "ChanServ" "kb $chan $victim $note"
      } else {
         mode $chan "+b" $mask
      }
   }

   if {[string length $note] > 0} {
      addban $chan $mask $uid $note
   } else {
      addban $chan $mask $uid
   }

   notice $nick "Banning `$mask' on `$chan'."
