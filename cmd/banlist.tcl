# banlist [chan]
# Get a listing of bans for a specific channel

   if {$nick != "=dcc"} {
      notice $nick "You must DCC to use this command."
      return
   }

   if {$args == 0} {
      if {![string match {[!#&+]*} $target]} {
         notice $nick "Please specify a channel."
         return
      }

      set chan $target
   } else {
      set chan [lindex $param 0]
   }

   if {![checkmode $uid $chan abw]} {
      rmode aw
   }

   set bans [getbanlist $chan]
   set len [llength $bans]

   if {$len == 0} {
      notice $nick "There are no bans on `$chan'."
   } else {
      notice $nick "Bans on `$chan': ([llength $bans])"

      for {set i 0} {$i < $len} {incr i} {
         set ban [split [getban [lindex $bans $i]]]

         set author [lindex $ban 0]
         set mask [lindex $ban 1]
         set note [join [lrange $ban 2 end]]

         if {[string length $note] == 0} {
            if {$author == 0} {
               notice $nick "  $mask"
            } else {
               notice $nick "  $mask ([user getname $author])"
            }
         } else {
            if {$author == 0} {
               notice $nick "  $mask ($note)"
            } else {
               notice $nick "  $mask ([user getname $author]: $note)"
            }
         }
      }
   }
