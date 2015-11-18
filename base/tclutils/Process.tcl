#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#      NSCLDAQ Development Group
#
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

# This software is Copyright by the Board of Trustees of Michigan
# State University (c) Copyright 2015.

# @file  Process.tcl 
# @author Jeromy Tompkins 
#
# ############################################################


package provide Process 1.0

package require snit

## @brief Encapsulation of a pipeline process
#
# This provides the user with a simple way to handle 
# pipelines. The -oneof option is a callback that allows 
# user-defined code to execute when the channel is closed.
#
# It is not possible to open more than one channel per 
# instance of this snit::type.
#
snit::type Process {
  option -command {}
  option -oneof {}
  
  variable fd {}

  ## @brief Open the pipeline and register the readable callback
  constructor {args} {
    $self configurelist $args

    set fd [open "| $options(-command) |& cat" r]
    chan configure $fd -blocking 0
    chan configure $fd -buffering line
    chan event $fd readable [mymethod onReadable $fd]
  }

  ##  Close the open channel if it is still open
  destructor {
    if {$fd ne {}} {
      catch {close $fd}
    }
  }

  ## @brief Callback for readable events on the open pipe
  #
  # If the channel is not at an EOF condition, this simply
  # prints the output to stdout. Otherwise, the script specified
  # by the -oneof (i.e. on end-of-file) option is executed in
  # the global scope.
  method onReadable channel {
    chan gets $channel line 
    if {[eof $channel]} {
      catch {close $channel}
      set fd {}
      uplevel #0 $options(-oneof) 
    } else {
      puts $line
    }
  }

  ## @brief Retrieve the pids of the processes in the pipeline
  method getPIDs {} {
    return [pid $fd]
  }
}
