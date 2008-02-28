package require tcltest
namespace import ::tcltest::*



source ConfigProcessor.tcl;		# package under test..
package require EVBConfiguration

#------------------------ Factorization helpers -----------------

# Create a configuration with two elements:

proc createConfig2 {} {
    lappend config [list thechad.nscl.msu.edu 1 /some/where/something [list arg list] 1 0 0]
    lappend config [list spdaq22.nscl.msu.edu 2 /some/where/something/else [list arg list 2] 0 1 0]


    return [evbConfiguration %AUTO% -config $config]
}

set storedCommands [list]

proc storeCommands command {
    global storedCommands 
    lappend storedCommands $command
}

#------------------------ The tests -----------------------------

test config-1.0 {Construct a configuration with no content} \
    -body {
	set config [evbConfiguration %AUTO% -config [list]]

	set count [$config nodeCount]

    }                            \
    -cleanup {
	$config destroy
    }                            \
    -result 0

test config-1.1 {Construct a configuration with a single node} \
    -body {
	set config [list thechad.nscl.msu.edu 1 /some/where/something [list arg list] 1 0 0]
	set evb    [evbConfiguration %AUTO% -config [list $config]]

	set count [$evb nodeCount]
    }                                                             \
    -result 1                                                     \
    -cleanup {
	$evb destroy
    }

test config-1.2 {Construct a configuration with two nodes} \
    -setup {
	set config [createConfig2]
    }                                                      \
    -body {
	set count [$config nodeCount]
    }                                                         \
    -result 2                                             \
    -cleanup {
	$config destroy
    }

test config-1.3 {Extract element 0 from a 2 node config} \
    -setup {
	set config [createConfig2]
    }                                                    \
    -body {
	$config getNode 0
    }                                                     \
    -cleanup {
	$config destroy
    }                                                      \
    -result {thechad.nscl.msu.edu 1 /some/where/something {arg list} 1 0 0}

test config-1.4 {Extract element 1 from a 2 node config} \
    -setup {
	set config [createConfig2]
    }                                                    \
    -body {
	$config getNode 1
    }                                                     \
    -cleanup {
	$config destroy
    }                                                      \
    -result {spdaq22.nscl.msu.edu 2 /some/where/something/else {arg list 2} 0 1 0}

test config-1.5 {Errors should be thrown for out of range getNodes} \
    -setup {
	set config [createConfig2] 
    }                                                               \
    -body {
	set errors 0
	if {[catch {$config getNode -1}]} {
	    incr errors
	}
	if {[catch {$config getNode 2}]} {
	    incr errors
	}
	set errors
    }                                                     \
    -cleanup {
	$config destroy
    } \
    -result 2


#--- Test command generation.
lappend expectedCommands [list assembler node thechad.nscl.msu.edu 1]
lappend expectedCommands [list assembler trigger 1]
lappend expectedCommands [list assembler node spdaq22.nscl.msu.edu 2]
lappend expectedCommands [list assembler window 2 1 0]

test config-2.0  {See if commands generted are correct} \
    -setup {
	set config [createConfig2]
	
    } \
    -body {
	set storedCommands [list]
	$config configBuilder storeCommands

	set storedCommands
    } \
    -result $expectedCommands \
    -cleanup {
	$config destroy
    }

cleanupTests






