
   if {$ctcp == "DCC"} {

      if {$args < 4} {
         return
      }

      set type [lindex $param 0]
      set text [lindex $param 1]
      set addr [lindex $param 2]
      set port [lindex $param 3]

      if {$type == "CHAT"} {
         if {[checkmode $uid - adw]} {
            connectdcc $addr $port $uid
         } else {
            notice $nick "I do not accept DCC connections from you."
         }
      }
   }

