# setpass <password>
# Set a password for your account

   rmode

   if {$args != 1} {
      notice $nick "Usage: setpass <password>"
      return
   }

   set pass [lindex $param 0]

   set uid [user getid -ident "$nick!$user@$host"]

   if {$uid == 0} {
      notice $nick "user not found"
      return
   }

   if [user setpass $uid $pass] {
      notice $nick "Could not set password."
   } else {
      if {[string equal $pass "-"]} {
         notice $nick "Password has been cleared."
      } else {
         notice $nick "Password has been set."
      }
   }

