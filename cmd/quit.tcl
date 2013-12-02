# quit [message]
# Completely shutdown bot

   rmode w

   if {$args == 0} {
      shutdown
   } else {
      shutdown [join $param]
   }

