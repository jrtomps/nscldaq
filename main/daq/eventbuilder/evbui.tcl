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
# @file evbui.tcl
# @brief Provide an event builder user interface (e.g.glom parameters()).
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide evbui 1.0
package require Tk
package require snit

namespace eval ::EVBC {
    
}


##
#  ::EVBC::tsselector
#     Megawidget to allow users to select glom's timestamp policy.
#
# OPTIONS
#    -policy  - reflects the current policy and sets the current radiobutton
#               if configured.
#    -state   - Widget state, e.g. disabled
#    -command - Command to invoke when a radiobutton is clicked.
#                %P is substituted with the policy selected.
#
snit::widgetadaptor ::EVBC::tsselector {
    option -policy -default earliest -configuremethod _configPolicy
    option -state  -default normal   -configuremethod _configState
    option -command [list]
    
    variable policies [list earliest latest average]
    
    ##
    # constructor
    #   Constructs the megawidget:
    #   -   Hull is a ttk::frame
    #   -   At the left is a label.
    #   -   To the right are a set of radio buttons, one for each policy.
    #
    constructor args {
        installhull using ttk::frame
        
        ttk::label $win.l -text {Ts is: }
        grid $win.l -row 0 -column 0 -sticky w
        set c 1
        foreach policy $policies {
            ttk::radiobutton $win.$policy                                       \
                -text $policy -value $policy -variable [myvar options(-policy)] \
                -command [mymethod _onChanged]                   
            grid $win.$policy -row 0 -column $c -sticky w
            incr c
        }
        #$self $configurelist $args
    }
    #---------------------------------------------------------------------------
    #  Configuration handling methods.
    #
    
    ##
    #  _configPolicy
    #    Configure the -policy option.  Throw an error if the value is not
    #    in policies.
    #
    # @param optname  - option name (-policy e.g.).
    # @param value    - proposed value.
    #
    method _configPolicy {optname value} {
        if {$value ni $policies} {
            error "-policy ($value) must be in {$policies} "
        } else {
            set options($optname) $value;    # this updates the radiobuttons too.
        }
    }
    ##
    # _configState
    #    propagates the requested state to all of the radioubuttons.
    # @param optname - option name being configured.
    # @param value   - new value
    #
    #
    method _configState {optname value} {
        
        #  Ensure that this is a valid state:
        
        if {$value ni [list normal disabled]} {
            error "-state ($value) must be in {normal disabled}"
        }
        foreach policy $policies {
            $win.$policy configure -state $value
        }
        set options($optname) $value
              
    }
    #---------------------------------------------------------------------------
    #   Action handlers:
    #
    
    ##
    # _onChanged
    #   Called when the radio buttons change state.
    #
    method _onChanged {} {
        set cmd $options(-command)
        if {$cmd ne ""} {
            set cmd [string map [list %P $options(-policy)] $cmd]
            uplevel #0 $cmd
        }
    }
    
}
##
# EVBC::buildparams
#    Megawidget that sets the build params.  These include
#    whether or not glom is building and, if so, the coincidence
#    interval.
#
#  OPTIONS
#     -build  - boolean true if building.
#     -dt     - Time interval if building.
#     -state  - Widget state (note that the dt widget is also disabled if
#               building is off).
#     -command - Script to invoke on UI changes:
#                 - %B - build boolena.
#                 - %T - coincidence interval.
#
snit::widgetadaptor ::EVBC::buildparams {
    option -build -default true  -configuremethod _config10
    option -dt    -default 1     -type [list snit::integer -min 1]
    option -command -default [list]
    option -state -default normal -configuremethod _configState
    
    constructor args {
        installhull using ttk::frame
        
        ttk::checkbutton $win.build -text Build \
            -variable [myvar options(-build)] -onvalue 1 -offvalue 0  \
            -command _onChanged
        ttk::spinbox $win.dt -text  -command _spinChanged     \
            -validatecommand _validateSpin -validate all        \
            -from 1 -to 100000
     
        grid $win.build $win.dt -sticky w   
    }
    #---------------------------------------------------------------------------
    # Configuration handlers:
    #
    
    ##
    # _config10
    #    Validate the -build option which must be a zero or a one.
    #    We can't use a bool type validator because the values of a checkbox
    #    are restricted to single values and there are several legal true/false
    #    Tcl bool values.
    #
    #  @param optname - name of the option being configured.
    #  @param value   - proposed value.
    #
    method _config10 {optname value} {
        if {$value ni [list 0 1]} {
            error "$optname must be either 1 or 0 was $value"
        }
        set options($optname) $value
    }
    ##
    # _configState
    #    Validate that the state is either normal or disabled and
    #    set the widgets to the proper state:
    #
    # @param optname - name of the option being configured.
    # @param value   - Proposed value.
    #
    method _configState {optname value} {
        
    }
}
