# fmsg <connection> <target> <text>
# Send a message to another server

   rmode w

   if {$args < 3} {
      notice $nick "Usage: fmsg <server> <target> <text>"
      return
   }

   set conn [lindex $param 0]
   set chan [lindex $param 1]
   set text [join [lrange $param 2 end]]

   irctarget $conn
   privmsg $chan $text
   irctarget

