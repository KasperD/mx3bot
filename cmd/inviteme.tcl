# inviteme <chan>
# Instruct bot to invite you to a channel

   rmode

   if {$args != 1} {
      notice $nick "Usage: inviteme <chan>"
      return
   }

   set chan [lindex $param 0]

   if {[checkmode $uid $chan anow] ||
       [checkmode $uid - aw]} {
      irc "INVITE $nick $chan"
   } else {
      notice $nick "Access Denied."
   }
