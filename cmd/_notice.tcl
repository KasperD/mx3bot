# Called when the bot receives a notice.  Useful if you need to
#   parse error messages from another bot or channel service.

   # Example: srvx authentication
   if {[string equal $host "Services.GamesNET.net"] &&
       [string match "You must first authenticate*" [join $param]]} {

      # You would need to set these via "setg gamesnet-handle bots_handle"
      set handle [getvar 0 "gamesnet-handle"]
      set pass   [getvar 0 "gamesnet-password"]

      privmsg "AuthServ@Services.GamesNET.net" "auth $handle $pass"
   }
