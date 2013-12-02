# Called when somebody (including the bot itself) joins a channel.
# _join: (chan, nick, user, host, uid)

   # check bans
   if {$nick == $botnick} {
      set bid 0
   } else {
      set bid [getbanid $chan "$nick!$user@$host"]
   }

   set exempt 0

   if {$uid != 0} {
      if {[checkmode $uid - aw] ||
          [checkmode $uid $chan aeow]} {
         set exempt 1
      }
   }

   if {[expr {$bid != 0}] && [expr {$exempt == 0}]} {
      set ban [split [getban $bid]]

      set by [lindex $ban 0]
      set mask [lindex $ban 1]
      set note [join [lrange $ban 2 end]]

      if {$note == ""} {
         set note "Banned"
      }

      mode $chan "+b" $mask

      if {$by == 0} {
         kick $chan $nick $note
      } else {
         kick $chan $nick "[user getname $by]: $note"
      }
   }

   # send join-message
   if {$uid == 0} {
      if [checkmode 0 $chan "m"] {
         notice $nick "[getmodeparam 0 $chan "m"]"
      }
   }

   # remaining commands only for recognized users
   if {$uid == 0} { return }

   # check for waiting notes
   set notes [llength [getnotelist $uid 0]]

   if {$notes > 0} {
      if {$notes == 1} {
         notice $nick "You have one note waiting.  Type $cp[]notes to read it."
      } else {
         notice $nick "You have $notes notes waiting.  Type $cp[]notes to read them."
      }
   }

   # check modes
   if [checkmode $uid $chan aow] {
      mode $chan +o $nick
   }

   if [checkmode $uid $chan v] {
      mode $chan +v $nick
   }

   # send join message
   if [checkmode 0 $chan "m"] {
      notice $nick "[getmodeparam 0 $chan "m"]"
   }

   # setinfo
   if [checkmode $uid $chan "i"] {
      privmsg $chan "\[$nick\] [getmodeparam $uid $chan "i"]"
   }

