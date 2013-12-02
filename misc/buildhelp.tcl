#
# This script reads all tcl files in the `cmd' directory that do not start
# with an underscore and builds a suitable help.tcl from the comments
# on the first two lines, and sends it to the standard output.
#
# Run from the root mx3bot directory.
#
# The first line should contain usage information and the second line
# should contain a short description of the command.
#
# Files without this format will not be included in the help.tcl.
#

   puts "# help <command>"
   puts "# Get help on a specified topic"
   puts "#"
   puts "# Generated on [clock format [clock seconds]] by buildhelp.tcl"
   puts "#"
   puts ""

   set list [lsort [glob cmd/*.tcl]]

   foreach fn $list {
      set command [lindex [split [lindex [split $fn /] end] .] 0]

      if {[regexp "^_" $command]} {
         continue
      }

      if {[string equal $command "help"]} {
         continue
      }

      set f [open "$fn" "r"]
      set line1 [gets $f]
      set line2 [gets $f]
      close $f

      if {[regexp "# *(.+)" $line1 m usage] && 
          [regexp "# *(.+)" $line2 m desc]} {

         if {[regexp {\(hide\) *$} $usage]} {
            continue
         }

         puts "   set usage($command) {$usage}"
         puts "   set  desc($command) {$desc}"
      }
   }

   set f [open "misc/help_static.tcl" r]

   while {![eof $f]} {
      puts [gets $f]
   }

   close $f
