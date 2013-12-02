# auth <handle> <password>
# Authenticate to an account

   if {$target != $botnick} {
      notice $nick "You must send `auth' directly via a message."
      return
   }

   if {$args != 2} {
      notice $nick "Usage: /msg $botnick auth <handle> <password>"
      return
   }

   set handle [lindex $param 0]
   set pass   [lindex $param 1]

   set uid    [user getid $handle]

   if {$uid == 0} {
      notice $nick "User `$handle' was not found."
      return
   }

   if {![user haspass $uid]} {
      notice $nick "You have not set a password."
      return
   }

   set found 0
   set list [user getmasklist $uid]
   set ident "$nick!$user@$host"

   foreach mid $list {
      if [match [user getmask $mid] $ident] {
         set found 1
      }
   }

   if {$found == 0} {
      notice $nick "You do not match any of $handle's masks."
      return
   }

   if [user checkpass $uid $pass] {
      notice $nick "You have been authorized."
      setauth $nick $uid
   } else {
      notice $nick "Authorization failed."
   }

