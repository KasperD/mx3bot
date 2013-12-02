# stats <type>
# Display various statistics about bot

   rmode

   if {$args < 1} {
      notice $nick "Usage: stats (tcl|users|db|version)"
      return
   }

   switch [lindex $param 0] {
      tcl {
         notice $nick "Using Tcl $tcl_patchLevel."
      }
      users {
         notice $nick "There are [user count] users."
      }
      db {
         notice $nick "[dbqueries] queries, [dbfetches] rows."
      }
      version {
         notice $nick "[getversion]"
      }
      udebug {
         rmode aw
         user_debug
      }
      default {
         notice $nick "Usage: stats (tcl|users)"
      }
   }
