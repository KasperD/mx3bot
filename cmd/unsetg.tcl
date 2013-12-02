# unsetg <name>
# Unset a global variable

   rmode aw

   if {$args != 1} {
      notice $nick "Usage: unsetg <name>"
      return
   }

   set name [lindex $param 0]
   unsetvar 0 $name

   notice $nick "`$name' has been cleared."
