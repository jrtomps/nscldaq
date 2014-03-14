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
	fconfigure $socket -buffering line -blocking 1
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

	if {[catch {puts $socket  "$command"} msg]} {
	    set socket "";	# In case we live through a connect fail.
	    $self Connect
	    $self setSlave
	    puts $socket "$command";	# Allow the retry to throw an error.
	}

	# At this point socket must be nonempty...but we might
	# have an EOF condition on the receive socket if we try
	# to read (e.g. empty input response and eof on socket).
	# in case of read failure, empty response + eof state
	# null out socket and throw an error.


        
	set response [$self ReadWithTimeout $options(-timeout)]
	if {($response eq "") || [eof $socket]} {
	    error "No response from the s800 or EOF [eof $socket]"
	}

	# Analyze the result:

	return [$self AnalyzeResponse $response]
	
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


	
}
