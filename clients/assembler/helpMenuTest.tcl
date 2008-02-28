#  Test the help menu


menu .menubar 
menu .menubar.help -tearoff 0
.menubar add cascade -label Help -menu .menubar.help 

source helpMenu.tcl
package require evbHelpMenu


set hm [helpMenu %AUTO% -menu .menubar.help]


$hm configure -abouttext "This would be some profound about text \n Version 1.0"


. configure -menu .menubar

