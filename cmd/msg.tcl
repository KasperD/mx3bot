# msg <target> <text>
# Send a message to a channel or nick

   rmode

   if {$args < 2} {
      notice $nick "Usage: msg <target> <text>"
      return
   }

   set chan [lindex $param 0]
   set text [join [lrange $param 1 end]]

   if {![checkmode $uid $chan amw]} {
      rmode aw
   }

   privmsg $chan $text
