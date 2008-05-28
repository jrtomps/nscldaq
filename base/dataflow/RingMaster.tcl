#!/bin/sh
# start tclsh: \
exec tclsh ${0} ${@}


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



#---------------------------------------------------------------------------
#
#  This script manages membership in all the ring buffers. It is responsible
#  for detecting the exit of clients.
#
#  The protocol is very simple ring clients connect and maintain a connection for
#  the life of their execution.  Clients only intereseted in info need not maintain
#  the connection.
# 
#  Once connected the following commands are understood:
#
#   CONNECT ringname connecttype pid comment
#     Indicates the client has connected to the ring 'ringname' with a connection type
#     connecttype that is either 'producer' or 'consumer'.  The client's pid is
#     provided as well as a descriptive comment.
#
#   DISCONNECT ringname connecttype pid
#     Indicates a 'clean' disconnect from a ring. This is done by a destruction of a 
#     CRingBuffer object that was created for the connection. 
#
#   LIST
#
#     Lists information about the ring usage.
#
#  On success, CONNECT and DISCONNECT reply with
#    "OK\n"
#   to the caller.
#  LIST replies with
#     "OK\n"
#  Followed by ring usage as described later.
#
#
#   CONNECT and DISCONNECT are only allowed on sockets that are connected from localhost.
#   LIST is allowed from any connection.
#
#  Errors in the protocol close the socket.
#
#  If a socket close is detected, the server releases all un-disconnected ring buffer pointers
#  still owned by the client.
#
#-------------------------------------------------------------------------------


# Additional packages required:

set libdir [file join [file dirname [info script]] .. TclLibs]
set libdir [file normalize $libdir]

lappend auto_path $libdir

package require portAllocator
package require log
package require ring


# Global variables/constants:


set localhost   127.0.0.1;		# IP address of localhost connections.

#-------------------------------------------------------------------------------
#
#  Figure out the known rings.. return them alphabetized.
#
proc listRings {} {
    ::log::log debug listRings
    set sockets [array names ::RingUsage]
    foreach socket $sockets {
	set usage $::RingUsage($socket)
	foreach item $usage {
	    set ring [lindex $item 0]
	    set rings($ring) $ring
	}
    }
    # The indices of the rings array are the known rings:

    return [lsort [array names rings]]
}

#--------------------------------------------------------------------------------
# 
# Remove a usage entry from the RingUsage array.  It is a non-error no-op to
# request the removal of a nonexistent item.
#
# Parameters:
#   socket  - The socket from which the entry is removed.
#   name    - Name of the ring buffer.
#   type    - Type of connection.
#   pid     - Process id of the client.
#
proc removeUsageEntry {socket name type pid}  {
    ::log::log debug removeUsageEntry

    if {[array names ::RingUsage $socket] ne ""} {
	set usage $::RingUsage($socket)
	::log::log debug "initial usage: $usage"
	set index 0
	foreach item $usage {
	    set uname [lindex $item 0]
	    set utype [lindex $item 1]
	    set upid  [lindex $item 2]
	    if {($uname eq $name)      &&
		($utype eq $type)      &&
		($upid  eq $pid)} {
		set usage [lreplace $usage $index $index]
		::log::log debug "final usage: $usage"
		set ::RingUsage($socket) $usage
		return
	    }
	    
	    incr index
	}
    }
}

#---------------------------------------------------------------------------------
#
#  The socket must be closed.  If the client on the socket owns any resources,
#  the must be disconnected.
#
#  Parameters:
#    socket - The socket to close
#    host   - The host the socket was connecte to.
#
proc releaseResources {socket host} {
    ::log::log debug releaseResources

    if {[array names ::RingUsage $socket] ne ""} {
	# There are resources to kill off:

	set usage $::RingUsage($socket)
	unset ::RingUsage($socket)
	::log::log debug $usage

	foreach connection $usage {
	    set ring   [lindex $connection 0]
	    set type   [lindex $connection 1]

	    if {$type eq "producer"} {
		::log::log debug "Disconnecting producer from $ring"
		catch {ringbuffer disconnect producer $ring}
	    } else {
		set typeList [split $type .]
		set index [lindex $typeList 1]
		::log::log debug "Disconnecting consumer $index from $ring"
		catch {ringbuffer disconnect consumer $ring $index}
	    }

	}
    }

    close $socket
}
#---------------------------------------------------------------------------------
#
#   Process connect messagse.  The form of this message, is:
#   CONNECT rigname connecttype pid comment
#
#   This information is recorded in the socket's entry in the RingUsage array.
# Parameter:
#   socket   - the socket that sent the CONNECT message.
#   client   - IP address of the client.
#   message  - The full message.
#
proc Connect {socket client message} {
    ::log::log debug Connect

     # The client must be local:

    if {$client ne $::localhost} {
	::log::log info "$message from non local host $client"
	releaseResources $socket $client
	return
    }
    #  The message must have a ringname, a connection type a pid and a comment:

    if {[llength $message] != 5} {
	::log::log info "$message not valid"
	releaseResources $socket $client
    }

    # Pull out the pieces of the message:

    set name     [lindex $message 1]
    set type     [lindex $message 2]
    set pid      [lindex $message 3]
    set comment  [lindex $message 4]

    # Just record this:

    ::log::log debug "Connect added entry for $socket : [list $name $type $pid $comment]"

    lappend ::RingUsage($socket) [list $name $type $pid $comment]

    puts $socket "OK"
}

#---------------------------------------------------------------------------------
# Disconnect
#   Process a disconnect message.  The form of this message is:
# DISCONNECT ringname client message
#
#  All we do is remove the information about this entry from the
#  socket's entry in the RingUsage array.
# 
# Parameters:
#   socket       - The socket that received the message.
proc Disconnect {socket client message} {
    ::log::log debug Disconnect

    # The client must be local:

    if {$client ne $::localhost} {
	::log::log inf  "$message from non local host $client"
	releaseResrouces $socket $client
    }
    # The message must have a rigname an connection type and a pid.

    if {[llength $message] ne 4} {
	::log::log info "$message not valid"
	releaseResources $socket $client
    }

    # pull out the pieces we need:

    set name    [lindex $message 1]
    set type    [lindex $message 2]
    set pid     [lindex $message 3]

    ::log::log debug "Removing entry from $socket : $name $type $pid"

    removeUsageEntry $socket $name $type $pid

    puts $socket "OK"
}
#--------------------------------------------------------------------------------
#
#  Process the LIST command.  This command has the form:
#  LIST
#
#   It returns the string OK\r\n followed by the usage from the
#   ring buffer Tcl command's usage for each known ring buffer.
#
#
# Parameters:
#   socket      - connection to the client.
#   client      - host f the client.
#   message     - Full message text.
proc List {socket client message} {

    # The message text can be only the LIST command:

    if {$message ne "LIST"} {
	releaseResources $socket $client
	return
    }
    set rings [listRings]
    set result [list]
    foreach ring $rings {
	if {![catch {ringbuffer usage $ring} usage]} {
	    lappend result [list $ring $usage]
	}
    }
    puts $socket "OK"
    puts $socket $result
}


#---------------------------------------------------------------------------------
#
# onMessage
#   called when a socket becomes readable:
#   - If the socket is at eof, cleanup the clients mess.
#   - Otherwise process the client's commands.
#
# Parameters:
#   socket    - The socket open on the client.
#   client    - The client's IP address in dotted form.
#
proc onMessage {socket client} {
    if {[eof $socket]} {
	::log::log info "Connection lost from $client"
	releaseResources $socket $client
	return
    }
    set message [gets $socket]
    
    # Often we get an empty messages just before the eof ignore those.

    if {$message eq ""} {
	return
    }

    # Process log and acknowledge the client's message.

    set message [string trimright $message];# chop off carriage control chars.
    set command [lindex $message 0]

    if       {$command eq "CONNECT"} {
	Connect $socket $client $message

    } elseif {$command eq "DISCONNECT"} {
	Disconnect $socket $client $message

    } elseif {$command eq "LIST" } {
	List $socket $client $message
    } else {
	# Bad command means close the socket:

	::log::log info "Invalid command $command from $socket closing"
	releaseResources $socket $client
	
    }
    ::log::log debug "Processed '$message' from client at $client"
}


#---------------------------------------------------------------------------------
#
#  onConnection:
#    called when a new connection to the serve is received.
#    The client socket is configured with line buffering.
#    A fileevent is established so that when the socket becomes readable,
#    the onMessage is invoked with the client socket as a parameter.
# Parameter:
#   channel    - Channel open on the client socket.
#   clientaddr - The IP address of the host connecting to us.
#                note that the ::localhost variable contains the IP
#                address that a localhost connection will give.
#   clientport - The client's port (not really relevant).
#
proc onConnection {channel clientaddr clientport} {
    ::log::log info "New connection from $clientaddr"

    # Set the channel to line buffering and establish the handler:

    fconfigure $channel -buffering line
    fileevent  $channel readable [list onMessage $channel $clientaddr]

}

#---------------------------------------------------------------------------------
#
#  Entry point. We get our listen port from the NSCL Port manage.  This 
#  also registers us for lookup by clients.
#
#


set allocator [portAllocator new]
set listenPort [$allocator allocatePort "RingMaster"]

# Establish the log destination:

::log::lvChannelForall stderr

# First enable all logging levels:

foreach level {emergency alert critical error warning notice info debug} {
    ::log::lvSuppress $level  0 
}

#  Disable the ones I don't want:

::log::lvSuppress debug




socket -server onConnection $listenPort

::log::log debug "Server listen established on port $listenPort entering event loop"


vwait forever;				# Start the event loop.

