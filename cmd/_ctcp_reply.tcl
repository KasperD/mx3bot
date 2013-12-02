# Called when a CTCP reply is received.

   if [string equal $ctcp "PING"] {
      set name [lindex $param 0]
      set ping [getvar 0 $name]
      unsetvar 0 $name

      if {[llength $ping] != 3} {
         return
      }

      set target [lindex $ping 0]
      set then "[lindex $ping 1].[lindex $ping 2]"
      set now "[getsec].[getusec]"

      set time [format "%.3f" [expr {$now - $then}]]

      notice $target "Ping Reply from $nick: $time"
   }
