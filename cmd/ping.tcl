# ping <nick>
# Check CTCP ping time between bot and another user

   rmode aw

   if {$args != 1} {
      notice $nick "Usage: ping <nick>"
      return
   }

   set ping "$nick [getsec] [getusec]"
   set id [setuvar 0 $ping]

   ctcp [lindex $param 0] "PING $id"
