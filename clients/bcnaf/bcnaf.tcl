#!/bin/sh
# start tclsh: \
exec tclsh ${0} ${@}

set me  [info script]
set dir [file dirname $me]

set cesbcnaf    [file join $dir cesbcnaf.tcl]
set wienerbcnaf [file join $dir wienerbcnaf.tcl]


proc usage {} {
    puts stderr "Usage:"
    puts stderr "    bcnaf ?-type? b c n a f ?d?"
    puts stderr "      type can be either -ces or -wiener to select the controller type"
}



set nparams [llength $argv]
set type ces

# Minimum parameter count:

if {$nparams < 5} {
    usage
    exit -1
}

#  Need to figure out if we have a selector switch:

set selector [lindex $argv 0]
set swchar   [string index $selector 0]

if {$swchar != "-"} {
    exec $cesbcnaf $argv
} else {
    if {$selector == "-ces"} {
	exec $cesbcnaf [lrange $argv 1 end]
    } elseif {$selector == "-wiener"} {
	exec $wienerbcnaf [lrange $argv 1 end]
    } else {
	usage
	exit -1
    }
}