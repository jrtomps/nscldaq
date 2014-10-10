#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
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



##
# @file remoteControl.tcl
# @brief Package to layer a Tcp/IP remote control server on the ReadoutGUI
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide ReadoutGuiRemoteControl 1.0
package require portAllocator
package require RunstateMachine
package require ui
package require Configuration


##
# @class ReadoutGuiRemoteControl
#
#   This snit::type layers a remote control server on top of the 
#   ReadoutGUI.  The server is largely compatible with the 
#   S800 server, however the port is allocated/registerd by the
#   port allocator making it possible to do service location.
#
# @note as with the S800 remote control interface, only one 
#       connection is allowed at a time.
#
snit::type ReadoutGuiRemoteControl {

  variable listenfd
  variable replyfd -1;                  # -1 is disconnected.
  variable requestfd -1;                 # -1 is disconnected.
  variable requestReplyReceived ;
  variable manager  -1;                  # Port manager client instance.

  variable legalVerbs [list set begin end get]

  # For my status area:

  variable statusmanager ""
  variable statusbar     ""

  ##
  # constructor:
  #    - No args are legal, they get processed to report them as illegal (we have no options).
  #    - Portallocator is asked for a port
  #    - Listen on that port is established dispatching to _onConnection
  # @param args - Option value pair list...better by empty.
  #
  constructor args {

  # Get the port.

    set manager [portAllocator %AUTO%]
    set port    [$manager allocatePort ReadoutGUIRemoteControl]

    # set up the listener:

    set listenfd [socket -server [mymethod _onConnection] $port]

  }
  ##
  # destructor
  #   - close the listen socket.
  #   - If we have a client ensure we're back to master mode.
  #   - close any active client.
  #
  destructor {
    close $listenfd
    if {$replyfd != -1}  {
      $self _setMaster;       # return control to the GUI.
      close $replyfd
    }

    if {$requestfd != -1} {
      close $requestFd
    }
  }

  ## @brief Send a message to the peer
  #
  # This is intended to be a symmetric communication such that a response is expected.
  #
  # @param script - the message to send to the peer
  #
  method send {script} {
    if {$requestfd != -1} {

      set requestReply ""

      # send the command
      puts $requestfd $script

      # wait for the response
      vwait [myvar requestReplyReceived]
      
      return $requestReply
    } else {
      return -code error "ReadoutGUIRemoteControl::send Connection does not exist."
    }
  }

  method isConnected {} {
    return [expr {$replyfd != -1}]
  }

  #-----------------------------------------------------------------------------
  # Private methods
  #
  ##
  # _slaveMode
  #    Returns true if we are in slave mode already.
  #
  method _slaveMode {} {
    set rctlPanel [::RunControlSingleton::getInstance]
    return [$rctlPanel isSlave]
  }
  ##
  # _setMaster
  #
  #   Called as we are being destroyed to set the state of the ui back
  #   to master.
  #
  method _setMaster {} {
    set rctlPanel [::RunControlSingleton::getInstance]
    if {[$self _slaveMode]} {
      $rctlPanel master            
    }
  }

  # _onConnection
  #   Called when a server connection comes in:
  #   -  Close the connection if we already have a client..only one customer at
  #      a time is allowed.
  #   -  Set buffering to line.
  #   -  Set a fileevent to fire when the socket is readable.
  #   -  Save the socket in the replyfd attribute.
  # 
  # @param client     - socket fd open on client.
  # @param clientaddr - IP address of client.
  # @param clientport - Remote Port address.
  #
  method _onConnection {client clientaddr clientport} {
    if {$replyfd != -1} {
      close $client
    } else {
      # setup the request socket so that we could send messages to the master
      set allocator [portAllocator %AUTO% -hostname $clientaddr]
      set port [$allocator findServer s800rctl]
      $allocator destroy
      if {$port ne ""} {
        set requestfd [socket $clientaddr $port]
        chan configure $requestfd -blocking 0 -buffering line
        chan event $requestfd readable [mymethod _onRequestReadable]
        
      } else {
        # not finding the service is not an error to fail on but the user needs
        # to be informed
        set msg "ReadoutGUIRemoteControl::_onConnection Unable to locate "
        append msg "s800rctl service on $clientaddr"
        puts stderr $msg
      }

      chan configure $client -buffering line -blocking 0
      set replyfd $client
      chan event $replyfd readable [mymethod _onReplyReadable]
      if {$statusmanager eq ""} {
        set statusmanager [::StatusBar::getInstance]
        set statusbar     [$statusmanager addMessage] 
        $statusmanager setMessage $statusbar "Remote controlled by: $clientaddr"
      }

    }
  }
  ##
  # _reply
  #   Send a reply to the client (or rather attempt to send a reply
  #   to the client.  The actual transfer is done inside a catch command
  #   so that we don't blow up if the wiley client has gone away between sending
  #   us their request and reading our reply.
  #
  # @param status - Status field of the command
  # @param tail   - If present this is a message put on the end of the status.
  # 
  # @note the reply is a list not just textual glomming.
  #
  method _reply {status {tail ""}} {
    set message $status
    if {$tail ne ""} {
      lappend message $tail
    }
    if {[catch {puts $replyfd $message}]} {
      $self _onClientExit
    }
  }

  ##
  # _isLegalCommand
  #   Determines if a message is a legal command.  At this point we just
  #   check legal verbs
  #
  # @param line - line of text from the client
  # @return bool - true if the command verb is legal.
  #
  method _isLegalCommand line {
    set verb [lindex $line 0]
    return [expr {$verb in $legalVerbs}]
  }
  ##
  # _executeCommand
  #    Executes a command that has been determined to have a legal verb.
  #    Each legal verb maps to a method in this class..just precede the
  #    verb name with an _ and you've got the method name...e.g.
  #    set title {some title string}
  #    executes:
  #     $self _set title {some title string}
  #
  # @param line - the line of text gotten from the client.
  #
  method _executeCommand line {
    set verb [lindex $line 0]
    set tail [lrange $line 1 end]

    $self _$verb {*}$tail;    # The executors are responsible for replying.
  }

  ##
  # _onClientExit
  #    Called when an end file is seen on a client.
  #    -  Close the socket
  #    -  set the replyfd attribute to -1 so new clients canconnect
  #
  method _onClientExit {} {
    close $replyfd
    set replyfd -1
    $self _setMaster
    $statusmanager setMessage $statusbar "Remote controlled by: nobody"
  }

  ##
  # _onCommand
  #    Called when the client has a command.
  #    - Retrieve a line of text from the socket.
  #    - ignore "" lines since those happen just before a close
  #    - check the line for legality
  #    - Execute legal commands.
  #
  method _onCommand {line} {
    #set line [gets $replyfd]

    #  empty lines can be read just before a channel goes EOF:

    if {$line eq ""} {
      return
    }
    if {[$self _isLegalCommand $line]} {
      $self _executeCommand $line
    } else {
      $self _reply FAIL "Invalid command '$line'"
    }
  }

  method _onReadable {fd} {
    # allow the specific handlers to handle how to deal with end of file
    #
    if {![eof $fd]} {
      # read what we can at the moment and return it
      if {[catch {gets $fd line} len]} {
        set msg "ReadoutGUIRemoteControl::_onReadable unable to read data from "
        append msg "peer"
        puts stderr $msg
      } else {
        # return a list consisting of whether it blocked and the data read
        return [list [chan blocked $fd] $line]
      }
    }
  }

  ##
  # _onReadable
  #    Called when the client file descriptor is readable.
  #    - If EOF call _onClientExit
  #    - If not EOF call _onCommand
  #
  method _onReplyReadable {} {
    set readInfo [$self _onReadable $replyfd]

    if {[eof $replyfd]} {
      $self _onClientExit
    } else {
      
      append replyReply [lindex $readInfo 1]

      if {![lindex $readInfo 0]} {
        $self _onCommand $replyReply
      }
    }
  }

  method _onRequestReadable {} {
    set readInfo [$self _onReadable $requestfd]

    if {[eof $requestfd]} {
      catch {close $requestfd}
    } else {
      append requestReply [lindex $readInfo 1]

      if {![lindex $readInfo 0]} {
        set requestReplyReceived 1
      }
    }
  }

  #-------------------------------------------------------------------------
  # Methods that execute specific command verbs.
  #

  ##
  # _set
  #    process the set command.  This can set the following things:
  #    *  slave  the slave or master mode (1 - slave, 0 - not slave)
  #    *  run    The desired run number (positive integer)
  #    *  title  The desired run title (string)
  #    *  recording - whether event recording will be done (bool)
  #    *  destination - Where event data are recorded.
  #
  # @param what  - the item to set (see above).
  # @param value - The new value for the item.
  method _set {what value} {

  # None of these are legal if the run is not halted:

  set sm [::RunstateMachineSingleton %AUTO%]
  set state [$sm getState]
  $sm destroy

  if {$what eq "slave"} {
    if {[_notBoolean $value]} {
      $self _reply ERROR "Slave state must be a boolean was: $value"
      return
    } else {

      set rctlPanel [::RunControlSingleton::getInstance]
      set current [$rctlPanel isSlave]
      if {$value} {
        if {$state ne "Halted" } {
          $self _reply ERROR "State must be halted to perform set operations"
          return
        }
        # if not a slave, set it...
        if {!$current} {
          $rctlPanel slave
        }  
        # if we are already a slave, then that is not an error.
        $self _reply OK
      } else {
        if {$current} {
          $rctlPanel master
        }
        $self _reply OK
      }
    }            

  } elseif {$what eq "run" } {
    if {[_notRunNumber $value ]} {
      $self _reply ERROR "Run number must be a positive integer was '$value'"
      return
    } else {
      if {![$self _slaveMode]} {
        $self _reply ERROR "Not in slave mode"
        return
      }
      ::ReadoutGUIPanel::setRun $value
      $self _reply OK
    }

  } elseif {$what eq "title"} {
    if {![$self _slaveMode]} {
      $self _reply ERROR "Not in slave mode"
      return
    }
    ::ReadoutGUIPanel::setTitle $value
    $self _reply OK

  } elseif {$what eq "recording"} {
    if {[_notBoolean $value]} {
      $self _reply ERROR "set recording value must be a boolean was: $value"
      return
    } else {
      if {![$self _slaveMode]} {
        $self _reply ERROR "Not in slave mode"
        return
      }
      if {$value} {
        ::ReadoutGUIPanel::recordOn
      } else {
        ::ReadoutGUIPanel::recordOff
      }
      $self _reply OK
    }

  } elseif {$what eq "destination"} {
  #  Need to be able to write to the destination:

    if {![file writable $value]} {
      $self _reply ERROR "set destination - cannot write to $value"
      return
    } else {
      if {![$self _slaveMode]} {
        $self _reply ERROR "Not in slave mode"
        return
      }
      ::Configuration::Set Stagearea $value
      ::Configuration::Set Experiment [file join $value experiment]
      $self _reply OK
    }
  } else {
    $self _reply ERROR "Invalid set: '$what'"
    return
  }
  }
  ##
  # _begin
  #    *  Run must be - beginnable.
  #    *  Use the state machine to start the run.  Callback bundles do all
  #       the rest.
  #
  method _begin {} {
    if {![$self _slaveMode]} {
      $self _reply ERROR "Not in slave mode"
      return
    }

    set sm [::RunstateMachineSingleton %AUTO%]
    set currentState [$sm getState]
    if {"Active" in [RunstateMachine listTransitions $currentState]} {
      $sm transition "Active"
      $self _reply OK
    } else {
      $self _reply ERROR "The current state ($currentState) does not allow a begin run"
    }
    $sm destroy
  }
  ##
  # _end
  #   * Run must be endable._
  #   * Use the state machine to end the run, and let the callback bundles do
  #     everything else.
  #
  method _end {} {
    flush stdout
    if {![$self _slaveMode]} {
      $self _reply ERROR "Not in slave mode"
      return
    }
    set sm [::RunstateMachineSingleton %AUTO%]
    set currentState [$sm getState]

    if {"Halted" in [RunstateMachine listTransitions $currentState]} {
      $sm transition "Halted"
      $self _reply OK
    } else {
      $self _reply ERROR "The current state ($currentState) does not allow the run to be ended"
    }
    $sm destroy

  }
  ##
  # _get
  #   Returns the value of one of the following:
  #   *   state - active or inactive to be compatible with s800.
  #   *   slave - boolean true if we are in slave mode.
  #
  method _get what {
    if {$what eq "state"} {
      flush stdout
      set sm [::RunstateMachineSingleton %AUTO%]
      set currentState [$sm getState]
      #
      #  TODO:  This is probably not a sufficient listing of state
      #         but it's all the S800readoutcallouts supports at this time.
      #
      if {$currentState in [list Active Paused]} {
        set state active
      } else {
        set state inactive
      }
      $self _reply OK $state
      $sm destroy
    } elseif {$what eq "slave"} {
      set rctlPanel [::RunControlSingleton::getInstance]
      set current [$rctlPanel isSlave]
      $self _reply OK $current

    } else {
      $self _reply ERROR "Invalid get: $what"
    }
  }


  #---------------------------------------------------------------------------
  #  Type scoped methods (not associated with an object)


  ##
  # _notBoolean
  #   @param value - the value to check
  #   @return boolean true if value is not a valid boolean.
  # 
  proc _notBoolean value {
    return [expr {![string is boolean -strict $value]}]
  }
  ##
  # _notRunNumber
  #
  # @param value - the value to check
  # @return boolean - true if value is not a valid run number.
  #
  proc _notRunNumber value {
    if {[string is integer -strict $value]} {
      if {$value >= 0} {
        return false
      } else {
        return true
      }
    } else {
      return true
    }
  }
}
##
# @class OutputMonitor
#    Provides a mechanism for a remote client to monitor what is sent to the
#    output window.
#
#
snit::type OutputMonitor {
  variable clientfds [list]

  variable listenfd
  ##
  # Constructor get a listen port for ReadoutGUIOutput
  # establish that as a server socket.
  # 
  constructor {} {
  set manager [portAllocator %AUTO%]
  set port    [$manager allocatePort ReadoutGUIOutput]        
  set listenfd [socket -server [mymethod _onConnect] $port]
  }
  ##
  # destructor
  #   Close all the sockets (including the listener).
  #   If we had listeners, remove the -monitorcmd callback.
  #
  destructor  {
    if {[llength $clientfds] > 0} {
      foreach client $clientfds {
        close $client
      }
      set ow [::Output::getInstance]
      $ow configure -monitorcmd [list]
    }
    close $listenfd
  }
  ##
  # _onConnect
  #
  #   A client connected.
  #   - If there are no current listeners, establish the -monitorcmd
  #   - Add the client to our client list.
  #   - Set buffering to line.
  #   - Establish a readable filevt to handle remote closes.
  #
  # @param fd     - Data transfer fd
  # @param client - IP of client
  # @param port   - Remote port of the client.
  #
  method _onConnect {fd client port} {
    if {[llength $clientfds] == 0} {
      set ow [::Output::getInstance]
      $ow configure -monitorcmd [mymethod _onGuiOutput]
    }
    lappend clientfds $fd
    fconfigure $fd -buffering line
    fileevent $fd readable [mymethod _onClientInput $fd]
  }
  ##
  # _onGuiOutput
  #    The GUI has output something to its output window.  We need to
  #    relay it to all our clients.
  #
  # @param output -the output
  #
  method _onGuiOutput output {
    foreach fd $clientfds {
      puts $fd $output
    }
  }
  ##
  # _onClientInput
  #    The client fd became readable..clients are not allowed to send us stuff
  #    therefore we assume this comes about because a client has closed the
  #    connection.  If that's not the case, the client is insane so we close
  #    anyway.
  # @param fd - The file descriptor that is readable.
  #
  method _onClientInput fd {
    close $fd
    set listIndex [lsearch -exact $clientfds $fd]
    set clientfds [lreplace $clientfds $listIndex $listIndex]
    if {[llength $clientfds] == 0} {
    # Last one, remove the -monitorcmd

      set ow [::Output::getInstance]
      $ow configure -monitorcmd [list]

    }
  }
}
