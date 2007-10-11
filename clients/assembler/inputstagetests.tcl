package require tcltest
namespace import ::tcltest::*

set env(SILENT) quiet;     # Shuts up the spectlcdaq helper program.



################ Creating the input stage ###################

set monitorEvent ""
set monitorNode  ""

proc monitorCallback {event node} {
    global monitorEvent
    global monitorNode

    set monitorEvent $event
    set monitorNode $node
}



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


test create-1.0 {Create with no config is an error} \
    -body {
	inputstage create
    }                                               \
    -returnCodes error                              \
    -result "No Nodes Specified in the configuration\n(Configuration)"


test create-1.1 {Create that should succeed (single node that is trigger)}   \
    -setup {
	setupConfiguration
    }                                          \
    -body {

	inputstage create
    }                                          \
    -returnCodes ok                            \
    -result {}                                 \
    -cleanup {
	cleanupConfiguration
    }

test create-1.2 {double create shoulid fail AlreadyExists error}   \
    -setup {
	setupConfiguration
    }                                     \
    -body {
	inputstage create
	inputstage create
    }                                     \
    -returnCodes error                    \
    -result "Attempting to create an existing object.\n(Configuration)" \
    -cleanup {
	cleanupConfiguration
    }
	

############### Deleting input stages. 
#               Simple deletes have been implicitly tested already.

test delete-2.0 {delete of nonexistent input stage should be an error} \
    -body {
	inputstage destroy
    }                                 \
    -returnCodes error               \
    -result "Object does not exist\nNeed to create before destroying" 

       

################# Starting the input stage.
#                 This includes the test to ensure you can't destroy'
#                 a running input stage.

test start-3.0 {Start nonexistent input stage is an error.}  \
    -body {
	inputstage start
    }                      \
    -returnCodes error     \
    -result "Object does not exist\n(InputStage)"   \
 


test start-3.1 {Start with normal completion}        \
    -setup {
	setupConfiguration
	inputstage create
    }                      \
    -body {
	inputstage start
    }                      \
    -result {}             \
    -returnCodes ok        \
    -cleanup {
	inputstage stop
	cleanupConfiguration

    }

test start-3.2 {Start a started stage is an error.} \
    -setup {
	setupConfiguration
	inputstage create
    }                               \
    -body {
	inputstage start
	inputstage start
    }                                 \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                                 \
    -returnCodes error                \
    -result "Object is in the running state\n(InputStage)"



test start-3.3 {deleting a started input stage is an error} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }                  \
    -body {
	inputstage destroy
    }                \
    -returnCodes error \
    -result "Object is in the running state\nStop it and try again" \
    -cleanup {
	inputstage stop
	cleanupConfiguration

    }

################### stopping an input stage
#   Normal stop has been implicitly tested in the starts.

test stop-4.0 {Stopping a halted input stage is an error}  \
    -setup {
	setupConfiguration
	inputstage create
    }                     \
    -body {
	inputstage stop
    }                      \
    -returnCodes error     \
    -result "Object is stopped\n(InputStage)"   \
    -cleanup {
	cleanupConfiguration

    }

test stop-4.1 {Stopping a nonexistent input stage is an error} \
    -body {
	inputstage stop
    }            \
    -returnCodes error   \
    -result "Object does not exist\n(InputStage)"  

######################### Simple statistics

test stats-5.0  {Statistics on a new input stage should be 3 empty lists} \
    -setup {
	setupConfiguration
	inputstage create
    }              \
    -body {
	set resultList [inputstage statistics]
	set answer [llength $resultList]
	foreach result $resultList {
	    lappend answer [llength $result]
	}
	set answer
    }                 \
    -result {3 0 0 0} \
    -cleanup {
	cleanupConfiguration
    }


test inject-6.0 {Inject statevar buffer} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set buffer [list 20 4 0 1 1 0 1 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	lappend buffer "set a b"

	inputstage inject $buffer
	inputstage statistics
    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -result {{{128 1}} {{4 1}} {{128 {{4 1}}}}}


test inject-6.1 {Inject a state change buffer} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set header [list 64 11 0 1 1 0 0 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set title [countedString "this is a title" 80]
	set startTime [list 0 0]
	set date [list 10 10 2007 9 17 0]

	set buffer [concat $header [list $title] $startTime $date]


	inputstage inject $buffer
	inputstage statistics
    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -result {{{128 1}} {{11 1}} {{128 {{11 1}}}}}

test inject-6.2 {Inject a scaler buffer} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set header [list [expr 26+64] 2 0 1 1 0 32 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set end    [list 10 0]
	set start  [list 0 0]
	set unused [list 0 0 0]
	for {set i 0} {$i < 32} {incr i} {
	    lappend scalers $i 0 
	}
	set buffer [concat $header $end $unused $start $unused $scalers]

	inputstage inject $buffer
	inputstage statistics
    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -result {{{128 1}} {{2 1}} {{128 {{2 1}}}}}

test inject-6.3 {Inject an event buffer with 3 events non jumbo buffers} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set header [list 46 1 0 1 1 0 3 0 0x80 0 5 0x0102 0x0304 0x0102 0 0]
	set event  [list 11 0 1 2 3 4 5 6 7 8 9]
	set buffer [concat $header $event $event $event]


	inputstage inject $buffer
	inputstage statistics
    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -result {{{128 3}} {{1 3}} {{128 {{1 3}}}}}

test inject-6.4 {Inject a scaler buffer  with a bad node id.} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set header [list [expr 26+64] 2 0 1 1 0 32 0 0x81 0 6 0x0102 0x0304 0x0102 0 0]
	set end    [list 10 0]
	set start  [list 0 0]
	set unused [list 0 0 0]
	for {set i 0} {$i < 32} {incr i} {
	    lappend scalers $i 0
	}
	set buffer [concat $header $end $unused $start $unused $scalers]

	inputstage inject $buffer
	inputstage statistics
    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -returnCodes error      \
    -result "There is no node with this node id\nInjecting data"

test inject-6.5 {Inject an event that is too small to have a timestamp} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set header [list 46 1 0 1 1 0 1 0 0x80 0 5 0x0102 0x0304 0x0102 0 0]
	set event  [list 1]
	set buffer [concat $header $event $event $event]


	inputstage inject $buffer
	inputstage statistics
    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }  \
    -returnCodes error \
    -result "Event is too small to have a timestamp.\nInjecting data"

test inject-6.6 {Inject events for a jumbo buffer} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set header [list 46 1 0 1 1 0 3 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set event  [list 12 0  0 1 2 3 4 5 6 7 8 9]
	set buffer [concat $header $event $event $event]


	inputstage inject $buffer
	inputstage statistics
    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -result {{{128 3}} {{1 3}} {{128 {{1 3}}}}}

################ Monitor scripts #######################

test monitor-7.0 {Monitor startup callback} \
    -setup {
	setupConfiguration
	inputstage create
	set monitorEvent ""
	set monitorNode ""
    }                               \
    -body {
	inputstage monitor monitorCallback
	inputstage start

	set monitorEvent
    }                              \
    -result startup               \
    -cleanup {
	inputstage unmonitor
	inputstage stop
	cleanupConfiguration
    }


test monitor-7.1 {Monitor stop callback} \
    -setup {
	setupConfiguration
	inputstage create
	set monitorEvent ""
	set monitorNode ""
	inputstage start
    }                               \
    -body {
	inputstage monitor monitorCallback
	inputstage stop
	set monitorEvent
    }                                 \
    -result shutdown                 \
    -cleanup {
	inputstage unmonitor
	cleanupConfiguration
    }

test monitor-7.2 {monitor new fragments callback}  \
    -setup {
	setupConfiguration
	inputstage create
	set monitorEvent ""
	set monitorNode ""
	inputstage start
    }        \
    -body {
	inputstage monitor monitorCallback

	# Create/inject the buffer.

	set header [list [expr 26+64] 2 0 1 1 0 32 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set end    [list 10 0]
	set start  [list 0 0]
	set unused [list 0 0 0]
	for {set i 0} {$i < 32} {incr i} {
	    lappend scalers $i 0
	}
	set buffer [concat $header $end $unused $start $unused $scalers]

	inputstage inject $buffer

	list $monitorEvent $monitorNode
    }    \
    -result {new 128}        \
    -cleanup {
	inputstage unmonitor
	inputstage stop
	cleanupConfiguration
    }


test monitor-7.3 {ensure unmonitor works}   \
    -setup {
	setupConfiguration
	inputstage create
	set monitorEvent ""
	set monitorNode ""
    }                               \
    -body {
	inputstage monitor monitorCallback
	inputstage unmonitor
	inputstage start

	set monitorEvent
    }                 \
    -result {}         \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }


################# get first event from a node ###########

test get-8.0 {Get with no data} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }        \
    -body {
	inputstage get 0x80
    }         \
    -returnCodes error \
    -result "Event queue has no events.\n(InputStage::get/pop)" \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }

test get-8.1 {Get with bad node id} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }        \
    -body {
	inputstage get 0
    }         \
    -returnCodes error \
    -result "There is no node with this node id\n(InputStage::get/pop)" \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }


test get-8.2 {Get a scaler event} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set header [list [expr 26+64] 2 0 1 1 0 32 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set end    [list 10 0]
	set start  [list 0 0]
	set unused [list 0 0 0]
	for {set i 0} {$i < 32} {incr i} {
	    lappend scalers $i 0
	}
	set buffer [concat $header $end $unused $start $unused $scalers]

	inputstage inject $buffer



	inputstage get 0x80


    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -result {2 0 128 {10 0 0 0 0 0 0 0 0 0 0 0 1 0 2 0 3 0 4 0 5 0 6 0 7 0 8 0 9 0 10 0 11 0 12 0 13 0 14 0 15 0 16 0 17 0 18 0 19 0 20 0 21 0 22 0 23 0 24 0 25 0 26 0 27 0 28 0 29 0 30 0 31 0}}


test get-8.3 {Get a state transition event}  \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set header [list 64 11 0 1 1 0 0 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set title [countedString "this is a title" 80]
	set startTime [list 0 0]
	set date [list 10 10 2007 9 17 0]

	set buffer [concat $header [list $title] $startTime $date]


	inputstage inject $buffer

	inputstage get 0x80
    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -result {128 0 11 {26740 29545 26912 8307 8289 26996 27764 8293 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 32 0 0 10 10 2007 9 17 0}}


test get-8.4 {Get a string list event} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set buffer [list 20 4 0 1 1 0 1 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	lappend buffer "set a b"

	inputstage inject $buffer
	inputstage get 0x80
    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -result {128 0 4 {25971 8308 8289 98}}

test get-8.5 {Get a physics event (jumbo buffer)} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set header [list 46 1 0 1 1 0 3 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set event  [list 12 0 0 1 2 3 4 5 6 7 8 9]
	set buffer [concat $header $event $event $event]


	inputstage inject $buffer

	inputstage get 0x80

    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -result {128 65536 1 {0 1 2 3 4 5 6 7 8 9}}


test get-8.6 {Get a physics event (non jumbo)} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set header [list 46 1 0 1 1 0 3 0 0x80 0 5 0x0102 0x0304 0x0102 0 0]
	set event  [list 11 0 1 2 3 4 5 6 7 8 9]
	set buffer [concat $header $event $event $event]


	inputstage inject $buffer
	inputstage get 0x80
    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -result {128 65536 1 {0 1 2 3 4 5 6 7 8 9}}

#################### Test pop (destructive read of event fragments).

test pop-9.0 {pop with no events.} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }        \
    -body {
	inputstage pop 0x80
    }         \
    -returnCodes error \
    -result "Event queue has no events.\n(InputStage::get/pop)" \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }

test pop-9.1 {pop to a bad node} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }        \
    -body {
	inputstage pop 0
    }         \
    -returnCodes error \
    -result "There is no node with this node id\n(InputStage::get/pop)" \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }

test pop-9.2 {Pop a scaler event gets right data leads to empty} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set header [list [expr 26+64] 2 0 1 1 0 32 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set end    [list 10 0]
	set start  [list 0 0]
	set unused [list 0 0 0]
	for {set i 0} {$i < 32} {incr i} {
	    lappend scalers $i 0
	}
	set buffer [concat $header $end $unused $start $unused $scalers]

	inputstage inject $buffer



	set result [inputstage pop 0x80]

	lappend result [catch {inputstage pop 0x80}]
	set result

    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -result {2 0 128 {10 0 0 0 0 0 0 0 0 0 0 0 1 0 2 0 3 0 4 0 5 0 6 0 7 0 8 0 9 0 10 0 11 0 12 0 13 0 14 0 15 0 16 0 17 0 18 0 19 0 20 0 21 0 22 0 23 0 24 0 25 0 26 0 27 0 28 0 29 0 30 0 31 0} 1}

test pop-9.3 {Pop a state transition event, gets right data leads to empty} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set header [list 64 11 0 1 1 0 0 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set title [countedString "this is a title" 80]
	set startTime [list 0 0]
	set date [list 10 10 2007 9 17 0]

	set buffer [concat $header [list $title] $startTime $date]


	inputstage inject $buffer

	set result [inputstage pop 0x80]
	lappend result [catch {inputstage pop 0x80}]

	set result
    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -result {128 0 11 {26740 29545 26912 8307 8289 26996 27764 8293 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 32 0 0 10 10 2007 9 17 0} 1}

test pop-9.4 {Pop a string list event, gets right data leads to empty} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set buffer [list 20 4 0 1 1 0 1 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	lappend buffer "set a b"

	inputstage inject $buffer
	set result [inputstage pop 0x80]
	lappend result [catch {inputstage pop 0x80}]
    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -result {128 0 4 {25971 8308 8289 98} 1}

test pop-9.5 {Can pop 3 events gets the right data, leads to empty (jumbo)} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set header [list 46 1 0 1 1 0 3 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set event  [list 12 0 0 1 2 3 4 5 6 7 8 9]
	set buffer [concat $header $event $event $event]


	inputstage inject $buffer

	set result [list [inputstage pop 0x80]]
	lappend result [inputstage pop 0x80]
	lappend result [inputstage pop 0x80]
	lappend result [catch {inputstage pop 0x80}]

    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -result {{128 65536 1 {0 1 2 3 4 5 6 7 8 9}} {128 65536 1 {0 1 2 3 4 5 6 7 8 9}} {128 65536 1 {0 1 2 3 4 5 6 7 8 9}} 1}

test pop-9.6 {Can pop 3 events gets the right data, leads to empty (non jumbo)} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set header [list 46 1 0 1 1 0 3 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set event  [list 12 0 0 1 2 3 4 5 6 7 8 9]
	set buffer [concat $header $event $event $event]


	inputstage inject $buffer

	set result [list [inputstage pop 0x80]]
	lappend result [inputstage pop 0x80]
	lappend result [inputstage pop 0x80]
	lappend result [catch {inputstage pop 0x80}]


    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -result {{128 65536 1 {0 1 2 3 4 5 6 7 8 9}} {128 65536 1 {0 1 2 3 4 5 6 7 8 9}} {128 65536 1 {0 1 2 3 4 5 6 7 8 9}} 1}

test getpop-9.7 {Get should not be destructive} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set header [list 64 11 0 1 1 0 0 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set title [countedString "this is a title" 80]
	set startTime [list 0 0]
	set date [list 10 10 2007 9 17 0]

	set buffer [concat $header [list $title] $startTime $date]


	inputstage inject $buffer

	inputstage get 0x80
	inputstage pop 0x80
    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -result {128 0 11 {26740 29545 26912 8307 8289 26996 27764 8293 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 8224 32 0 0 10 10 2007 9 17 0}}


################### Test empty (clear event queue).

test empty-10.0 {Empty bad node} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }        \
    -body {
	inputstage empty 0
    }         \
    -returnCodes ok \
    -result {}   \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }

test empty-10.1 {Empty empty node is ok} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }        \
    -body {
	inputstage empty 0x80
    }         \
    -returnCodes ok \
    -result {} \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }


test empty-10.2 {Empty a node with a single event frag (scaler) leaves it empty} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set header [list [expr 26+64] 2 0 1 1 0 32 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set end    [list 10 0]
	set start  [list 0 0]
	set unused [list 0 0 0]
	for {set i 0} {$i < 32} {incr i} {
	    lappend scalers $i 0 
	}
	set buffer [concat $header $end $unused $start $unused $scalers]

	inputstage inject $buffer
	inputstage empty 0x80

	inputstage pop 0x80
    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -returnCodes error \
    -result "Event queue has no events.\n(InputStage::get/pop)" 

test empty-10.3 {Empty a node with multiple frags (phy) leaves it empty} \
    -setup {
	setupConfiguration
	inputstage create
	inputstage start
    }          \
    -body {
	set header [list 46 1 0 1 1 0 3 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set event  [list 12 0 0 1 2 3 4 5 6 7 8 9]
	set buffer [concat $header $event $event $event]


	inputstage inject $buffer

	inputstage empty 0x80
	inputstage pop 0x80

    }                      \
    -returnCodes error    \
    -result "Event queue has no events.\n(InputStage::get/pop)" 



############## Report results and exit ###################

after 1000
cleanupTests
exit

