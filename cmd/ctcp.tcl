# ctcp <target> <text>
# Send a CTCP to a channel

   rmode

   if {$args < 2} {
      notice $nick "Usage: ctcp <target> <text>"
      return
   }

   set chan [lindex $param 0]
   set text [join [lrange $param 1 end]]

   if {![checkmode $uid $chan amw]} {
      rmode aw
   }

   ctcp $chan $text

