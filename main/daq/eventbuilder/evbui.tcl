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
    option -dt    -default 1     -type [list snit::integer -min 1] \
            -configuremethod _configdt
    option -command -default [list]
    option -state -default normal -configuremethod _configState
    
    constructor args {
        installhull using ttk::frame
        
        ttk::checkbutton $win.build -text Build \
            -variable [myvar options(-build)] -onvalue 1 -offvalue 0  \
            -command [mymethod _onChanged]
        ttk::spinbox $win.dt -text {coincidence interval} \
            -command [mymethod _spinChanged ] \
            -validatecommand [mymethod _validateSpin %P] -validate all          \
            -from 1 -to 100000 -width 5
        $win.dt set 1
	ttk::label $win.dtlabel -text {Coincidence interval}
     
        grid $win.build $win.dt $win.dtlabel -sticky w   
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
    # _configdt
    #   Process changes in the -dt configuration.  Type checking has ensured
    #   that the new value is ok but we need to update the widget contents:
    #
    # @param optname - name of the option modified.
    # @param value   - new value.
    #
    method _configdt {optname value} {
        set $options($optname) $value
        $win.dt set $value
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
        if {$optname ni {normal disabled} } {
            error "$optname's value must be normal or disabled, was $value"
        }
        for win [list build dt] {
            $win.$win configure -state $value
        }
        set options($optname) $value
    }
    #--------------------------------------------------------------------------
    # Action handlers
    
    ##
    # _onChanged
    #   Notifies the user of a change in the user interface values.
    #
    method _onChanged {} {
        set cmd $options(-command)
        if {$cmd ne ""} {
            set cmd [string map [list %B $options(-build) %T $options(-dt)] $cmd]
            uplevel #0 $cmd
        }
    }
    ##
    # _spinChanged
    #    Let the user know the spinbox changed value.
    #    Update the options(-dt) from the spinbox current value
    #    and dispatch to _onChanged.
    #    the validation handler has already ensured the value is legal.
    #
    method _spinChanged {} {
        set options(-dt) [$win.dt get]
        $self _onChanged
    }
    #---------------------------------------------------------------------------
    #  Validation handlers:
    
    ##
    #  _validateSpin
    #
    #    Ensures the spin box has a valid integers (1 or more).
    #
    # @param proposed - the new proposed value.
    # @return boolean - true if proposed is an integer >=1.
    #
    method _validateSpin proposed {
        if {![string is integer -strict $proposed]} {
            bell
            return 0
        }
        if {$proposed < 1} {
            bell
            return 0
        }
        return 1
    
    }
    
}
##
# ::EVBC::glomparams
#     label frame that contains the UI elements for the glom parameters.
#     this is essentially a vertical stack of tssselector and buildparams
#
#     options get delegated and -command gets delegated to a unique handler.
#     We also expose the -title option of the labeled frame and =relief.
#
snit::widgetadaptor ::EVBC::glomparams {
    component tsselector
    component buildparams

    option -state -default normal;      # handle this ourself.

    delegate option -policy to tsselector
    delegate option -tscommand to tsselector as -command

    delegate option -build to buildparams
    delegate option -dt    to buildparams
    delegate option -buildcommand to buildparams as -command

    delegate option -title to hull as -text
    delegate option -relief to hull

    constructor args {
	installhull using ttk::labelframe 

	install tsselector using ::EVBC::tsselector $win.tsselector
	install buildparams using ::EVBC::buildparams $win.buildparams

	puts "$tsselector $buildparams"

	grid $tsselector -sticky w
	grid $buildparams -sticky ew

	$self configurelist $args
    }

}

##
# ::EVBC::intermedRing
#
#  Provide the intermediate ring parameters:
#
# OPTIONS:
#   -tee   - boolean/checkbox enable teeing of output ring.
#   -ring  - Name of the ring to which the tee'd ordered fragments will
#            go.
#   -command - something changed.
#              %W  - widget.
#   -state - widget state
#
#   -title - Title string of ttk::lableframe.
#   -relief - Relief of ttk::labelframe.
snit::widgetadaptor ::EVBC::intermedRing {
    option -tee -default 0 -configuremethod _configTee
    option -ring -default ""
    option -command [list]
    option -state -default normal -configuremethod _configState

    delegate option -title to hull as -text
    delegate option -relief to hull

    constructor args {
	installhull using ttk::labelframe

	ttk::checkbutton $win.tee -variable [myvar options(-tee)] \
	    -onvalue 1 -offvalue 0 -text {Tee output to this ring} \
	    -command [mymethod _onCheckbox]
	ttk::label    $win.rlabel -text {Ring Name}
	ttk::entry     $win.ring -textvariable [myvar options(-ring)] \
	    -state disabled
	
	# Layout:

	grid $win.tee -sticky w
	grid $win.rlabel $win.ring -sticky w

	bind $win.ring <FocusOut> [mymethod _onRingChange]
	bind $win.ring <Return>   [mymethod _onRingChange]
    }
    #-----------------------------------------------------------------
    # Configuration methods:

    ##
    # _configTee
    #
    #    - proposed value must be boolean. The entry
    #    - The ring text entry is only enabled if the value is true.
    # 
    # @param optname - name of the option that's being configured (-tee).
    # @param value   - proposed value.
    #
    method _configTee {optname value} {
	if {$value ni [list 0 1]} {
	    error "$optname value must be in {0,1} was $value"
	}
	set options($optname) $value

	# Handle the state of the entry widget:

	if {$value} {
	    set state normal
	} else {
	    set state disabled
	}
	#  We can only change the state of the 
	#  entry if we ourselves are enabled:

	if {$options(-state)  eq "normal"} {
	    $win.ring configure -state $state

	}
    }
    ##
    # _configState
    #    Process changes to -state:
    #    -   must be normal or disabled.
    #    -   If disabled, all input controls are disabled.
    #    -   If normal, the tee checkbox is enabled.
    #    -   If -tee is true, the ring entry is enbled too.
    #
    # @param optname - name of the option being modified.
    # @param value   - proposed new value.
    #
    method _configState {optname value} {
	if {$value ni [list disabled normal]} {
	    error "$optname value must be one of {disabled, normal} was $value"
	}
	set options($optname) $value

	if {$value eq "disabled"} {
	    foreach w  [list tee ring] {
		$win.$w config -state disabled
	    }
	} else {
	    $win.tee config -state normal
	    if {$options(-tee)} {
		$win.ring config -state normal
	    } else {
		$win.ring config -state disabled;   # we don't ask prior state.
	    }
	}
    }
    #-------------------------------------------------------------------
    #  Action handlers:

    ##
    # _onCheckbox
    #    Called when the tee checkbox was changed.  We need to
    #    -  Configure -state so that the enable/disable is normal
    #    -  invoke the _command method.
    #
    method _onCheckbox {} {
	#
	#  This looks a bit funky but it's right --- trust me.
	#  because changing the checkbutton does not invoke the
	#  configuration handler since Tk has no way to know
	#  the variable's semantics

	$self  _configTee -tee $options(-tee)

	$self _command
    }
    ##
    # _onRingChange
    #   Called when the text entry reflects a committed change to the
    #   ring name entry.   Invoke the _command method.  At present we don't
    #   have a configuration handler for the -ring option so we don't need
    #   to invoke it.
    #
    method _onRingChange {} {
	$self _command
    }
    ##
    # _command
    #   dispatch the -command script along with the substitutions it uses.
    #
    method _command {} {
	set cmd $options(-command)
	if {$cmd ne ""} {
	    set cmd [string map [list %W $win] $cmd]
	    uplevel #0 $cmd
	}
    }
}
##
# ::EVBC::destring
#    Prompt for information about the destination ring.  A text entry
#    and a checkbutton which indicates whether or not data from that
#    ring should be recorded.
#
# OPTIONS
#    -state - normal|disabled - state of widgets.
#    -command - Widget was changed. %W is substituted to be the widget.
#    -ring    - Name of ringbuffer.
#    -record  - 1|0  If the ring should have its data recorded.
#    -title   - The -text of the hull which is a ttk::labelframe
#    -relief  - The -relief of the hull which is a ttk::labelframe.
#
snit::widgetadaptor ::EVBC::destring {
    option -state -default normal -configurecommand _configState

}
