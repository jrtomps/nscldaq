package require tcltest
namespace import ::tcltest::*

set env(SILENT) quiet;     # Shuts up the spectlcdaq helper program.

################ Creating the input stage ###################



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

	set buffer [concat $header $title $startTime $date]


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
	set header [list [expr 28+32] 2 0 1 1 0 32 0 0x80 0 6 0x0102 0x0304 0x0102 0 0]
	set end    [list 10 0]
	set start  [list 0 0]
	set unused [list 0 0 0]
	for {set i 0} {$i < 32} {incr i} {
	    lappend scalers $i
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
	set event  [list 10 0 1 2 3 4 5 6 7 8 9]
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
	set header [list [expr 28+32] 2 0 1 1 0 32 0 0x81 0 6 0x0102 0x0304 0x0102 0 0]
	set end    [list 10 0]
	set start  [list 0 0]
	set unused [list 0 0 0]
	for {set i 0} {$i < 32} {incr i} {
	    lappend scalers $i
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
	set event  [list 11 0 0 1 2 3 4 5 6 7 8 9]
	set buffer [concat $header $event $event $event]


	inputstage inject $buffer
	inputstage statistics
    }                      \
    -cleanup {
	inputstage stop
	cleanupConfiguration
    }                      \
    -result {{{128 3}} {{1 3}} {{128 {{1 3}}}}}

############## Report results and exit ###################

after 1000
cleanupTests
exit

