# notes
# Read any notes waiting for you

   rmode

   set list [getnotelist $uid 0]

   if {[llength $list] == 0} {
      notice $nick "You have no notes waiting."
      return
   }

   foreach noteid $list {
      set note [split [getnote $noteid]]
      delnote $noteid

      set from [user getname [lindex $note 1]]
      set sent [timestr [lindex $note 2]]
      set msg [join [lrange $note 3 end]]

      notice $nick "$sent $from: $msg"
   }
