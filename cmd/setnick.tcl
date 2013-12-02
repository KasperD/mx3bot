# setnick <nick>
# Set bot's nick

   rmode w

   if {$args != 1} {
      notice $nick "Usage: setnick <nick>"
      return
   }

   setnick [lindex $param 0]
