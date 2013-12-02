# setg <name> <val>
# Set a global variable

   rmode aw

   if {$args < 2} {
      notice $nick "Usage: setg <name> <val>"
      return
   }

   set name [lindex $param 0]
   set val  [join [lrange $param 1 end]]

   setvar 0 $name $val

   notice $nick "`$name' has been set to `$val'."
