#
#  PortManager
#     This snit class is responsible for managing the a set of
#     integers: TCP/IP ports in the port manager application.
#  Options:
#     -range    a two element lowport/highport indicating the
#               set of ports to be managed.
#  Instance variables:
#     FreePorts A list of ports that have not yet been allocated.
#     UsedPorts Array of ports that have been allocated.  Indexed by
#               port number. Each element contains a list that has
#               the application name and user running the application.
#
package provide PortManager 0.1
package require snit


::snit::type PortManager {
    option    -range {30001 31000}
    variable  FreePorts ""
    variable  UsedPorts
    
    
    #  The constructor will take the option range
    #  and turn it into a list of free ports.
    #
    constructor {args} {
        $self configurelist $args
        
        set low [lindex $options(-range) 0]
        set hi  [lindex $options(-range) 1]
        
        for {set i $low} {$i <= $hi} {incr i} {
            lappend FreePorts $i
        }
        if {[llength $FreePorts] <= 0} {
            error "Invalid port range."
        }
    }
    #
    #  Configuration handler for -range:
    #  - The value must be a 2 element list.
    #  - FreePorts must be empty.
    #  - UsedPorts must not have any elements.
    # Parameters:
    #   value:
    #      The new value for the port range
    # Errors:
    #   Too late to configure range - configure done after construction.
    #   Range must be {low hi} list - value must be 2 element list.
    onconfigure -range {value} {
        if {[llength $value] != 2} {
            error "Range must be {low hi} list"
        }
        if {([llength $FreePorts] != 0) ||
            ([llength [array names UsedPorts]] != 0)} {
            error "Too late to configure range"
        }
        set options(-range) $value
    }
    #
    # allocatePort:
    #     Allocates a port from the free list.
    #     The port allocated is put into the UsedPorts
    #     array
    # Parameters:
    #     application:
    #        Name of the application that requests the port.
    #     user:
    #        Name of the user that is running the application that is
    #        requesting the port.
    # Returns:
    #    The number of the port that was allocated.
    # Errors:
    #    no free ports  - The free port list is exhausted.
    method allocatePort {application user} {
        
        #  Ensure we have ports to give:
        
        
        if {[llength $FreePorts] == 0} {
            error "no free ports"
        }
        set allocated [lindex $FreePorts end]
        set FreePorts [lrange $FreePorts 0 end-1]
        
        set UsedPorts($allocated) [list $application $user]
                 
        return $allocated
    }
    # freePort:
    #   Free a port that has been allocated.  Note that it is an error to free
    #   a port that has not been allocated.
    # Parameters:
    #   port:
    #      Number of the port to free.
    # Errors:
    #     The port is not an allocated port - the port is not in the allocated list.
    #
    method freePort {port} {
        
        # Be sure the port is allocated:
        
        if {[array names UsedPorts $port] == ""} {
            error "The port is not an allocated port"
        }
        unset UsedPorts($port)
        lappend FreePorts $port
    }
    #  listUsedPorts:
    #    Returns a list of the ports that are in use.
    #  Returns:
    #    A list with one element for each port that is in use.
    #   Each element is a list containing:
    #     {port application user}
    #
    method listUsedPorts {} {
        set result ""
        foreach port [lsort [array names UsedPorts]] {
            set app   [lindex $UsedPorts($port) 0]
            set user  [lindex $UsedPorts($port) 1]
            lappend result [list $port $app $user]
        }
        return $result
    }

    #
    #  writeUsageFile:
    #    Writes a file containing the port usage.
    #    The file consists of a line per allocation.
    #    Each line contains the port number and the list of application /port.
    #    Items in the file are separated by <tab>
    # Parameters:
    #   fd:
    #     A channel open on the file to write this to.
    #
    method writeUsageFile {fd} {
        set info [$self listUsedPorts]
        foreach port $info {
            puts $fd "[lindex $port 0]\t[lindex $port 1]\t[lindex $port 2]"
        }
    }
    
}
