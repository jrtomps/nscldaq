#!/bin/sh
# Start wish \
exec wish ${0} ${@}

set me $argv0
set mydir [file dirname $me]

source $mydir/vhqpanel.ui.tcl

set crate 0;           # Default vme crate number.


#
#  Set default channel values (e.g. if raw config file).
#
proc SetDefaultValues {} {
    global SetPoint
    global RampSpeed
    global ILimit
    global maxi

    if {![info exists SetPoint(a)]} {
	set SetPoint(a) 0
    }
    if {![info exists  SetPoint(b)]} {
	set SetPoint(b) 0
    }
    if {![info exists RampSpeed(a)]     } {
	set RampSpeed(a) 100
    }
    if {![info exists RampSpeed(b)]} {
	set RampSpeed(b) 100
    }
    if {![info exists ILimit(a)]} {
        set ILimit(a)   0
    }
    if {![info exists ILimit(b)]} {
        set ILimit(b)   0
    }
}
#
#   Unset a variable with no error if it doesn't exist.
#
proc Unset {name} {
    global $name
    catch "unset $name"
}
#
#  Read a configuration and create the interface for it:
#
proc ReadConfig {file} {
    global SetPoint
    global RampSpeed
    global ILimit
    global base maxv maxi resi description
    global name
    global crate
    
    set   crate 0;    # Default crate is 0.

    Unset SetPoint 
    Unset RampSpeed 
    Unset ILimit 
    Unset base 
    Unset maxv 
    Unset maxi 
    Unset resi 
    Unset description


    source $file
    global $name

    SetDefaultValues

    toplevel .$name
    vhqpanel_ui .$name
    bind .$name <Destroy> exit


	set $name\(crate) $crate
    set $name\(base) $base
    set $name\(maxv) $maxv
    set $name\(maxi) $maxi
    set $name\(resi) $resi
    set $name\(name) $description
    DefineVhq $name

    SetPoint $name a $SetPoint(a)
    SetPoint $name b $SetPoint(b)
    RampSpeed $name a $RampSpeed(a)
    RampSpeed $name b $RampSpeed(b)
    Ilimit    $name a $ILimit(a)
    Ilimit    $name b $ILimit(b)
    

}



wm withdraw .

foreach file $argv {
    ReadConfig $file
}



