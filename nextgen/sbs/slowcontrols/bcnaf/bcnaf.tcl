#!/bin/sh
# start tclsh: \
exec tclsh ${0} ${@}

set me  [info script]
set dir [file dirname $me]

set cesbcnaf    [file join $dir cesbcnaf]
set wienerbcnaf [file join $dir wienerbcnaf]


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
    catch {exec $cesbcnaf $argv 2>/dev/null} msg
} else {
    if {$selector == "-ces"} {
	catch {exec $cesbcnaf [lrange $argv 1 end] 2>/dev/null} msg
    } elseif {$selector == "-wiener"} {
	catch {exec $wienerbcnaf [lrange $argv 1 end] 2>/dev/null} msg
    } else {
	usage
	exit -1
    }
}
puts $msg
