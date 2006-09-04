#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#

# This package provides a client to access the 
# readout's port for control operations.
#

package provide client   1.0

package require snit

#
#  Snit type to connect with a controls client.
#  Construction implies connection, construction errors will
#  Typically imply that the server is not available.
#   
# Options and defaults:
#    -server  localhost    - The host to connect to.
#    -port    27000        - The connection port.
#  NOTE:  
#       All options can only be provided at construction time.. after that
#       they have no effect.
# Methods:
#    Get    device channel
#    Set    device channel value
#    Update device
#
#
snit::type controlClient {
    variable connection -1

    option    -server   localhost
    option    -port     27000

    constructor args {
	$self configurelist $args

	if  {[catch {socket $options(-server) $options(-port)} connection]} {
	    error $connection
	}
	fconfigure $connection -buffering line
	
    }
    destructor {
	if {[catch {close $connection} msg]} {	# in case already closed catch.
	    error "Error destroying $self : $connection :  $msg"
	}
	
    }
    
    # Get device channel
    #      Attempts to get the value of a channel within a device.
    #      The model is that there may be many devices, each with serveral
    #      channels of data (e.g. a power supply may have a voltage set point,
    #      voltage value, current limit interlock state etc.
    #  Returns:
    #      Success - The result from the server  Note that returns that
    #                begin ERROR mean a server detected error.
    #      Failure - Generally because the connection was dropped by the
    #                server e.g. Readout exited.
    #
    method Get {device channel} {
	puts $connection "Get $device $channel"
	return [gets $connection]

    }
    # Set device channel value
    #      Attempts to set the value of a channel within a device.
    # Returns:
    #     Success - The result from the server.  Note that returns
    #               beginning with "ERROR" are errors detected by the server.
    #     Failure - Generally because the connection was dropped by the server.
    #
    method Set {device channel value} {
	puts $connection "Set $device $channel $value"
	return [gets $connection]
    }
    # Update device
    #     Force the device settings/readings to be udpated.
    #
    method Update device {
	puts $connection "Update $device"
	return [gets $connection]
    }
}
#  The following are convenience functions that may be used
#  by clients.  These are intended to be used within tk.
#  and make nice graphical error messages if the channel got dropped.
#

#  connect
#     Connects to a server in local host.  Returns the connection
#     object or "" if "" the connection could not be established.
#     in that case, a modal information dialog has been popped up
#     for the convenience of the user.
#
proc connect {} {
    if {[catch {controlClient %AUTO%} client]} {
	tk_messageBox  -icon error -title {No Server}    \
	    -message {Unable to connect to control server be sure Readout is runing}
	return ""
    } else {
	return $client
    }
}
#
#  controlOp object ...
#     Performs an arbitrary control operation.
#     If the operation succeeds, the server return value is returned.
#     If the operation fails, a nice dialog is popped up and "" is returned.
#
proc controlOp {object args} {
    if {[catch {eval $object $args} msg]} {
	tk_messageBox -icon error -title {Control op failed} \
	    -message "Unable to perform a [lindex $args 0] : $msg"
	return ""
    } elseif {$msg eq ""} {
	tk_messageBox -icon error -title {Control op failed} \
	    -message "Unable to perform a [lindex $args 0] : Connection dropped."
	return ""
    } else {
	return $msg
    }
}
