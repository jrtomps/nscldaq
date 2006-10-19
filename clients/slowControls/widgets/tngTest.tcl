source typeNGo.tcl
set george 0

proc limit value {
    if {($value >= 0) && ($value <= 6)} {
	return 1
    }
    tk_messageBox -icon error -title {Bad value} -message {Values must be in [0,6]}
    return 0
}

proc commit {widget value} {
    global george
    set george $value
    $widget Set ""
}

controlwidget::typeNGo  .test -command [list commit %W %V] -validate [list limit %V] \
    -textvariable george -text Commit
pack .test
