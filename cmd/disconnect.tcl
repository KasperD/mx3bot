# disconnect [message]
# Disconnect from the current server

   rmode w

   if {$args < 1} {
      disconnect
   } else {
      disconnect [join $param]
   }
