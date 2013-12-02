# quote <text>
# Send raw text to server

   rmode w

   if {$args < 1} {
      notice $nick "Usage: quote <text>"
      return
   }

   irc [join $param]

