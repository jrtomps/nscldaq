#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# @file LogController.tcl
# @brief Provide the logic that links log messages with the view.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide LogController.tcl

package require snit


##
# @class LogController
#    This is a controller in the MVC sense of the word for log messages.
#     It:
#     - Receives new messages from some external agency and passes them on to
#       the model for storage and the view if filtering criteria are met.
#     - Applies new filtering criteria to the view.
#
# @note - Since the model may contain persistent storage, once the model,
#         controller and the initial filtering options are set,
#         we load the view from the model right away.
#
snit::type LogController {
    option -model -default [list] -readonly yes
    option -view  -default [list] -readonly  yes
    option -filter -default [list] -configuremethod _configureFilter
    
    ##
    # constructor
    #   - configure options.
    #   - load the view from the model.
    #
    constructor args {
        $self configurelist $args
        #
        #  We must have a -model and a -view:
        #
        if {($options(-model) eq "") || ($options(-view) eq "")} {
            error "LogController must be created with -model and -view options"
        }
        $self _loadView
    }
    #--------------------------------------------------------------------------
    # Public methods
    #
    
    ##
    # addMessage
    #   Process a new message that's arrived from wherever.
    #   - Add to the model.
    #   - If the active filter criteria are met, add to the view as well.
    #
    # @param logMessage - decoded log message.  This is the output of
    #                     statusdecode applied to a message.
    #
    method addMessage logMessage {
        $options(-model) add $logMessage
        set messageDict [$self _messageToDict $logMessage];  # turn to dictionary.
        if {[$self _passesFilter $messageDict]} {
            $options(-view) add $messageDict
        }
    }
    #-------------------------------------------------------------------------
    #  Private methods.
    #
    
    ##
    # _configureFilter
    #   Get a new filter.
    #   Note that if both the view and model exist this means that we clear
    #   the view, get new filtered parameters from the model and load them
    #   into the view.
    #
    #   If either has not been configured; we're constructing.
    #
    # @param opt   - Name of the option (-filter).
    # @param value - new value of the filter.
    #
    method _configureFilter {opt value} {
        set options($opt) $value
        
        if {($options(-model) ne "") && ($options(-view) ne "")} {
            $self _loadView;             # update the view from the model.
        }
    }
    ##
    # _loadView
    #    Load the view widget from the model via the filter:
    #
    method _loadView {} {
        $options(-view) clear
        $options(-view) load [$options(-model) get $options(-filter)]
    }
    ##
    # _messageToDict
    #   Turn a message into an internal form dictionary.   This is the form
    #   the view expects, the model returns and is easiest to check against the
    #   current filter.
    #
    # @param rawMessage - the decoded message parts.
    # @return dict      - that contains the log message in canonical form.
    #
    method _messageToDict {rawMessage} {
        set header [lindex $rawMessage 0]
        set body   [lindex $rawMessage 1]
        
        return [dict create \
            severity    [dict get $header severity]         \
            application [dict get $header application]      \
            source      [dict get $header source]           \
            timestamp   [dict get $body timestamp]          \
            message     [dict get $body message]            \
        ]
    }
    ##
    # _passesFilter
    #
    #    Checks to see if a dict form message passes the current filter
    #    criteria
    #
    # @param message - Dict form message
    # @return bool   - True if the message makes the filter criteria.
    # @note uses $options(-filter).
    #
    method _passesFilter {message} {
        foreach clause $options(-filter) {
            set field [lindex $clause 0]
            set op    [lindex $clause 1]
            set value [lindex $clause 2]
            if {$field eq timestamp} {
                set value [clock scan $value]
            }
            set messageChunk [dict get $message $field]
            if {!($messageChunk $op $value)} {
                return false
            }
        }
        return true;             # All criteria matched.
    }
}