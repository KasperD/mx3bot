# whois <nick>
# Get a user handle from a nick

   rmode

   if {$args != 1} {
      notice $nick "Usage: whois <nick>"
      return
   }

   set n [lindex $param 0]
   set uid [getuser $n]

   if {$uid == 0} {
      notice $nick "I don't know who `$n' is."
      return
   }

   notice $nick "I know `$n' as `[user getname $uid]'."
