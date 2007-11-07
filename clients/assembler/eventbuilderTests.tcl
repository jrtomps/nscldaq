package require tcltest
namespace import ::tcltest::*

set env(SILENT) quiet

proc setupConfiguration {} {
    assembler node thechad.nscl.msu.edu 0x80
    assembler trigger 0x80
}
proc cleanupConfiguration {} {
    inputstage destroy
    assembler clear
    catch {exec killall -9 spectcldaq}
}

proc countedString {string length} {
    incr length -1
    while {[string length $string] < $length} {
	append string " "
    }
    return $string
}


test build-1.0 {Build a state transition event} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
	eventbuilder reload
    }                                            \
    -body {
	set header [list 64 11 0 1 1 0 0 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set title [countedString "this is a title" 80]
	set startTime [list 0 0]
	set date [list 10 10 2007 9 17 0]

	set buffer [concat $header [list $title] $startTime $date]


	inputstage inject $buffer
	eventbuilder stats
    }                                             \
    -cleanup {
	inputstage stop
	cleanupConfiguration
	eventbuilder clear
	eventbuilder reload
    }                                               \
    -result {{{128 1}} {{11 1}} {} {}}


test build-1.1 {Do a passthrough event} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
	eventbuilder reload
    }                                            \
    -cleanup {
	inputstage stop
	cleanupConfiguration
	eventbuilder clear
	eventbuilder reload
    }                                               \
    -body {
	set buffer [list 20 4 0 1 1 0 1 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	lappend buffer "set a b"

	inputstage inject $buffer
	eventbuilder stats
    }                           \
    -result {{{128 1}} {{4 1}} {} {}}

test build-1.2 {Do a event buffer with 3 events} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
	eventbuilder reload
    }                                            \
    -cleanup {
	inputstage stop
	cleanupConfiguration
	eventbuilder clear
	eventbuilder reload
    }                                               \
    -body {
	set header [list 46 1 0 1 1 0 3 0 0x80 0 5 0x0102 0x0304 0x0102 0 0]
	set event  [list 11 0 1 2 3 4 5 6 7 8 9]
	set buffer [concat $header $event $event $event]


	inputstage inject $buffer
	eventbuilder stats
    }                                               \
    -result {{{128 3}} {{1 3}} {} {}}

######## Report the tests and exit.

cleanupTests
 
exit
