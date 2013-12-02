# action <target> <param>
# Performs a CTCP Action (/me)

   rmode

   if {$args < 2} {
      notice $nick "Usage: action <target> <text>"
      return
   }

   set chan [lindex $param 0]
   set text [join [lrange $param 1 end]]

   if {[checkmode $uid $chan amw] ||
       [checkmode $uid - aw]} {
      ctcp $chan "ACTION $text"
   } else {
      notice $nick "Access Denied"
   }

