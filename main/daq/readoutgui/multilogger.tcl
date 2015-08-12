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
# @file multilogger.tcl
# @brief Provide mechanism to create loggers on multiple rings.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
#  This optional plugin provides a mechanism to log data  from multiple rings
#  in addition to the default event logger.  Data are logged only when event
#  recording is enabled.  The package provide:
#   -  A multi-logger menu bar entry that includes commands that allow you to
#      configure the rings that can be logged as well as select which loggers
#      will be active at any time (see menu commands below).
#   - A state transition callback bundle that manages the enabled loggers
#     at appropriate state transitions.
#
#  What is a logger:
#    A logger is defined as a source ring (URI) and a target directory.
#    At this time the only logger that can be run is $DAQBIN/eventlog with pretty
#    vanilla switches.  In addition, loggers can be enabled or disabled.
#    At the start of a recording run, only the enabled loggers are started.
#    Unlike then normal event recording bundle we are going to run the loggers
#    with stdout/stderr going to a pipe and:
#    # Monitor that pipe for output which is logged.
#    # Monitor that pipe for unexpected exits.
#    # Use the closure of the pipe from the eventlogger's side to determine
#      when an event logger (--oneshot is used) exits.
#
#  Menu Commands:
#    The package adds a menubar entry that contains the following menu commands:
#    *  Add Logger...     - Allows you to define a new logger.
#    *  Remove loggers... - Allows you to destroy an existing logger.
#    *  Enable Loggers... - Allows you to specify which loggers will be active.
#    *  List Loggers      - Lists the loggers and their states.
#
#  This package is installed by default but you must load it (e.g. with  ReadoutCallouts.tcl)
#  to incorporate it into the readout gui.
#
#
package provide multilogger 1.0
package require Tk
package require ReadoutGui
package require RunstateMachine
package require DataSourceUI
package require snit


# Namespace to hold our variables:

namespace eval multilogger {
    variable Initialized 0;                       # Protect against multi load:
}

##
# @class EventLogger
#    This class encapsulates an event logger:
#
# OPTIONS:
#   *  -ring     - URI from which data are taken.
#   *  -out      - Directory in which the data from that ring are logged.
#   *  -enable   - True/false reflecting the enabled state.
#   *  -timeout  - Number of seconds to wait for the logger to exit.
#
# METHODS:
#   *  start - Start the logger (if enabled).
#   *  stop  - Stop the logger waiting for it to finish.
#
# Note that start/stop insulate the caller from the -enable state, that is:
#   *  start is a noop if enable is false.
#   *  stop is a noop if there is no active logger.
#
snit::type EventLogger {
    option -ring
    option -out
    option -enable   0
    option -timeout 20
    
    constructor args {
        $self configurelist $args
    }
    
    method start {} {}
    method stop {} {}
}

##
# @class AddLogger
#   Form for adding a new logger.  This can be wrapped in a dialog wrapper
#   to create a dialog.
#
# OPTIONS:
#    -ring   - Ring to be logged from.
#    -out    - Output directory.
#    -enable - Initial enable.
#    -timeout - Timeout waiting for exits.
#
snit::widgetadaptor AddLogger {
    option -ring
    option -out
    option -enable   0
    option -timeout 20
    

    
    ##
    # Constructor:
    #   1. Create the hull as a ttk::frame
    #   2. Process the options.
    #   3. Create and layout the widgets so they are bound to the
    #      options.
    #
    constructor args {
        installhull using ttk::frame
        
        $self configurelist $args
        
        ttk::label $win.rlabel -text {Ring URI}
        ttk::entry $win.ring   -textvariable ${selfns}::options(-ring)
        
        ttk::label  $win.lout   -text {Output directory}
        ttk::entry  $win.outdir -textvariable ${selfns}::options(-out)
        ttk::button $win.outbrowse -text Browse... -command [mymethod _browseDir]
        
        ttk::label $win.ltimeout -text {End Run timeout}
        ttk::spinbox $win.timeout -from 2 -to 1000 -textvariable ${selfns}::options(-timeout)
        
        
        ttk::checkbutton $win.enable -text {Enable} -onvalue 1 -offvalue 0 \
            -variable ${selfns}::options(-enable)
        
        grid $win.rlabel $win.ring                   -sticky w
        grid $win.lout $win.outdir $win.outbrowse    -sticky w
        grid $win.ltimeout $win.timeout              -sticky w
        grid $win.enable                   -column 1 -sticky w
    }
    
    ##
    # _browseDir
    #    Browse for a directory.  When/if accepted, the directory is loaded
    #    into options(-out)
    #
    method _browseDir {} {
        set outdir [tk_chooseDirectory -mustexist 1 -initialdir $options(-out) \
            -parent $win -title {Choose output directory}                       \
        ]
        if {$outdir ne ""} {
            set options(-out) $outdir
        }
    }
}

##
# @class SelectLoggers
#   Presents a list of loggers that can be selected.
#
# OPTIONS
#   -loggers   - Set of selected loggers (expressed as objects conforming to the
#                EventLogger interface.  This must be set at construction time.
#
# METHODS
#   getSelected - Returns the list of selected loggers.
#
#
# The user interface is just a listing of the loggers with checkboxes next to
# them.  Checked boxes indicate selection.
#
#
snit::widgetadaptor SelectLoggers {
    option -loggers -default [list] -readonly 1 ;# list of event loggers.
    
    variable checkbuttons    [list]            ; # list of checkbutton widgets.
    
    ##
    #  Construction:
    #    1.  Install the hull as a ttk::frame.
    #    2.  Process options.
    #    3.  Build up the widgets and lay them out.
    #
    constructor {args} {
        installhull using ttk::frame
        $self configurelist $args
        
        set buttonIdx 0
        
        foreach logger $options(-loggers) {
            set ring [$logger cget -ring]
            set odir [$logger cget -out]
            set labelText "$ring -> $odir"
            
            set b $win.b$buttonIdx
            puts $b
            lappend checkbuttons [ttk::checkbutton $b -text $labelText]
            pack $b
            incr buttonIdx
        }
    }
    
    ##
    # getSelected
    #   Return a list of the selected loggers:
    #
    method getSelected {} {
        set result [list]
        foreach button $checkbuttons logger $options(-loggers) {
            $button instate {selected} {
                lappend result $logger
            }
        }
        return $result
    }
}

##
# @class LoggerList
#    List loggers and their characteristics.
#
# OPTIONS:
#   -loggers  - List of objects that supply the EventLogger OPTIONs.
#
#
snit::widgetadaptor LoggerList {
    option -loggers -configuremethod _LoggersChanged -default [list]
    
    
    ##
    # Constructor:
    #   1. Set up the frame as the hull
    #   2. Process the configuration options.
    #
    #   @note  - the configuremethod for -loggers will take care of
    #            creating and laying out the widgets.
    #
    constructor args {
        installhull using ttk::frame
        $self configurelist $args
    }
    
    ##
    # _LoggersChanged
    #    Get rid of children of the hull
    #    Repopulate given the new list.
    #
    # @param optname - the option being configured.
    # @param value    - its new value.
    #
    method _LoggersChanged {optname value} {
        set options($optname) $value
        
        # Destroy all children of the hull:
        
        foreach w [winfo children $win] {
            destroy $w
        }
        #  Rebuild the display
        
        ttk::label $win.ringtitle -text "From Ring"
        ttk::label $win.dirtitle  -text "To directory"
        ttk::label $win.enabled   -text "Is enabled"
        ttk::label $win.timeout   -text "End run timeout (Sec)"
        grid $win.ringtitle $win.dirtitle $win.enabled $win.timeout \
            -sticky w  -padx 5 -pady 5
        
        set idx 0
        foreach logger $value {
            set ring   [$logger cget -ring]
            set dest   [$logger cget -out]
            set enflag [$logger cget -enable]
            set tmo    [$logger cget -timeout]
            if {$enflag} {
                set enableText Yes
            } else {
                set enableText No
            }
            ttk::label $win.ring$idx -text $ring
            ttk::label $win.dest$idx -text $dest
            ttk::label $win.ena$idx  -text $enableText
            ttk::label $win.tmo$idx  -text $tmo
            
            grid $win.ring$idx $win.dest$idx $win.ena$idx $win.tmo$idx \
                -sticky w -padx 5
            
            incr idx
        }
        
    }
}






#-----------------------------------------------------------------------------
# One time package initialization.


proc ::multilogger::initPackage {} {
    if {!$::multilogger::Initialized } {
        
        # Setup the menu:
        
        set myMenu [::ReadoutGUIPanel::addUserMenu multilog MultiLogger]
        $myMenu add command -label {Add Logger...} -command ::multilogger::addLogger
        $myMenu add command -label {List Loggers...}  -command ::multilogger::listLoggers
        $myMenu add separator
        $myMenu add command -label {Enable Loggers...} -command ::multilogger::enableLoggers
        $myMenu add command -label {Delete Loggers...} -command ::multilogger::deleteLoggers
        
        
        set ::multilogger::Initialized 1
    }
}

after 500 ::multilogger::initPackage

