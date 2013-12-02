# help_static.tcl: included in help.tcl by misc/buildhelp.tcl

   set usage(help) {help <command>}
   set  desc(help) {Get help on a specified command or topic}

   set  desc(user) "If you are a new user, try `/msg $botnick help addme'."
   append desc(user) "  If you've set a password, try `/msg $botnick "
   append desc(user) "help auth'."

   if {$args != 1} {
      notice $nick "Usage: help <command>"
      return
   }

   set topic [string tolower [lindex $param 0]]

   if [info exists desc($topic)] {
      notice $nick "$topic: $desc($topic)"

      if {[info exists usage($topic)]} {
         notice $nick "Usage: $usage($topic)"
      }
   } else {
      notice $nick "No help for `$topic'."
   }
