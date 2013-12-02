# note <handle> <note>
# Leave a note for a user

   rmode

   if {$args < 2} {
      notice $nick "Usage: note <handle> <note>"
      return
   }

   set handle [lindex $param 0]
   set note [join [lrange $param 1 end]]

   set id [user getid $handle]

   if {$id == 0} {
      notice $nick "`$handle' is not a valid user."
      return
   }

   if {![checkmode $uid - n]} {
      notice $nick "You may not send notes."
      return
   }

   if {![checkmode $id - n]} {
      notice $nick "$handle may not receive notes."
      return
   }

   addnote $id $uid $note

   notice $nick "Note left for $handle."

