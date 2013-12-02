# notice <target> <text>
# Send a notice to a channel or nick

   rmode

   if {$args < 2} {
      notice $nick "Usage: notice <target> <text>"
      return
   }

   set chan [lindex $param 0]
   set text [join [lrange $param 1 end]]

   if {![checkmode $uid $chan amw]} {
      rmode aw
   }

   notice $chan $text

