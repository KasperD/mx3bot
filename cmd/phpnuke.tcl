# phpnuke (hide)
# Display news from a phpnuke site

# This will connect to a nearby MySQL server and pull the latest article
# from a phpnuke database.
# Change host/user/pass/name as according to phpnuke's config.php.

   return

   set host "localhost"
   set user "phpnuke"
   set pass "secret"
   set name "phpnuke"
   set delay 15

   if {[checkmode $uid - x]} {
      return
   }

   if {![info exists last_phpnuke]} {
      set last_phpnuke 0
   }

   if {[expr {[getsec] - $last_phpnuke}] < $delay} {
      return
   }

   set last_phpnuke [getsec]

   if {![phpnuke connect $host $user $pass $name]} {
      debug "Cannot connect to phpnuke database."
      return
   }

   set story [phpnuke getstory latest]

   set sid	[lindex $story 0]
   set author	[lindex $story 1]
   set title	[lindex $story 2]
   set date	[lindex $story 3]
   set comments	[lindex $story 4]
   set reads	[lindex $story 5]
   set text	[lindex $story 6]

   regsub -all {<.+?>} $text {} text

   set response "\002$title\002 posted \002$date\002 by \002$author\002"
   append response " ($comments comments, $reads reads)"

   privmsg $reply $response
   privmsg $reply "\002\002$text"

   phpnuke disconnect
