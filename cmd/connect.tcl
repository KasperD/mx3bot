# connect <name> <server> <port> <nick> [username] [realname]
# Connect to another IRC server

   rmode w

   if {$args < 4} {
      notice $nick "Usage: connect <name> <server> <port> <nick>"
      return
   }

   set name [lindex $param 0]
   set addr [lindex $param 1]
   set port [lindex $param 2]
   set botnick [lindex $param 3]

   if {![string is integer $port]} {
      notice $nick "Invalid port `$port'."
      return
   }

   if {$args == 4} {
      connect $name $addr $port $botnick
   } else {
      set username [lindex $param 4]

      if {$args == 5} {
         connect $name $addr $port $botnick $username
      } else {
         set realname [join [lrange $param 5 end]]
         connect $name $addr $port $botnick $username $realname
      }
   }

   notice $nick "Connecting to `$addr:$port' as `$botnick'."
