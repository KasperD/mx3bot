# This will be called when the bot connects or reconnects to a server.

   # Join all channels with +a (auto-join)
   set channels [getmodelist "a"]

   foreach chan $channels {
      set key [getmodeparam 0 $chan "a"]

      if {[string length $key] > 0} {
         joinchan $chan $key
      } else {
         joinchan $chan
      }
   }

