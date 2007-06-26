package require tcltest
namespace import ::tcltest::*


#
#  This is the setup proc used by most of the tests.
#  It forces out an end run buffer which flushes any
#  partial event buffer, then clears the statistics:
#

proc flushAndClear {} {
    outputstage control  12 0 [list 1234 54321 [clock format [clock seconds]] "This is the title"]
    outputstage clear
}



##################### Clear should give me empty statistics: #########

test clear-1.0 {Clear should give empty stats} \
-setup flushAndClear                           \
-body {
    outputstage clear
    set result [outputstage statistics]
}                                              \
-result {0 0 {} {}}


######### Sending various individual buffer types (non-event) ########

test stateevent-2.0 {Sending a type 11 buffer}         \
    -setup flushAndClear                             \
    -body {
	outputstage control 11 0 [list 666 0 [clock format [clock seconds]] "Title string"]
	set result [outputstage statistics]
    }                                                \
    -result {1 1 {{11 1}} {{0 1}}}


test stateevent-2.1 {Sending a type 12 buffer}         \
    -setup flushAndClear                             \
    -body {
	outputstage control 12 0 [list 666 0 [clock format [clock seconds]] "Title string"]
	set result [outputstage statistics]
    }                                                \
    -result {1 1 {{12 1}} {{0 1}}}


test stateevent-2.2 {Sending a type 13 buffer}  \
    -setup flushAndClear                             \
    -body {
	outputstage control 13 0 [list 666 0 [clock format [clock seconds]] "Title string"]
	set result [outputstage statistics]
    }                                                \
    -result {1 1 {{13 1}} {{0 1}}}

test stateevent-2.3 {Sending a type 14 buffer} \
    -setup flushAndClear                             \
    -body {
	outputstage control 14 0 [list 666 0 [clock format [clock seconds]] "Title string"]
	set result [outputstage statistics]
    }                                                \
    -result {1 1 {{14 1}} {{0 1}}}


test stateevent-2.4 {Sending one of each control event:}   \
    -setup flushAndClear                             \
    -body {
	foreach type {11 12 13 14} {
	    outputstage control $type 0 [list 666 0 [clock format [clock seconds]] "Title string"]
	}
	set result [outputstage statistics]
    }                                                \
    -result {4 4 {{11 1} {12 1} {13 1} {14 1}} {{0 4}}}


##################### Test scaler events.###########################
# Scaler events can come from more than one node.  Assume nodes
# 1,2,3... first send an event from each..individually, then
# send snapshots from each, then send a scaler and snapshot from each in 
# the same test.
#

test scaler-3.0 {Scaler from node 1}                  \
    -setup flushAndClear                                  \
    -body {
	outputstage control 2 1 [list 0 10 1 2 3 4]
	set result [outputstage statistics]
    }                                                     \
    -result {1 1 {{2 1}} {{1 1}}}

test scaler-3.1 {Scaler from node 2}                \
    -setup flushAndClear                                  \
    -body {
	outputstage control 2 2 [list 0 10 1 2 3 4]
	set result [outputstage statistics]
    }                                                     \
    -result {1 1 {{2 1}} {{2 1}}}


test scaler-3.2 {Scaler from node 3}                    \
    -setup flushAndClear                                  \
    -body {
	outputstage control 2 3 [list 0 10 1 2 3 4]
	set result [outputstage statistics]
    }                                                     \
    -result {1 1 {{2 1}} {{3 1}}}


test scaler-3.3 {Snapshot from node 1}  \
    -setup flushAndClear                                  \
    -body {
	outputstage control 3 1 [list 0 10 1 2 3 4]
	set result [outputstage statistics]
    }                                                     \
    -result {1 1 {{3 1}} {{1 1}}}

test scaler-3.4 {Snapshot from node 2}                   \
    -setup flushAndClear                                  \
    -body {
	outputstage control 3 2 [list 0 10 1 2 3 4]
	set result [outputstage statistics]
    }                                                     \
    -result {1 1 {{3 1}} {{2 1}}}

test scaler-3.5 {Snapshot from node 3}                    \
    -setup flushAndClear                                  \
    -body {
	outputstage control 3 3 [list 0 10 1 2 3 4]
	set result [outputstage statistics]
    }                                                     \
    -result {1 1 {{3 1}} {{3 1}}}

test scaler-3.6 {scaler and snapshots from all nodes: } \
    -setup flushAndClear                                  \
    -body {

	foreach node {1 2 3} {
	    outputstage control 2 $node [list 0 10 1 2 3 4]
	    outputstage control 3 $node [list 0 10 1 2 3 4]
	}

	set result [outputstage statistics]
    }                                                     \
    -result {6 6 {{2 3} {3 3}} {{1 2} {2 2} {3 2}}}

########################### Documentation buffers
#  By now we're pretty confident the statisstics are close to right.
#  send one of each documentation buffer type from each of three
#  nodes named 2 4 6.
#
test docevents-4.0 {Documentation buffers}  \
    -setup flushAndClear                    \
    -body {
	foreach node {2 4 6} {
	    foreach type {4 5 6} {
		outputstage control $type $node [list "String 1" "String2"]
	    }
	}
	set result [outputstage statistics]
    }                                       \
    -result {9 9 {{4 3} {5 3} {6 3}} {{2 3} {4 3} {6 3}}}

##################### Several events can fit in a buffer.
# We can use control buffers to flush them out however.
#

test physics-1.0 {Single physics event single buffer}   \
    -setup flushAndClear                                \
    -body {
	outputstage event 100
	outputstage control 13 0  [list 666 0 [clock format [clock seconds]] "Title string"]
	outputstage statistics
    }                                                   \
    -result {2 2 {{1 1} {13 1}} {{0 2}}}


test physics-1.2 {Several physics events in one buffer} \
    -setup flushAndClear                                \
    -body {
	for {set i 0} {$i < 100} {incr i} {
	    outputstage event 100
	}
	outputstage control 13 0  [list 666 0 [clock format [clock seconds]] "Title string"]
	outputstage statistics
    }                                                   \
    -result {101 2 {{1 1} {13 1}} {{0 101}}}

# 16Kwords in the buffer, so 18 1000 word events will give 2 buffers:

test physics-1.3 {Enough physics events will autoflush the buffer} \
    -setup flushAndClear                                \
    -body {
	for {set i 0} {$i < 18} {incr i} {
	    outputstage event 1000
	}
	outputstage control 13 0  [list 666 0 [clock format [clock seconds]] "Title string"]
	outputstage statistics
    }                                                   \
    -result {19 3 {{1 2} {13 1}} {{0 19}}}

cleanupTests

exit