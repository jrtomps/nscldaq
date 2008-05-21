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

package require portAllocator
package require log

# Global variables/constants:


set localhost   127.0.0.1;		# IP address of localhost connections.

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
	close $socket
	return
    }
    set message [gets $socket]
    
    # Often we get an empty messages just before the eof ignore those.

    if {$message eq ""} {
	return
    }

    # Process log and acknowledge the client's message.

    puts $socket "OK"

    ::log::log info "Processed '$message' from client at $client"
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

foreach level {emergency alert critical error warning notice info debug} {
    ::log::lvSuppress $level  0 
}


socket -server onConnection $listenPort

::log::log info "Server listen established on port $listenPort entering event loop"


vwait forever;				# Start the event loop.

