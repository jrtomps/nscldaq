package require Tk


menu .menubar
menu .menubar.file -tearoff 0
.menubar add cascade -label File -menu .menubar.file

source fileMenu.tcl
package require evbFileMenu

set configuration {someConfig}

set fm [fileMenu %AUTO% -menu .menubar.file -configvar configuration]


. configure  -menu .menubar

entry .config -textvariable configuration
pack .config
