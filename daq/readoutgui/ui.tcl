#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file ui.tcl
# @brief Readout GUI User interface elements.
# @author Ron Fox <fox@nscl.msu.edu>
package require Tk
package require snit
package require RunstateMachine

package provide ui   1.0
package provide ReadoutGUIPanel 1.0

namespace eval ::ReadoutGUIPanel {}

#------------------------------------------------------------------------------
# Menubar:
#


##
# @class readoutMenubar
#
#  This class provides a framework for managing the ReadoutGUI's menubar.
#  The class is used by both the standard UI and the the API to make the
#  ReadoutGUI menu and maintain it.
#
# METHODS:
#   addMenu     - Add a new menu. This is a cascade command on the
#                 hull.
#   lookupMenu  - Look up a menu widget path by the text name.
#   listMenus   - returns the names of all menus.
#   addSeparator- Add a separator to to a menu
#   addCommand  - Add a command item to a menu
#   addMenuItem - Add a generic menu item to a menu (when the above
#                 convenience methods don't work.
#
# OPTIONS
#   All options supported by the menu tk command are supported.
snit::widgetadaptor readoutMenubar {
    
    delegate option * to hull
    delegate method * to hull
    
    # This variable contains a lookup table for the submenus
    # The index is the text in the menu (e.g. File) the value is the
    # menu widget path.
    #
    variable menus -array [list]
    variable menucount 0
    
    ##
    # constructor
    #
    #  Installs the hull as a menu.
    #
    # @param args - configuration parameters..
    #
    constructor args {
        installhull using menu -tearoff 0
        
        $self configurelist $args
    }
    
    #--------------------------------------------------------------------------
    #
    # Public methods.
    #
    
    ##
    #   addMenu
    #     Add a new menu. This is a cascade command on the hull.
    #
    # @param text - The menu item label.
    #
    # @note the menu widget is assigned and the menu is added to the menus
    #       array.
    #
    method addMenu text {
        set widget [menu $win.menu[incr menucount] -tearoff 0]
        $hull add cascade -label $text -menu $widget
        
        set menus($text) $widget
    }
    ##
    #   lookupMenu
    #      Look up a menu widget path by the text name.  This is provided in
    #      case there are special needs that are not met by the methods provided.
    #
    # @param text         - The label on the drop down menu.
    # @return widget-path - The path to the menu that is associated with that
    #                       cascade. If no menu with this label exists,
    #                       an empty string is returned.
    #
    method lookupMenu text {
        if {[array names menus $text] ne ""} {
            return $menus($text)
        } else {
            return ""
        }
    }
    ##
    # listMenus
    #    Lists the names of all  menus, in no particular order.
    #
    # @return list
    #
    method listMenus {} {
        return [array names menus]
    }
    ##
    #   addSeparator
    #      Add a separator to to a menu
    #
    # @param name - Name of the menu.
    # @throw If the name does not correspond to an existing menu.
    #
    method addSeparator name {
        $self addMenuItem $name separator
    }
    ##
    #   addCommand
    #     Add a command item to a menu
    #
    # @param name    - Menu name.
    # @param label   - Label for the command item.
    # @param command - script to run when menu itme is clicked.
    #
    method addCommand {name label command} {
        $self addMenuItem $name command -label $label -command $command
    }
    ##    
    #   addMenuItem - Add a generic menu item to a menu (when the above
    #                 convenience methods don't work.
    # @param name - The name of the menu in which the add is done.
    # @param itemtype - The type of entitity that is being added.
    # @param args - The -option value pairs that further describe the item.
    #
    method addMenuItem {name itemtype args} {
        set menu [$self lookupMenu $name]
        if {$menu eq ""} {
            error "There is no menu labeled $name"
        }
        $menu add $itemtype {*}$args
    }
}

#------------------------------------------------------------------------------
#  API elements related to menus.
# ASSUMPTIONS:
#   *    The menu object has been instantiated and associated with the toplevel.
#   *    The main window is the toplevel.
#

##
# ::ReadoutGUIPanel::addUserMenu
#
#  Adds a new user menu.
#
# @param ident - Ignored but present for compatibility sake.
# @param label - The Label on the menu.
#
# @return Window path to the menu item.
#
proc ::ReadoutGUIPanel::addUserMenu {ident label} {
    set menu [. cget -menu]
    $menu addMenu $label
    return [$menu lookupMenu $label]
}


#-------------------------------------------------------------------------------
#
#  Run identification widget and API  - Run number and title

##
# @class RunIdentification
#
#   This widget supplies an entry for the run title and a second for providing
#   the run number.  The run number is validated to ensure it is an integer >= 0.
#   validation is done when we are reasonably certain the run number has been
#   completely entered (on focus change not on every keystroke).
#   If an invalid run number is entered, a dialog pops and the old run number
#   is restored.
#
#   Note that since not all data sources can set a title or run number,
#   -haverun and -havetitle allow the client to run off those widgets.
#
# OPTIONS:
#    -haverun   - True if the run number widget should be displayed.
#                 false otherwise.
#    -havetitle - True if the title should be displayed else false.
#
#    -title     - Current title (read/write).
#    -run       - Current run number (read/write) subject to validation.
#    -state     - State to set the subwidgets.   This is used to disable
#                 modification of the widgets when a run is going on
#                 (Active or Paused).
#
snit::widgetadaptor RunIdentification {
    option -haverun   -default 1   -configuremethod _rebuild
    option -havetitle -default 1 -configuremethod _rebuild
    option -state     -default normal -configuremethod _setState
    
    option -title -default ""   -configuremethod _setTitle -cgetmethod _getTitle
    option -run   -default "0"  -configuremethod _setRun   -cgetmethod _getRun
    
    
    variable lastRunNumber 0
    
    ##
    # constructor
    #
    #   Install the hull as a ttk::frame and process the configuration.
    #   once processed, invoke _rebuild to ensure the widget gets displayed
    #   correctly (because I'm honestly not sure the configure methods will get
    #   called for default values of the options).
    #
    #  @parameter args - the -option values  that modify the initial default
    #                    configuration of the widget.
    #
    constructor args {
        installhull using ttk::frame
        $self configurelist $args
        $self _rebuild -haverun $options(-haverun)
    }
    #--------------------------------------------------------------------------
    # Configuration processing (private).
    
    ##
    # _rebuild
    #
    #   Called when either -haverun or -havetitle are modified.
    #   In this case the widget contents must be rebuilt depending on the new
    #   values of those options.
    #
    # @param optname - Name of the new option.
    # @param value   - New value (must be boolean).
    #
    method _rebuild {optname value} {
        if {![string is boolean -strict  $value]} {
            error "$optname can only take valid boolean values: '$value' is not boolean"
        }
        set options($optname) $value
        
        #  Destroy the existing widgets
        
        foreach widget [winfo children $win] {
            destroy $widget
        }
        
        #  Create the appropriate set:
        
        set havetitle $options(-havetitle)
        set haverun   $options(-haverun)
        set widgets   [list]
        
        if {$havetitle} {
            lappend widgets [ttk::label $win.tlabel -text Title: ]
            lappend widgets [ttk::entry $win.title -width 60 ]
            $win.title insert end $options(-title)
        }
        if {$haverun} {
            lappend widgets [ttk::label $win.rlabel -text {Run Number:} ]
            lappend widgets [ttk::entry $win.run \
                -width 6 -validate focusout \
                -validatecommand [mymethod _validate]]
            $win.run insert end $options(-run)
            bind $win.run <Return> [mymethod _validate]
        }
        
        # Grid them in a horizontal strip;
        
        if {[llength $widgets] > 0} {
            grid {*}$widgets
        }
        
    }
    ##
    #  _setState
    #
    #    Called to process a change in the widget state.  Note that only
    #    entry classed objects will be modified.
    #
    #  @param optname - name of the option being modified (-state).
    #  @param value   - new state.  Must be one of 'normal', 'disabled',
    #                   or readonly.  See ttk::entry(3tk) for the meaning
    #                   of these states.
    # 
    method _setState {optname value} {
        set allowedStates [list normal disabled readonly]
        if {$value ni $allowedStates} {
            error "$opname must be one of {[join $allowedStates {, }]} was $value"
        }
        foreach widget [winfo children $win] {
            if {[winfo class $widget] eq "TEntry"} {
                $widget configure -state $value
            }
        }
        set options($optname) $value
    }
    ##
    # _setTitle
    #   Set a new title for the widget.  Note this just saves the title if
    #   -havetitle is not true.
    #
    # @param optname  Name of the option being set (-title).
    # @param value    New value.
    #
    method _setTitle {optname value} {
        set options($optname) $value
        if {$options(-havetitle)} {
            $win.title delete 0 end
            $win.title insert end $value
        }
    }
    ##
    # _setRun
    #   set a new run number for the widget.  Note this just saves the
    #   run number if -haverun is not true.
    #
    # @param optname - Name of the option being configured (-run)
    # @param value   - New run number which must be an integer >= 0.
    #
    method _setRun   {optname value} {
        if {![string is integer -strict $value] || ($value < 0)} {
            error "$optname must be an integer >= 0 and was '$value'"
        }
        set options($optname) $value
        set lastRunNumber $value
        if {$options(-haverun)} {
            $win.run delete 0 end
            $win.run insert end $value
        }
    }
    ##
    # _getTitle
    #    If there is a title widget, the string is read from it into
    #    options($optname).  Regardless, the value of $options($optname) is
    #    returned.
    #
    # @param optname - the name of the option being queried (-title).
    # @return string - The current title value.
    #
    method _getTitle optname {
        if {$options(-havetitle)} {
            set options($optname) [$win.title get]
        }
        return $options($optname)
        
    }
    ##
    # _getRun
    #
    #   If there is a run number widget, the string is read from it into
    #   options($optname) regardless, $options($optname) is returned.
    #
    # @param optname - The option being fetched.
    # @return integer - Run number.
    #
    method _getRun   optname {
        if {$options(-haverun)} {
            set options($optname) [$win.run get]
        }
        return $options($optname)
    }
    #--------------------------------------------------------------------------
    # Action handlers
    #
    method _validate {} {
        set now [$win.run get]
        if {![string is integer -strict $now] || ($now < 0)} {
            tk_messageBox -title {Bad Run Number} -icon error \
                -message {Invalid run number - must be an integer >= 0} \
                -type ok -parent $win
            $win.run delete 0 end
            $win.run insert end $lastRunNumber
            return 0
        } else {
            set lastRunNumber $now
            return 1
        }
    }
}
#------------------------------------------------------------------------------
# Run identification API.  We need to cooperate with the
# main application to make this work.. ReadoutGUIPanel::runIdWidget must be
# set to the widget path of the RunIdentification widget.
#
namespace eval ::ReadoutGUIPanel {
    variable runIdWidget ""
}

##
# ::ReadoutGUIPanel::ghostWidgets
#
#    Disables all the widgets that need to be turned off whent he run is
#    either paused or active
#
# TODO:  Be sure to also get the recording enable checkbox when that's added.
#
proc ::ReadoutGUIPanel::ghostWidgets {} {
    $::ReadoutGUIPanel::runIdWidget configure -state readonly
}
##
# ::ReadoutGUIPanel::unghostWidgets
#
#  Enables all widges that need to be turned on when the run is halted.
#
#  TODO: Handle the recording checkbutton.
# 
proc ::ReadoutGUIPanel::unghostWidgets {} {
    $::ReadoutGUIPanel::runIdWidget configure -state normal
}
##
# ::ReadoutGUIPanel::getTitle
#
#   Return the run title
#
# @return string - the current run title string.
#
proc ::ReadoutGUIPanel::getTitle {} {
    return [$::ReadoutGUIPanel::runIdWidget cget -title]
}
##
# ::ReadoutGUIPanel::setTitle
#
#   Set a new title string in the widget:
#
# @param title - The new title string.
#
proc ::ReadoutGUIPanel::setTitle title {
    $::ReadoutGUIPanel::runIdWidget configure -title $title
}
##
# ::ReadoutGUIPanel::getRun
#
#  @return - The current run number.
#
proc ::ReadoutGUIPanel::getRun   {} {
    return [$::ReadoutGUIPanel::runIdWidget cget -run]
}
##
#  ::ReadoutGUIPanel::setRun
#
#   Set a new run number.  Note that validation is done by the widget so
#   I'm not going to be concerned with that here.
#
# @param run - The new run number.
#
proc ::ReadoutGUIPanel::setRun run {
    $::ReadoutGUIPanel::runIdWidget configure -run $run
}
##
# ::ReadoutGUIPanel::incrRun
#
#  Adds 1 to the current run number.
#
proc ::ReadoutGUIPanel::incrRun {} {
    set now [::ReadoutGUIPanel::getRun]
    incr now
    ::ReadoutGUIPanel::setRun $now
}

#-----------------------------------------------------------------------------
#
#  The next widget and API are involved in providing a UI for the run
#  state machine.
#


##
# @class RunControl
#    This megawidget provides a control panel for the run state machine.
#    buttons are enabled and disabled depending on the actual run state.
#    as reflected by the run state machine singleton.  We therefore
#    register methods as callout bundles in order to be notified of state
#    transitions so that the UI can be updated.
#    This going to be a pair of snit clases:
#   * RunControl - which provides the widget.
#   * RunControlSingleton - which provides a way for the RunstateMachine
#     callback bundle to locate the object to trigger state changes.
#     RunControlSingleton will be responsible for registering the callback
#     bundle as well.
#
# LAYOUT:
#    +-----------------------------------------------------+
#    |                [Start (button)]                     |
#    | [Begin (button)]                 [Pause (button)]   |
#    |                 [ ] Record                          |
#    +-----------------------------------------------------+
#
# OPTIONS:
#    -pauseable     - Boolean indicating if the pause button should be displayed.
#    -recording     - State of the record checkbox.
#
snit::widgetadaptor RunControl {
    component stateMachine
    
    option -pauseable -default 1 -configuremethod _changePauseVisibility
    option -recording -default 0
    
    #
    #  The variable below is an array indexed by state the values are two
    #  element lists of widget subnames and the state of each widget when
    #  we are in that state.
    #
   variable widgetStates -array [list \
        NotReady [list {start normal} {beginend disabled} {pauseresume disabled} \
                  {record normal}]                                                \
        Starting [list {start disabled} {beginend disabled} {pauseresume disabled} \
                  {record normal}]                                                \
        Halted   [list {start disabled} {beginend normal} {pauseresume disabled}  \
                  {record normal}]                                                \
        Active   [list {start disabled} {beginend normal} {pauseresume normal}    \
                  {record disabled}]                                              \
        Paused   [list {start disabled} {beginend normal} {pauseresume normal}     \
                    {record disabled}]                                            \
    ]
     
    ##
    #  The variable below contains an array indexed by state whose values
    #  are lists of 2 element sublists.  Each sublist contains the subname of a
    #  widget and the -text value it should have in that state.
    #
    
    variable widgetLabels -array [list   \
        NotReady [list {beginend Begin} {pauseresume Pause}]                \
        Starting [list {beginend Begin} {pauseresume Pause}]                \
        Halted   [list {beginend Begin} {pauseresume Pause}]                \
        Active   [list {beginend End}   {pauseresume Pause}]                \
        Paused   [list {beginend End}   {pauseresume Resume}]               \
    ]
    
    ##
    # constructor
    #
    #   Construct and configure the widget.
    #   - Install a ttk::frame as the hull.#
    #   - create and layout the widgets.
    #   - Install the RunStateMachine singleton as our stateMachine component.
    #   - Make our appearance match the statemachine's current state.
    #   - configure any options provided at instantiation time.
    #
    # @param args - The -option value configuration options provided at
    #               instantiation time.
    #
    constructor args {
        installhull using ttk::frame
        
        ttk::button $win.start       -text Start -command [mymethod _start]
        ttk::button $win.beginend    -text Begin -command [mymethod _beginend]
        ttk::button $win.pauseresume -text Pause -command [mymethod _pauseresume]
        ttk::checkbutton $win.record -text Record -onvalue 1 -offvalue 0 \
            -variable [myvar options(-recording)]
        
        grid $win.start -columnspan 2
        grid $win.beginend $win.pauseresume
        grid $win.record -columnspan 2
        
        install stateMachine using RunstateMachineSingleton %AUTO%
        $self _updateAppearance
        
        $self configurelist $args
        
    }
    ##
    # destructor
    #
    #  Destroy the stateMachine component:
    #
    destructor {
        if {$stateMachine ne ""} {
            $stateMachine destroy
        }
    }
    
    #---------------------------------------------------------------------------
    #  Configuration methods
    
    
    ##
    # _changePauseVisibility
    #
    #   Determines if the pause button is visible.  This can be turned off
    #   if not all data sources can pause a run.
    #
    # @param optname - Name of the configuration option that controls this.
    # @param value   - New value, must be boolean.
    method _changePauseVisibility {optname value} {
        if {![string is boolean -strict $value]} {
            error "$optname takes a boolean value and you gave me $value"
        }
        set options($optname) $value
        
        # See if pause is already gridded:
        
        set gridded [expr {"$win.pauseresume" in [grid slaves $win -row 1]}]

        if {$value} {
            if {!$gridded} {
                grid $win.pauseresume -row 1 -column 1
            }
            
        } else {
            if {$gridded} {
                grid forget $win.pauseresume
            }
        }
    }
    
    #--------------------------------------------------------------------------
    #  Button event handlers.
    
    ##
    # _start
    #   Start the data sources in the RunstateMachine.  Note that
    #   the appearance of the buttons will change only as a result of the callback
    #   bundle eventually invoking the _udpateAppearance method..which will look
    #   at the actual state machine and set button states accordingly.
    # 
    #
    method _start {} {
        $stateMachine transition Starting
    }
    ##
    #  _beginend
    #
    #  Invoked when the begin/end button is clicked.  The transition we actually
    #  initiate depends on the current  runstate:
    #  *  Halted -> Active
    #  * {Paused, Active} -> Halted.
    #  * Other    - A bug error.
    # @note The callback bundle will be responsible for invoking _updateAppearance
    #       to change the appearance of the button itself.
    # 
    method _beginend {} {
        set state [$stateMachine getState]
        if {$state eq "Halted"} {
            $stateMachine transition Active
        } elseif {$state in [list Paused Active]} {
            $stateMachine transition Halted
        } else {
            error "ERROR: begin/end button clicked when state is $state which should not happen"
        }
    }
    ##
    # _pauseresume
    #
    #  Invoked when the pause/resume button is clicked.
    #  Action depends on the state:
    #  *  Paused -> Active
    #  *  Active -> Paused
    #  * Any other -> Bug error
    method _pauseresume {} {
        set state [$stateMachine getState]
        if {$state eq "Paused"} {
            $stateMachine transition Active
        } elseif {$state eq "Active"} {
            $stateMachine transition Paused
        } else {
            error "ERROR: pause/resume button clicked when state is $state which should not happen"
        }
    }
    
    #---------------------------------------------------------------------------
    #  Other private methods:
    #
    
    ##
    # _updateAppearance
    #   Called to make the widget appearance consistent with the current run
    #   state.  See the variables widgetLabels and widgetStates for more
    #   information.
    #
    method _updateAppearance {} {
        set state [$stateMachine getState]
        
        # Widget states:
        
        if {[array names widgetStates $state] eq ""} {
            error "ERROR: State is $state but there's no corresponding set of widget states!"
        }
        foreach stateInfo $widgetStates($state) {
            set widget $win.[lindex $stateInfo 0]
            $widget configure -state [lindex $stateInfo 1]
        }
        
        # Widget labels:
        
        if {[array names widgetLabels $state] eq ""} {
            error "ERROR: State is $state but there's no corresponding set of widget labels"
        }
        foreach labelInfo $widgetLabels($state) {
            set widget $win.[lindex $labelInfo 0]
            $widget configure -text [lindex $labelInfo 1]
        }
    }
}
##
#   The functions/namespace below implement the run control singleton pattern.
#
namespace eval ::RunControlSingleton {
    variable theInstance ""
    namespace export attach enter leave
}
##
#  ::RunControlSingleton::getInstance
#
#     Returns the instance of the widget.
#
# @param path - (defaults to .runcontrol)  the widget path of the instance.
# @param args - (defaults to "") additional configuration options.
#
proc ::RunControlSingleton::getInstance {{path ""} args} {
    if {$::RunControlSingleton::theInstance eq ""} {
        set ::RunControlSingleton::theInstance [RunControl $path {*}$args]
        set stateMachine [RunstateMachineSingleton %AUTO%]
        $stateMachine addCalloutBundle RunControlSingleton
        $stateMachine destroy
    } elseif {[llength $args] > 0} {
        $::RunControlSingleton::theInstance configure {*}$args
    }
    return $::RunControlSingleton::theInstance
}

##
#  attach
#   Called when the bundle is attached to the run control state machine.
# @param state - the current machine state.
# 
proc ::RunControlSingleton::attach {state} {
    set rctl [::RunControlSingleton::getInstance]
    $rctl _updateAppearance

}
##
# enter
#    Called when the state machine enters a new state.
#
# @param from - old state.
# @param to   - Current state
#
proc ::RunControlSingleton::enter {from to} {
    set rctl [::RunControlSingleton::getInstance]
    $rctl _updateAppearance

}
##
#  leave
#   Called whenn the state machine leaves a state (unused)
#
proc ::RunControlSingleton::leave {from to} {}

##
#  API elements for all this fun:
#

##
#  ::ReadoutGUIPanel::recordOff
#
#   Request that the next run not record events.  This can be issued
#   during an active run, however it might give the mistaken impression
#   that recording is active now...The state of the recording checkbox
#   only matters when the run starts.
#
proc ::ReadoutGUIPanel::recordOff {} {
    set r [::RunControlSingleton::getInstance]
    $r configure -recording 0
}

##
# ::ReadoutGUIPanel::recordOn
#
#   Request that the next run record events.  See recordOff above, however
#
proc ::ReadoutGUIPanel::recordOn {} {
    set r [::RunControlSingleton::getInstance]
    $r configure -recording 1
}
##
#  ::ReadoutGUIPanel::recordData
#     Returns the state of the recording flag inthe readout GUI.  Note that this
#     reflects the state of the record checkbutton.  If application specific
#     code invoked one of the two procs above, this may not indicate whether
#     a currently active run is recording.
#
proc ::ReadoutGUIPanel::recordData {} {
    set r [::RunControlSingleton::getInstance]
    $r cget -recording
}