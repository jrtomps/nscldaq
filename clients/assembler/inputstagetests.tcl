package require tcltest
namespace import ::tcltest::*

################ Creating the input stage ###################


proc setupConfiguration {} {
    assembler node thechad.nscl.msu.edu 0x80
    assembler trigger 0x80
}

proc cleanupConfiguration {} {
    inputstage destroy
    assembler clear
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
	exec killall spectcldaq
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
	exec killall spectcldaq
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
	exec killall spectcldaq
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




############## Report results and exit ###################

after 1000
cleanupTests
exit

