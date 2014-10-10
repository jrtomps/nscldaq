#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2009.
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

package provide  s800 1.0
package require  snit
package require  portAllocator


###
#   s800rctl:
#     This package provides an interface to the S800 run control
#     it is implemented as a Snit type that maintains
#     the connection state and connection information in the 
#     object options and member variables.
#
#



snit::type s800rctl {
  option -host "localhost";	# Host on which the s800 usb rdo runs.
  option -port 8000;		# Port on which the s800 usb rdo listens.
  option -timeout 10000;	# ms to wait for the s800 to reply

  variable socket "";		# Socket connected to the s800 rctl server.
  variable requestReplyReceived 0;
  variable requestReply "";

  variable listenSocket "";		# Socket that can respond to requests from the slave
  variable replySocket "";		# Socket that can respond to requests from the slave
  variable replyReply ""; 

  variable ACK "OK"
  variable NAK "FAIL"

  variable readDoneFlag;	# Incremented to indicate socket readable.
  variable timedOut;   	# Incremented by timeout handler.
  variable timeoutId  -1;	# ID of the timeout [after].


  ##
  #  The constructor forms the connection to the -host, -port configured
  #  into the system.
  # -host is pretty much mandatory as it's not likely the 
  #  the rdo is running on localhost.
  #
  # on Connection failure, a error is generated.
  #
  constructor args {
    $self configurelist $args

    # allocate a port
    set allocator [portAllocator %AUTO%]
    set port [$allocator allocatePort s800rctl]
    set listenSocket [socket -server [mymethod _onConnection] $port]
    $allocator destroy

    $self Connect

  }

  ##
  # Destruction sets the s800 in to master mode and closes the connection
  #
  #
  destructor {
  #
  #  The catch is in case we're being destroyed because we lost
  #  our connection.
  #
    catch {
      if {$socket ne ""} {
        $self setMaster
        close $socket
      } 

      # close the server
      catch {close $listenSocket}

      # close any connection to the server that may have been made
      if {$replySocket ne ""} {
        close $replySocket
        set replySocket ""
      }
    }
  }

  #------------------------------------------------------------------
  #
  #  Operations on the s800:
  #

  ##
  # Set the S800 into slave mode:
  #
  # If a NAK is provided the remainder of the line is thrown as
  # an error:
  #
  method setSlave {} {
    set result [$self Transaction "set slave 1"]
    $self ThrowIfNak $result
  }
  ##
  # Set the s800 back into master mode:
  #
  # If a NAK is provided the remainder of the line is thrown as
  # an error.
  #
  method setMaster {} {
    set result [$self Transaction "set slave 0"]
    $self ThrowIfNak $result
  }
  ##
  #  Request that a run start.
  #
  # NAKs result in an error throw:
  #
  method begin {} {
    set result [$self Transaction begin]
    $self ThrowIfNak $result
  }
  ##
  # Request that a run end.
  #
  # NAKs result in error throws:
  #
  method end {} {
    set result [$self Transaction end]
    $self ThrowIfNak $result
  }
  ##
  # Set the run number to the appropriate value
  #
  # @param n - run number must be an integer >= 0.
  #
  # Throws an error on NAK.
  # Throws an error on an invalid run number.
  #
  method setRun {n} {
    if {![$self ValidRun $n]} {
      error "ERROR - setRun detected an invalid run number $n"
    }
    set result [$self Transaction "set run $n"]
    $self ThrowIfNak $result
  }
  ##
  # Set the run title.
  #
  # @param title - The title of the new run.
  #
  # Throws an error if a NAK.
  # @note The title has any "'s turned into \" so that the string
  #       is a valid Tcl quoted string.
  #
  #
  method setTitle title {
    set title [$self SubstituteSpecialChars $title]
    set result [$self Transaction "set title \"$title\""]
    $self ThrowIfNak $result
  }

  ##
  # Request recording
  #
  # Throws an error on NAK
  #
  method setRecordingOn {} {
    set result [$self Transaction "set recording 1"]
    $self ThrowIfNak $result
  }
  ##
  # Request recording off
  #
  # Throws an error on NAK.
  #
  method setRecordingOff {} {
    set result [$self Transaction "set recording 0"]
    $self ThrowIfNak $result
  }
  ##
  # Set the file save destination.
  #  The s800 will interpret this as a directory relative to this
  # base directory (usually the user's home directory).  That directory
  # must have S800 write access.
  #
  # @param dir - destination directory.
  #
  # Throws on NAK.
  #
  method setDestination dir {
    set result [$self Transaction "set destination $dir"]
    $self ThrowIfNak $result

  }
  ##
  # Return the current s800 state (see below).
  #
  # @return state string which is one of "active" "inactive"
  #
  method getState {} {
    set result [$self Transaction "get state"]
    $self ThrowIfNak $result
    set tail [lindex $result 1]
    set tail [string trim $tail]
    return $tail
  }



  #------------------------------------------------------------------
  #
  #  Private methods
  #
  ##
  #   Connect to the run control
  # Implicit inputs:
  #   $options(-host) contains the target host.
  #   $options(-port) contains the target port.
  # Implicit outputs:
  #   the socket variable will have the connected socket.
  # Errors:
  #   Connection failure throws an error.
  #
  method Connect {} {
    set socket [socket $options(-host) $options(-port)]
    chan configure $socket -buffering line -blocking 0
    chan event $socket readable [mymethod _onRequestReadable]
  }

  ##
  # Send a command to the run control and get an Ack back
  # ACK is a string whose first characters match the ACK variable contents.
  # NAK is a string whose first characters match the NAK variable contents.
  #
  # @param command - The command to send.
  #
  # @return - two item list the first item is either 'OK' or 'ERROR' depending
  #           on whether or not an ACK or NAK was received.  The second
  #           item is the remainder of the response line from the server.
  # Errors:
  #   If the socket is disconnected and could not be reconnected, an error
  #   is thrown.
  method Transaction {command} {

  #  If the socket is disconnected (empty $socket) try to reconnect:

    if {$socket eq ""} {
      $self Connect;		# Throws error on failure.
    }

    # Try to send the message.  If there's a failure
    # 1. set the socket to null
    # 2. try once to reconnect (throws an error if necesary).
    # 3. If a successful reconnect, re-try the message:
    #
    set requestReply ""
    puts "transaction $command [clock microseconds]"
    if {[catch {puts $socket  "$command"} msg]} {
      puts "failed to send command to socket"
      set socket "";	# In case we live through a connect fail.
      $self Connect
      $self setSlave
      puts $socket "$command";	# Allow the retry to throw an error.
    }

    # wait until the reply has been fully received before proceeding
    vwait [myvar requestReplyReceived]
    puts "done waiting for reply"

   set requestReplyReceived 0

   # Analyze the result:
   return [$self AnalyzeResponse $requestReply]

  }
  ## 
  # Read from the socket with a timeout.
  # This is done by 
  # - Establishing a filevent for readability on the socket
  #   which just increments readDoneFlag
  # - Establishing an after timer which increments the read done flag and sets timed out
  # - vwaiting on readDoneFlag.
  # - etc. etc.
  #
  # @param timeout  - ms to wait for a response.
  #
  # @return the data read on success.
  #
  # @throw - if timedout an error string of "Read timed out on s800"
  # @throw - gets on socket failed.
  #
  method ReadWithTimeout timeout {

  # wait for the socket to be readable with a timeout:
    set timedOut 0


    ##
    # Grumble.... none of this works in the presence of I/O that's
    # scheduled via an AFTER because the vwait below allows those to come
    # crashing down on top of us messing everything up.  For now
    # (and probably forever) I'm giving up the timeout in order to keep'
    # the ability to status poll.

    return [gets $socket]


    fileevent $socket readable [list incr ${selfns}::readDoneFlag]
    set timeoutId \
      [after    $timeout "set ${selfns}::timedOut 1; incr ${selfns}::readDoneFlag"]
    vwait ${selfns}::readDoneFlag

    # Reset the fileevent and if we did not time out cancel the timer:

    fileevent $socket readable [list]
    if {!$timedOut} {
      after cancel $timeoutId
    }
    set timeoutId -1;	

    #  Now figure out what happened:

    if {$timedOut} {
      error "Read timed out on s800"
    }
    return [gets $socket]
  }


  ##
  #  Analyze the response message and break it up into 
  #  the chunks described in Transaction above.
  #
  # @param msg - the resonse message.
  #
  # @return list as described in Transaction above.
  #
  method AnalyzeResponse msg {
    set status [lindex $msg 0]
    set tail   [string range  $msg [string length $status] end]
    set tail   [string range $tail  1 end];	# strip off space after status.

    if {$status eq $ACK} {
      set status "OK"
    } elseif {$status eq $NAK} {
      set status "ERROR"
    } else {
      error "Invalid response from server: $msg"
    }
    return [list $status $tail]

  }
  ##
  # Throws an error if NAK
  #
  # @param response - two element list that is the cooked response
  #                   from the s800
  #
  method ThrowIfNak {response} {
    if {[lindex $response 0] eq "ERROR"} {
      error [lindex $response 1]
    }
  }
  ##
  # Ensures a value is a valid run number.
  #
  # @param v - Value to check.
  #
  # @return bool true if valid, false if not.
  #
  method ValidRun v {

  # Run numbers must be valid nonempty integers:

    if {![string is integer -strict $v]} {
      return false
    }
    # ..and those integers cannot be negative:

    if {$v < 0} {
      return false
    }
    return true
  }
  #  Substitute the special string characters for their backslashed equivalents.
  #
  # @param string - input string
  # @return string -substituted string e..g This is a "test" becomes This is a \"test\"
  #
  method SubstituteSpecialChars {string} {
    regsub -all -- {"} $string {\"} string;	# " <-- stupid emacs colorization/tabbing.
    return $string
  }

  method _onReadable {fd} {
    puts "s800rctl .. _onReadable [clock format [clock microseconds]]"
    if {[eof $fd]} {
      puts "s800rctl .. eof detected"
      catch {close $fd}
    } else {
      if {[catch {gets $fd line} len]} {
        puts $fd "FAIL unable to read data from peer"
      } else {
        return [list [chan blocked $fd] $line]
      }
    }
  }

  method _onReplyReadable {} {
    puts "_onReplyReadable"
    set readInfo [$self _onReadable $replySocket]
    
    puts "blocked? [lindex $readInfo 0], data=\"[lindex $readInfo 1]\""

    append replyReply [lindex $readInfo 1]
    if {![lindex $readInfo 0]} {
      puts "all data present"

      if {[ $self _isValidCommand $replyReply]} {
      # if we had a valid command, then send the response immediately.
      # Executing the command the peer sent may require further
      # communication and at the moment the peer is waiting for a response
      # in blocking mode. Don't let it continue to block! 
        $self _sendResponse $replySocket "OK"
        set result [$self _handleCommand $replyReply]
      } else {
        $self _sendResponse $replySocket "FAIL invalid command"
      }

      # reset  the string
      set replyReply ""
    }
  }

  method _onRequestReadable {} {
    puts "_onRequestReadable"
    set readInfo [$self _onReadable $socket]
    puts "blocked? [lindex $readInfo 0], data=\"[lindex $readInfo 1]\""
    
    append requestReply [lindex $readInfo 1]
    if {![lindex $readInfo 0]} {
      set requestReplyReceived 1
    }
  }

  method _isValidCommand {verb} {
    set acceptedVerbs [list end]
    return [expr {$verb in $acceptedVerbs}]
  }

  method _handleCommand {script} {
    set res [catch {uplevel #0 eval $script} msg]
    if {$res} { 
      return "FAIL $msg"
    } else {
      if {$msg ne ""} {
        return "OK $msg"
      } else {
        return "OK"
      }
    }
  }

  ## @brief A unidirectional message
  #
  #
  method _sendResponse {fd response} {
    if {$fd eq ""} {
    # just in case the socket blew away after reading the message
      set msg "ReadoutGUIRemoteControl::_sendResponse Cannot send response "
      append msg "to peer because it does not exist."
      return -code error $msg 
    } else {
      puts $fd $response
    }
  }

  # _onConnection
  #   Called when a server connection comes in:
  #   -  Close the connection if we already have a client..only one customer at
  #      a time is allowed.
  #   -  Set buffering to line.
  #   -  Set a fileevent to fire when the socket is readable.
  #   -  Save the socket in the clientfd attribute.
  # 
  # @param client     - socket fd open on client.
  # @param clientaddr - IP address of client.
  # @param clientport - Remote Port address.
  #
  method _onConnection {client clientaddr clientport} {
    if {$replySocket != ""} {
      close $client
    } else {
      puts "S800rctl::_onConnection request socket made connection $clientaddr:$clientport"
      chan configure $client -buffering line
      set replySocket $client
      chan event $replySocket readable [mymethod _onReplyReadable]
    }
  }
  }
