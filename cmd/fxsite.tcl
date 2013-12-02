# fxsite (hide)
# Displays news from an fxSite database.

# fxSite is a PHP based news/forum package that I wrote, that never quite
#   got released.  It runs http://bot.mx3.org.

   return

   set host "localhost"
   set user "fxsite"
   set name "fxsite"
   set pass "secret"
   set section 3
   set delay 15

   if {[checkmode $uid - x]} {
      return
   }

   if {![info exists last_fxsite]} {
      set last_fxsite 0
   }

   if {[expr {[getsec] - $last_fxsite}] < $delay} {
      return
   }

   set last_fxsite [getsec]

   if {![fxsite connect $host $user $pass $name]} {
      debug "Cannot connect to fxSite database."
      return
   }

   set item [fxsite getnews $section]
   fxsite disconnect

   set aid	[lindex $item 0]
   set author	[lindex $item 1]
   set title	[lindex $item 2]
   set date	[lindex $item 3]
   set comments	[lindex $item 4]
   set text	[lindex $item 5]

   regsub -all {<.+?>} $text {} text
   regsub -all {\[.+?\]} $text {} text

   set response "\002$title\002 posted \002$date\002 by \002$author\002"
   append response " ($comments comments)"

   privmsg $reply $response
   privmsg $reply "\002\002$text"
