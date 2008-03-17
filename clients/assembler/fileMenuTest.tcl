package require Tk

proc callback {why widget variable} {
    puts "$why Callback $widget $variable"
}


menu .menubar
menu .menubar.file -tearoff 0
.menubar add cascade -label File -menu .menubar.file

source fileMenu.tcl
package require evbFileMenu

set configuration {someConfig}

set fm [fileMenu %AUTO% -menu .menubar.file -configvar configuration \
	    -newcommand  [list callback New %W %V]   \
	    -opencommand [list callback Open %W %V] \
	    -savecommand [list callback Save %W %V] \
	    -exitcommand [list callback Exit %W %V]]


. configure  -menu .menubar

entry .config -textvariable configuration
pack .config
