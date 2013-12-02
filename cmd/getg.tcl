# getg <name>
# Get the value of a global variable

   rmode aw

   if {$args != 1} {
      notice $nick "Usage: getg <name>"
      return
   }

   set name [lindex $param 0]
   set val  [getvar 0 $name]

   if {[llength $val] == 0} {
      notice $nick "`$name' is not set."
   } else {
      notice $nick "`$name' is set to `$val'."
   }
