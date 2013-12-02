# reconnect [reason]
# Disconnect and reconnect to server

   rmode w

   if {$args < 1} {
      reconnect
   } else {
      reconnect [join $param]
   }

