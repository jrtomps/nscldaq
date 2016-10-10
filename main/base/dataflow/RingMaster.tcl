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
#  REGISTER ring
#     Reports the creation of a new ring.
#  UNREGISTER ring
#     Reports the deletion of an existing ring.
#  REMOTE ring
#     Requests ring from the data to be hoisted via a socket.
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
package require dns


#  Locate the directory in which the hoister lives and
#  build a full path for it:

set bindir [file join [file dirname [info script]] .. bin]
set bindir [file normalize $bindir]
set hoisterProgram [file join $bindir ringtostdout]


# Provide the shared memory directory.. This may need to be changed if the code s ported
# to a non linux system:

set shmDirectory [file join / dev shm]

# Global variables/constants:


set localhost   127.0.0.1;		# IP address of localhost connections.
set knownRings  [list];			# Registered rings.


#----------------------------------------------------------------------
#
#  Determine if a ring name is a remote proxy.
#
# parameters:
#    name - Name of the remote ring.
# Returns:
#    true if so
#    false if not.
#
# If the host translates we'll assume this is a proxy.
#
proc isRemoteProxy name {
    set nameList [split $name @]
    set host     [lindex $nameList 1]
    set ring     [lindex $nameList 0]

    # Must have a non-blank extension:

    if {$host eq ""} {
	return "false"
    }

    if {![catch {exec host $host} ip]} {
	return "true"
    }
    return "false"
}

#-------------------------------------------------------------------------------
#
# Enumerate the existing rings.  The rings are assumed to be in device special
# files located in 'shmdir'.  For each regular file in that directory, we will
# ask for the usage from the ring buffer.  If that fails, then the shared memory
# region is assumed not to be a ring buffer.
# If the ring has a . in it, and the stuff prior to it is a valid
# hostname, We assume that this is a defunct proxy ring and
# remove it.
#
# The rings are enuemrated into the global variable ::knownRings above.
#
proc enumerateRings {} {
    set files [glob -nocomplain [file join $::shmDirectory *_12]]
    set ::knownRings [list]

    puts "Initial file list: $files"

    foreach file $files {
	puts "Trying $file"
	if {[file type $file] eq "file"} {
	    puts "Is ordinary"
	    set shmname [file tail $file]
	    set shmname [string range $shmname 0 end-3]
	    puts "ring name: $shmname"
	    if {[catch {ringbuffer usage $shmname} data] == 0} {
		puts "Is a ring"
		# See if this is a defunct proxy ring:
		#
		if {[isRemoteProxy $shmname]} { 
		    puts "Deleting remote proxy $file"
		    catch {file delete -force $file}
		} else {
		    lappend ::knownRings $shmname
		}
	    } else {
		puts $data
	    }
	}
    }
	       
}


#-------------------------------------------------------------------------------
#
# Start hoisting data to a remote client.  The remote client has specified
# REMOTE ringname.  If the ring exists, we will start an instance of 
# ringtostdout that is pointed at the socket as stdout.  Once started,
# we rundown all the resources associated with the socket and close it.
#
# Parameters:
#   socket    - The socket requesting remote access to the ring data.
#   client    - The IP address of the client.
#   tail      - The command.. should look like REMOTE ringname.
#
proc RemoteHoist {socket client tail} {
    ::log::log debug "REMOTE from $client : $tail"
    # Ensure client provided a ring:

    if {[llength $tail] != 2} {
	::log::log info "$tail not valid"
	releaseResources $socket $client
	return
    }
    # Ensure the ring exists.
    
    set ringname [lindex $tail 1]
    if {[lsearch -exact $::knownRings $ringname] == -1} {
	puts $socket "ERROR $ringname does not exist"
    } else {
	# If possible convert the client name in to an fqdn for the
	# host (documented in the hoisters parameters).
	
	set dnsTok [::dns::resolve $client]
	set result [dns::wait $dnsTok]
	if {$result eq "ok"} {
	    set client [::dns::name $dnsTok];  # FQDN of peer.    
	} 
	::dns::cleanup $dnsTok
	
	puts $socket "OK BINARY FOLLOWS"
	exec $::hoisterProgram $ringname $client >@ $socket  &
	releaseResources $socket $client
    }


}
#------------------------------------------------------------------------------
# Kill clients of a specified ring.  This is done when a ring is being unregistered.
#
#  Parameters:
#    ringName - Name of the ring on which to operate.
#
# NOTE: This is unix specific as it assumes the existence of a 'kill' command that
#       can be execed.
#

proc killClients ringName {
    ::log::log debug "Killclients for $ringName"
    set status [catch {ringbuffer usage $ringName} ringUsage]
    
    # If status is 1 the ring probably doesn't actually exist any more so no way to
    # get client info ...just return.

    if {$status} {
	    return
    }

    # start with the producer:
    
    set producerPID [lindex $ringUsage 3]
    if {$producerPID != -1} {
	::log::log debug "Killing producer: $producerPID"
	catch {exec kill -9 $producerPID}
    }
    # Now the consumers:

    set consumerInfo [lindex $ringUsage 6]
    foreach client $consumerInfo {
	set pid [lindex $client 0]
	if {$pid != -1} {;	# Should not need this but...
	    ::log::log debug "Killing consumer: $pid"
	    catch {exec kill -9 $pid}
	}
    }
}
#-------------------------------------------------------------------------------
#
#  Unregisteran existing ring.  If the ring is known, it is removed from the
#  ::knowRings list.  Otherwise an error messagse is returned to the client.
#  this sort of error is not sufficient to disconnect the client.
#
# Parameters:
#   socket - Socket connected to the client.
#   client - IP address of the client.
#   tail   - the message.
#
proc Unregister {socket client tail} {
    ::log::log debug "Unregister from $client : $tail"
    if {[llength $tail] != 2} {
	::log::log info "$tail not valid"
	releaseResources $socket $client
	return
    }

    set ring  [lindex $tail 1]
    set which [lsearch -exact $::knownRings $ring]
    if {$which != -1} {
	::log::log debug "Killing clients of $ring"
	killClients $ring
	::log::log debug "Removing $ring from $::knownRings"
	set ::knownRings [lreplace $::knownRings $which $which]
	puts $socket "OK"
    } else {
	::log::log info "Attempted remove of nonexistent $ring"
	puts $socket "ERROR $ring does not exist"
    }
}

#-------------------------------------------------------------------------------
#
#   Register a new ring. If the ring is unknown it is added to the
#   ::knownRing list.  If it is known that's an error, but not
#  sufficient to kill the connection to the client.
#
#
# Parameters:
#   socket - Socket connected to the client
#   client - Client ip.
#   tail   - The full message we received.
# 

proc Register {socket client tail} {
    ::log::log debug "Register from $client : $tail"
    if {[llength $tail] != 2} {
	::log::log info "$tail not valid"
	releaseResources $socket $client
	return
    }
    set ring [lindex $tail 1]
    if {[lsearch -exact $::knownRings $ring] == -1} {
	::log::log debug "Appending $ring -> $::knownRings"
	lappend ::knownRings $ring
	puts $socket "OK"
    } else {
	::log::log info "Attempted duplicate registration of $ring"
	puts $socket "ERROR $ring is already registered"
    }

}
#-------------------------------------------------------------------------------
#
#  Figure out the known rings.. return them alphabetized.
#
proc listRings {} {
    ::log::log debug listRings

    return [lsort $::knownRings]
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

    set name     [lindex [lindex $message 1] 0];# Strips the {}'s sent in.
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

    if {[llength $message] != 4} {
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
##
# listClients
#   Lists the clients of a specified ring.  The ring must exist.
#
# @param socket  - the socket to which the reply will be written.
# @param client  - Clients IP address.
# @param ring    - Name of the ring buffer we're asking about.
# 
# Returns to the user a two element list.  The first element is 
# the list of words in the producer command.  This is an empty list if
# there is no producer.  The second list is a list of the
# words in the consumer commands.  This is, of course empty if there
# are no consumers.  Only rings in the local host can be listed...
# though they can be listed from anywhere.
#
proc listClients {socket client ring} {
    set status [catch {ringbuffer usage $ring} usage]
    if {$status} {
	puts $socket "ERROR - no such ring: $ring"
    }
    
    set result [list]

    # the producer process:

    set producerPid [lindex $usage 3]
    if {$producerPid != -1} {
	lappend result [pidToCommand $producerPid]
    } else {
	lappend result ""
    }
    
    # The consumer:
    
    set consumers [lindex $usage 6]
    set consumerCommands [list]
    foreach consumer $consumers {
	set pid [lindex $consumer 0]
	lappend consumerCommands [pidToCommand $pid]
    }
    lappend result $consumerCommands
    puts $socket  "OK"
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
    } elseif {$command eq "REGISTER"} {
	Register $socket $client $message
    } elseif {$command eq "UNREGISTER"} {
	Unregister $socket $client $message
    } elseif {$command eq "REMOTE"} {
	RemoteHoist $socket $client $message
    } elseif {$command eq "DEBUG"} {
	# Enable/disable debug logging.
	set state [lindex $message 1]
	::log::lvSuppress debug  $state
    } elseif {$command eq "CLIENTS"} {
	set ring [lindex $message 1]
	listClients $socket $client $ring
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

while {1} {
    if {[catch {set listenPort [$allocator allocatePort "RingMaster12"]}] == 0} {
	break
    }
    after 1000;			# Retry connection in a second.
}
::log::log debug "Obtained a listen port: $listenPort"

# Establish the log destination:

::log::lvChannelForall stderr

# First enable all logging levels:

foreach level {emergency alert critical error warning notice info debug} {
    ::log::lvSuppress $level  0 
}

#  Disable the ones I don't want:

 ::log::lvSuppress debug

enumerateRings


socket -server onConnection $listenPort

::log::log debug "Server listen established on port $listenPort entering event loop"


vwait forever;				# Start the event loop.

