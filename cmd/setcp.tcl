# setcp <prefix>
# Set bot's public command prefix

   rmode aw

   if {$args != 1} {
      notice $nick "Usage: setcp <prefix>"
      return
   }

   set cp [lindex $param 0]

   if {[string length $cp] != 1} {
      notice $nick "Command prefix must be one character."
      return
   }

   notice $nick "Command prefix is now `[setcp $cp]'."
