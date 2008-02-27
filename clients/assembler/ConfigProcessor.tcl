#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

package provide EVBConfiguration 1.0
package require snit

#  Provides a snit::type that can decode event builder configuration
# manager data and either produce a set of commands for configuring
# the event builder or be inquired for information that can run the 
# GUI:


snit::type evbConfiguration {
    option -config

    constructor args {
	$self configurelist $args
    }

    # Return the number of nodes in the configuration.  This is just
    # the length of the configuration list.

    method nodeCount {} {
	return [llength $options(-config)]
    }
    # Return an item from the configuration.  It is an error
    # to request a configuration item that does not exist.
    #
    # Parameters:
    #    index  - Index of the item to return.
    #
    method getNode index {
	set count [$self nodeCount]
	if {($index < 0) || ($index >= $count) } {
	    error "Nonexistent node index: $index requested"
	}
	return [lindex $options(-config) $index]
    }

    #  Generates an event builder configuration invoking a 
    #  command for each event builder configuration command created.
    # Parameters:
    #  command - The command to invoke for each event builder config
    #            command. The generated command is the parameter of the
    #            command.
    #
    method configBuilder {command} {
	foreach node $options(-config) {
	    set nodeName    [lindex $node 0]
	    set nodeId      [lindex $node 1]
	    set isTrigger   [lindex $node 4]
	    set window      [lindex $node 5]
	    set offset      [lindex $node 6]

	    # Create the node:

	    eval $command [list assembler node $nodeName $nodeId]
	    if {$isTrigger} {
		eval $command [list assembler trigger $nodeId]
	    } else {
		eval $command [list assembler window $nodeId $window $offset]
	    }
	}

    }

}