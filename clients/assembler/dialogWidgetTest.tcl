source dialogwidget.tcl
package require dialog


dialog .d
set l [label [.d workArea].t -text {Click a button}]
.d configure -workarea $l  -buttons {Ok Cancel Help Helpless}


while 1 {
    
    set result [.d execute]
    if {$result eq ""} {
	puts  "Destroyed"
	flush stdout
	exit
    } else {
	puts $result
	flush stdout
    }
}

exit
