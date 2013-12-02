# away [reason]
# Set your status to `away'.

   rmode

   if {$args > 0} {
      set status [join $param]
   } else {
      set status "away"
   }

   setvar $uid "status" $status
   setvar $uid "status-time" [getsec]

   notice $nick "Status set to `$status'."


