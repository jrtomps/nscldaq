package require tcltest
namespace import ::tcltest::*

set env(SILENT) quiet

proc setupConfiguration {} {
    assembler node thechad.nscl.msu.edu 0x80
    assembler trigger 0x80
}

# Setup a configuration with 2 nodes.. however these two
# nodes are really the test system and it's local host.
#

proc setupConfiguration2 {} {
    setupConfiguration
    assembler node 127.0.0.1 0x81
    assembler window 0x81 100
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

proc startup {} {
    inputstage create
    inputstage start
    eventbuilder reload
    eventbuilder clear
}

proc stopall {} {
    inputstage stop
    cleanupConfiguration
    eventbuilder clear
    eventbuilder reload
}

testConstraint runall 1

########### Tests for simple assemblies involving a single node ##################
test build-1.0 {Build a state transition event} \
    -constraints runall                       \
    -setup {
	setupConfiguration
	startup
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
	stopall
    }                                               \
    -result {{{128 1}} {{11 1}} {} {}}


test build-1.1 {Do a passthrough event} \
    -constraints runall                       \
    -setup {
	setupConfiguration
	startup
    }                                            \
    -cleanup {
	stopall
    }                                               \
    -body {
	set buffer [list 20 4 0 1 1 0 1 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	lappend buffer "set a b"

	inputstage inject $buffer
	eventbuilder stats
    }                           \
    -result {{{128 1}} {{4 1}} {} {}}

test build-1.2 {Do a event buffer with 3 events} \
    -constraints runall                       \
    -setup {
	setupConfiguration
	startup
    }                                            \
    -cleanup {
	stopall
    }                                               \
    -body {
	set header [list 46 1 0 1 1 0 3 0 0x80 0 5 0x0102 0x0304 0x0102 0 0]
	set event  [list 11 0 1 2 3 4 5 6 7 8 9]
	set buffer [concat $header $event $event $event]


	inputstage inject $buffer
	eventbuilder stats
    }                                               \
    -result {{{128 3}} {{1 3}} {} {}}

################### Tests of assembly involving multiple nodes ######################

test build-2.0 {Build state change buffer for 2 nodes. } \
    -constraints runall                       \
    -setup {
	setupConfiguration2
	startup
    }                                       \
    -cleanup {
	stopall
    }                                         \
    -body {
	set header [list 64 11 0 1 1 0 0 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set title [countedString "this is a title" 80]
	set startTime [list 0 0]
	set date [list 10 10 2007 9 17 0]

	set buffer [concat $header [list $title] $startTime $date]


	inputstage inject $buffer
	set header [lreplace $header 8 8 0x81];   # Turn it into a fragment from 0x81
	inputstage inject [concat $header [list $title] $startTime $date]

	# Should trigger the assembly.

	eventbuilder stats
    }                                           \
    -result {{{128 1} {129 1}} {{11 1}} {} {}}

test build-2.1 {Pass through events don't get assembled: } \
    -constraints runall                       \
    -setup {
	setupConfiguration2
	startup
    }                                                           \
    -cleanup {
	stopall
    }                                                \
    -body {
	set buffer [list 20 4 0 1 1 0 1 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	lappend buffer "set a b"

	inputstage inject $buffer
	inputstage inject [lreplace $buffer 8 8 0x81]
	eventbuilder stats
    }                                              \
    -result {{{128 1} {129 1}} {{4 2}} {} {}}

test build-2.2 {Events with the same timestamp from different nodes trivally assemble (trigger first)} \
    -constraints runall                       \
    -setup {
	setupConfiguration2
	startup
    }                                                           \
    -cleanup {
	stopall
    }                                                \
    -body {
	set header [list 46 1 0 1 1 0 3 0 0x80 0 5 0x0102 0x0304 0x0102 0 0]
	set event  [list 11 0 1 2 3 4 5 6 7 8 9]
	set buffer [concat $header $event $event $event]


	inputstage inject $buffer
	set buffer [lreplace $buffer 8 8 0x81]
	inputstage inject $buffer

	eventbuilder stats
    }                                                \
    -result {{{128 3} {129 3}} {{1 3}} {} {}}

test build-2.3 {Events with the same timestamp from different nodes assemble if trigger is not first} \
    -constraints runall                       \
    -setup {
	setupConfiguration2
	startup
    }                                                           \
    -cleanup {
	stopall
    }                                                \
    -body {
	set header [list 46 1 0 1 1 0 3 0 0x81 0 5 0x0102 0x0304 0x0102 0 0]
	set event  [list 11 0 1 2 3 4 5 6 7 8 9]
	set buffer [concat $header $event $event $event]


	inputstage inject $buffer
	set buffer [lreplace $buffer 8 8 0x80]
	inputstage inject $buffer

	eventbuilder stats
    }                                                \
    -result {{{128 3} {129 3}} {{1 3}} {} {}}

test build-2.4 {Events with inexact matching (trigger node first).} \
    -constraints runall                       \
    -setup {
	setupConfiguration2
	startup
    }                                                           \
    -cleanup {
	stopall
    }                                                \
    -body {
	set header [list 46 1 0 1 1 0 3 0 0x80 0 5 0x0102 0x0304 0x0102 0 0]
	set event  [list 11 0 1 2 3 4 5 6 7 8 9]
	set buffer [concat $header $event $event $event]


	inputstage inject $buffer
	set header [lreplace $buffer 8 8 0x81]
	set event1 [lreplace $event 1 1 10]
	set event2 [lreplace $event 1 2 0xfff0 0]
	set event3 [lreplace $event 1 1 80]
	inputstage inject [concat $header $event1 $event2 $event3]

	eventbuilder stats
    }                                                \
    -result {{{128 3} {129 3}} {{1 3}} {} {}}
test build-2.5 {Events with inexact matching (trigger node not first) } \
    -constraints runall                       \
    -setup {
	setupConfiguration2
	startup
    }                                                           \
    -cleanup {
	stopall
    }                                                \
    -body {
	set header [list 46 1 0 1 1 0 3 0 0x81 0 5 0x0102 0x0304 0x0102 0 0]
	set event  [list 11 0 1 2 3 4 5 6 7 8 9]
	set buffer [concat $header $event $event $event]


	set event1 [lreplace $event 1 1 10]
	set event2 [lreplace $event 1 2 0xfff0 0]
	set event3 [lreplace $event 1 1 80]
	inputstage inject [concat $header $event1 $event2 $event3]

	set header [lreplace $header 8 8 0x80]
	inputstage inject [concat $header $event $event $event]

	eventbuilder stats
    }                                                \
    -result {{{128 3} {129 3}} {{1 3}} {} {}}

test build-2.6 {Events with inexact matching where window spans zero (trigger first)} \
    -constraints runall                       \
    -setup {
	setupConfiguration2
	startup
    }                                                           \
    -cleanup {
	stopall
    }                                                \
    -body {
	set header81 [list 46 1 0 1 1 0 3 0 0x81 0 5 0x0102 0x0304 0x0102 0 0]
	set header80 [lreplace $header81 8 8 0x80]

	set triggerevent  [list 11 50 0  2 3 4 5 6 7 8 9] ; # t = 50.


	set event1 [lreplace $triggerevent 1 2 60 0];        # t = 60.
	set event2 [lreplace $triggerevent 1 2 0xfff6 0xffff];    # t = -10
	set event3 [lreplace $triggerevent 1 2 100 0];               # t = 100.



	inputstage inject [concat $header80 $triggerevent $triggerevent $triggerevent]
	inputstage inject [concat $header81 $event1 $event2 $event3]

	eventbuilder stats
    }                                                \
    -result {{{128 3} {129 3}} {{1 3}} {} {}}


test build-2.7 {Events with inexact matching where window spans zero (trigger not first)} \
    -constraints runall                       \
    -setup {
	setupConfiguration2
	startup
    }                                                           \
    -cleanup {
	stopall
    }                                                \
    -body {
	set header81 [list 46 1 0 1 1 0 3 0 0x81 0 5 0x0102 0x0304 0x0102 0 0]
	set header80 [lreplace $header81 8 8 0x80]

	set triggerevent  [list 11 50 0  2 3 4 5 6 7 8 9] ; # t = 50.


	set event1 [lreplace $triggerevent 1 2 60 0];        # t = 60.
	set event2 [lreplace $triggerevent 1 2 0xfff6 0xffff];    # t = -10
	set event3 [lreplace $triggerevent 1 2 100 0];               # t = 100.



	inputstage inject [concat $header81 $event1 $event2 $event3]
	inputstage inject [concat $header80 $triggerevent $triggerevent $triggerevent]

	eventbuilder stats
    }                                                \
    -result {{{128 3} {129 3}} {{1 3}} {} {}}



test build-2.8 {Event streams that have non-matches [trigger first failed match last]} \
    -constraints runall                       \
    -setup {
	setupConfiguration2
	startup
    }                                                           \
    -cleanup {
	stopall
    }                                                \
    -body {
	set header81 [list 46 1 0 1 1 0 3 0 0x81 0 5 0x0102 0x0304 0x0102 0 0]
	set header80 [lreplace $header81 8 8 0x80]

	set triggerevent  [list 11 50 0  2 3 4 5 6 7 8 9] ; # t = 50.


	set event1 [lreplace $triggerevent 1 2 0xfff6 0xffff ]; 
	set event2 [lreplace $triggerevent 1 2 60 0];
	set event3 [lreplace $triggerevent 1 2 0 1];



	inputstage inject [concat $header80 $triggerevent $triggerevent $triggerevent]
	inputstage inject [concat $header81 $event1 $event2 $event3]

	eventbuilder stats
    }                                                \
    -result {{{128 3} {129 3}} {{1 2}} {} {}}

test build-2.9 {Event streams that have non-matches, [trigger not first failed match last]} \
    -constraints runall                       \
    -setup {
	setupConfiguration2
	startup
    }                                                           \
    -cleanup {
	stopall
    }                                                \
    -body {
	set header81 [list 46 1 0 1 1 0 3 0 0x81 0 5 0x0102 0x0304 0x0102 0 0]
	set header80 [lreplace $header81 8 8 0x80]

	set triggerevent  [list 11 50 0  2 3 4 5 6 7 8 9] ; # t = 50.


	set event1 [lreplace $triggerevent 1 2 0xfff6 0xffff ]; 
	set event2 [lreplace $triggerevent 1 2 60 0];
	set event3 [lreplace $triggerevent 1 2 0 1];



	inputstage inject [concat $header81 $event1 $event2 $event3]
	inputstage inject [concat $header80 $triggerevent $triggerevent $triggerevent]

	eventbuilder stats
    }                                                \
    -result {{{128 3} {129 3}} {{1 2}} {} {}}


#####################  Pushing out data via control buffers #############################


test build-3.0 {Push out partial  event when trigger 1'st failed match last} \
    -constraints runall                       \
    -setup {
	setupConfiguration2
	startup
    }                                                           \
    -cleanup {
	stopall
    }                                                \
    -body {
	set header81 [list 46 1 0 1 1 0 3 0 0x81 0 5 0x0102 0x0304 0x0102 0 0]
	set header80 [lreplace $header81 8 8 0x80]

	set triggerevent  [list 11 50 0  2 3 4 5 6 7 8 9] ; # t = 50.


	set event1 [lreplace $triggerevent 1 2 0xfff6 0xffff ]; 
	set event2 [lreplace $triggerevent 1 2 60 0];
	set event3 [lreplace $triggerevent 1 2 0 1];



	inputstage inject [concat $header80 $triggerevent $triggerevent $triggerevent]
	inputstage inject [concat $header81 $event1 $event2 $event3]


	# Now the control buffer (end run):

	set header [list 64 12 0 1 1 0 0 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set title [countedString "this is a title" 80]
	set startTime [list 0 0]
	set date [list 10 10 2007 9 17 0]

	set buffer [concat $header [list $title] $startTime $date]


	inputstage inject $buffer
	set header [lreplace $header 8 8 0x81];   # Turn it into a fragment from 0x81
	inputstage inject [concat $header [list $title] $startTime $date]


	eventbuilder stats
    }                                                \
    -result {{{128 4} {129 4}} {{1 2} {12 1}} {{129 1}} {{129 1}}}



test build-3.1  {Push out partial event when trigger last, failed matchfirst.} \
    -constraints runall                       \
    -setup {
	setupConfiguration2
	startup
    }                                                           \
    -cleanup {
	stopall
    }                                                \
    -body {
	set header81 [list 46 1 0 1 1 0 3 0 0x81 0 5 0x0102 0x0304 0x0102 0 0]
	set header80 [lreplace $header81 8 8 0x80]

	set triggerevent  [list 11 50 0  2 3 4 5 6 7 8 9] ; # t = 50.


	set event1 [lreplace $triggerevent 1 2 0xfff6 0xffff ]; 
	set event2 [lreplace $triggerevent 1 2 60 0];
	set event3 [lreplace $triggerevent 1 2 0 1];


	inputstage inject [concat $header81 $event1 $event2 $event3]
	inputstage inject [concat $header80 $triggerevent $triggerevent $triggerevent]


	# Now the control buffer (end run):

	set header [list 64 12 0 1 1 0 0 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set title [countedString "this is a title" 80]
	set startTime [list 0 0]
	set date [list 10 10 2007 9 17 0]

	set buffer [concat $header [list $title] $startTime $date]


	inputstage inject $buffer
	set header [lreplace $header 8 8 0x81];   # Turn it into a fragment from 0x81
	inputstage inject [concat $header [list $title] $startTime $date]


	eventbuilder stats
    }                                                \
    -result {{{128 4} {129 4}} {{1 2} {12 1}} {{129 1}} {{129 1}}}


test build-3.2 {Push out partial when unmatching fragment is not the last (trigger first) } \
    -constraints runall                       \
    -setup {
	setupConfiguration2
	startup
    }                                                           \
    -cleanup {
	stopall
    }                                                \
    -body {
	set header81 [list 46 1 0 1 1 0 3 0 0x81 0 5 0x0102 0x0304 0x0102 0 0]
	set header80 [lreplace $header81 8 8 0x80]

	set triggerevent  [list 11 50 0  2 3 4 5 6 7 8 9] ; # t = 50.


	set event1 [lreplace $triggerevent 1 2 0xfff6 0xffff ]; 
	set event2 [lreplace $triggerevent 1 2 60 0];
	set event3 [lreplace $triggerevent 1 2 0 1];


	inputstage inject [concat $header80 $triggerevent $triggerevent $triggerevent]
	inputstage inject [concat $header81 $event3 $event1 $event2]


	# Now the control buffer (end run):

	set header [list 64 12 0 1 1 0 0 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set title [countedString "this is a title" 80]
	set startTime [list 0 0]
	set date [list 10 10 2007 9 17 0]

	set buffer [concat $header [list $title] $startTime $date]


	inputstage inject $buffer
	set header [lreplace $header 8 8 0x81];   # Turn it into a fragment from 0x81
	inputstage inject [concat $header [list $title] $startTime $date]


	eventbuilder stats
    }                                                \
    -result {{{128 4} {129 4}} {{1 2} {12 1}} {{129 1}} {{129 1}}}


test build-3.3 {Push out partial when unmatching fragment is not the last (trigger last) } \
    -constraints runall         \
    -setup {
	setupConfiguration2
	startup
    }                                                           \
    -cleanup {
	stopall
    }                                                \
    -body {
	set header81 [list 46 1 0 1 1 0 3 0 0x81 0 5 0x0102 0x0304 0x0102 0 0]
	set header80 [lreplace $header81 8 8 0x80]

	set triggerevent  [list 11 50 0  2 3 4 5 6 7 8 9] ; # t = 50.


	set event1 [lreplace $triggerevent 1 2 0xfff6 0xffff ]; 
	set event2 [lreplace $triggerevent 1 2 60 0];
	set event3 [lreplace $triggerevent 1 2 0 1];


	inputstage inject [concat $header81 $event3 $event1 $event2]
	inputstage inject [concat $header80 $triggerevent $triggerevent $triggerevent]


	# Now the control buffer (end run):

	set header [list 64 12 0 1 1 0 0 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set title [countedString "this is a title" 80]
	set startTime [list 0 0]
	set date [list 10 10 2007 9 17 0]

	set buffer [concat $header [list $title] $startTime $date]


	inputstage inject $buffer
	set header [lreplace $header 8 8 0x81];   # Turn it into a fragment from 0x81
	inputstage inject [concat $header [list $title] $startTime $date]


	eventbuilder stats
    }                                                \
    -result {{{128 4} {129 4}} {{1 2} {12 1}} {{129 1}} {{129 1}}}


test build-3.4 {Push out partial when unmatching fragment is not last (trigger first).} \
    -constraints runall                       \
    -setup {
	setupConfiguration2
	startup
    }                                                           \
    -cleanup {
	stopall
    }                                                \
    -body {
	set header81 [list 46 1 0 1 1 0 3 0 0x81 0 5 0x0102 0x0304 0x0102 0 0]
	set header80 [lreplace $header81 8 8 0x80]

	set triggerevent  [list 11 50 0  2 3 4 5 6 7 8 9] ; # t = 50.


	set event1 [lreplace $triggerevent 1 2 0xfff6 0xffff ]; 
	set event2 [lreplace $triggerevent 1 2 60 0];
	set event3 [lreplace $triggerevent 1 2 0 1];


	inputstage inject [concat $header80 $triggerevent $triggerevent $triggerevent]
	inputstage inject [concat $header81 $event3 $event1 $event2]


	# Now the control buffer (end run):

	set header [list 64 12 0 1 1 0 0 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set title [countedString "this is a title" 80]
	set startTime [list 0 0]
	set date [list 10 10 2007 9 17 0]

	set buffer [concat $header [list $title] $startTime $date]


	inputstage inject $buffer
	set header [lreplace $header 8 8 0x81];   # Turn it into a fragment from 0x81
	inputstage inject [concat $header [list $title] $startTime $date]


	eventbuilder stats
    }                                                \
    -result {{{128 4} {129 4}} {{1 2} {12 1}} {{129 1}} {{129 1}}}

########################################### Tests for orphaned fragment recovery #####


test build-4.0 {fragments are orphaned} \
    -constraints runall                       \
     -setup {
	setupConfiguration2
	startup
    }                                                           \
    -cleanup {
	stopall
    }                                                \
   -body {
	set header81 [list 46 1 0 1 1 0 3 0 0x81 0 5 0x0102 0x0304 0x0102 0 0]
	set header80 [lreplace $header81 8 8 0x80]

	set triggerevent  [list 11 50 0  2 3 4 5 6 7 8 9] ; # t = 50.


	set event1 [lreplace $triggerevent 1 2 0xfff6 0xffff ]; 
	set event2 [lreplace $triggerevent 1 2 60 0];
	set event3 [lreplace $triggerevent 1 2 0 1];
       

       # We're going to inject a fragment buffer , wait 3 seconds then
       # inject a second fragment buffer with different timestamps.
       # That should declare the first buffer orphaned.
       # We look at the stats to verify that.

       inputstage inject [concat $header81 $event1 $event2 $event3]
       
       set event1 [lreplace $triggerevent 1 2 0 1]
       set event2 [lreplace $triggerevent 1 2 50 1]
       set event3 [lreplace $triggerevent 1 2 100 1]

       after 3000;             # Waiting 3 seconds.
       inputstage inject [concat $header81 $event1 $event2 $event3]

       eventbuilder stats

    }                                   \
    -result {{{129 6}} {} {{129 3}} {{129 3}}}


test build-4.1 {triggers are orphaned} \
     -setup {
	setupConfiguration2
	startup
    }                                                           \
    -cleanup {
	stopall
    }                                                \
     -body {
	set header81 [list 46 1 0 1 1 0 3 0 0x81 0 5 0x0102 0x0304 0x0102 0 0]
	set header80 [lreplace $header81 8 8 0x80]

	set triggerevent  [list 11 50 0  2 3 4 5 6 7 8 9] ; # t = 50.


	set event1 [lreplace $triggerevent 1 2 0xfff6 0xffff ]; 
	set event2 [lreplace $triggerevent 1 2 60 0];
	set event3 [lreplace $triggerevent 1 2 0 1];

	 # We are going to inject a fragment buffer from the trigger node.  Wait 3 seconds
	 # then inject another set of trigger fragments.  This should declare the
	 # fragments in the first buffer to be obsolete and trigger a prune.
	 #

	 inputstage inject [concat $header80 $triggerevent $triggerevent $triggerevent]

	 after 3000
	 inputstage inject [concat $header80 $triggerevent $triggerevent $triggerevent]

	 eventbuilder stats
   }                                               \
    -result {{{128 6}} {} {{128 3}} {{128 3}}}



######## Report the tests and exit.

cleanupTests
 
exit
