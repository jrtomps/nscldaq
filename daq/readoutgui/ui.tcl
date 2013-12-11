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
catch {package require Tk};    # In batch tests there's no display!
package require snit
package require RunstateMachine
package require StateManager

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
#  ::ReadoutGUIPanel::getRunIdInstance
#
# return/create the runid instance.
#
# @param widget (only looked at the first time)
# @return widget path - to the run id widget.
#
proc ::ReadoutGUIPanel::getRunIdInstance {{widget {}}} {
    if {$::ReadoutGUIPanel::runIdWidget eq ""} {
        set ::ReadoutGUIPanel::runIdWidget [RunIdentification  $widget]
    }
    return $::ReadoutGUIPanel::runIdWidget
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

#--------------------------------------------------------------------------------
#
#  Elapsed run time widget and related API elements.
#


##
# @class Stopwatch
#
#  This non graphical type is used to provide a self contained timer/stopwatch:
#
# METHODS:
#    start       - Start the timer from the last known initial time.
#    stop        - Stop the timer.
#    reset       - Reset the elapsed time to zero.
#    addAlarm    - add a Callback invoked at the global level
#                  when the elapsed time reaches a specified value.
#    removeAlarm - Remove an alarm.
#    isRunning   - Tests for the timer running.
#    elapsedTime - Get the elapsed time.
#
# NOTE:
#    There is the assumption the event loop is working since this all uses after.
#
snit::type Stopwatch {
    
    ##
    #  elapsedTime - Number of milli-seconds we've been running.
    #  afterId     - ID of the tcl after command/scheduler...-1 if not running.
    #  alarms      - Array of callbacks.  Array inidices are elapsed times in seconds
    #                values are lists of commands called when that time is hit.
    #
    variable elapsedTimeMs 0
    variable afterId    -1
    variable alarms -array [list]
    
    variable timerResolution 250;   # Ms per tick.
    
    ##
    # destructor needs to kill off any after scheduled so it does not call into
    # a deleted object:
    
    destructor {
        $self _stopTimer
    }
    #-------------------------------------------------------------------------
    # Public methods.
    #
    
    ##
    # Start the timer.  Note the elapsed time is not reset.  This is because
    # we are allowed to pause the timer (stop/restart)...e.g. if a run is paused.
    #
    # @throw  if the timer is already running
    #
    method start {} {
        if {![$self isRunning] } {
            $self _startTimer
        } else {
            error "Timer is already running!"
        }
    }
    ##
    # Stop the timer.
    #
    # @throw  if the timer is already halted.
    #
    method stop {} {
        if {[$self isRunning]} {
            $self _stopTimer
        } else {
            error "Timer is already stopped"
        }
    }
    ##
    # Reset the elapsed time to zero.  Like a stopwatch this is allowed when
    # the timer is running, however in the run control case, this will typically
    # happen when the run state machine leaves halted for active.
    #
    method reset {} {
        set elapsedTimeMs 0
    }
    ##
    #  addAlarm
    #
    #  Add an alarm to the stopwatch.  This is normally used to
    #  force an end to a timed run.  The alarm is called when the elapsedTimeMs
    #  is at least when seconds of total time.
    #
    # @param when   - When in seconds the alarm script should fire.
    # @param script - The script to run (at global level) if the alarm fires.
    #
    # @note several alarms can be set...even on the same time.
    # @note recurring alarms are not yet supported, however a script can
    #       remove itself and schedule a later alarm for itself.
    # @throw if when is not an integer or is less than zero.
    #
    method addAlarm {when script} {
        if {![string is integer -strict $when] || ($when < 0)} {
            error "Alarms must be set on an integer number of seconds > 0: $when"
        }
        set when [expr {$when*1000}]
        lappend alarms($when) $script
    }
    ##
    # removeAlarm
    #
    #  Removes an existing alarm.
    #
    # @param when - the time the alarm was set form.#
    # @param script - The script to remove.
    #
    # @throw - If when is illegal (see addAlarm)
    # @throw - If there are no alarms for when.
    # @throw - If the script is not registered at the time specified by when.
    #
    method removeAlarm {when script} {
        if {![string is integer -strict $when] || ($when < 0)} {
            error "Alarms can only be removed from an integer number of ssecnods > 0: $when"
        }
        set whenMs [expr {$when*1000}]
        if {[array names alarms $whenMs] eq ""} {
            error "There are no alarms set for $when"
        }
        
        set whichOne [lsearch -exact $alarms($whenMs) $script]
        if {$whichOne == -1} {
            error "The script specified was not registerd as an alarm for $when :\n $script"
        }
        set alarms($whenMs) [lreplace $alarms($whenMs) $whichOne $whichOne]
        if {[llength $alarms($whenMs)] == 0} {
            array unset alarms $whenMs;     # eliminate empty lists.
        }
    }
    ##
    # isRunning
    #
    # @return bool - True if the timer is timing false otherwise.
    #
    method isRunning {} {
        return [expr {$afterId != -1}]
    }
    ##
    # elapsedTime
    #   Give the elapsed time in milliseconds.
    method elapsedTime {} {
        return $elapsedTimeMs
    }
    #--------------------------------------------------------------------------
    # Private methods
    
    
    ##
    # _startTimer
    #
    #   Set up the first after.  Note that the caller is supposed to ensure
    #   we are running.
    #
    method _startTimer {} {
        set afterId [after $timerResolution [mymethod _tick]]
    }
    ##
    # _stopTimer
    #   Stops the timer.  The caller is supposed to ensure it's not already stopped.
    #
    method _stopTimer {} {
        after cancel $afterId
        set afterId -1
    }
    ##
    # _tick
    #    called when timer tick must be counted.
    #
    method _tick {} {
        incr elapsedTimeMs $timerResolution
        $self _startTimer;     # schedules the next one.
        $self _callScripts;    # this order gives a more stable time.
    }
    ##
    # _callScripts
    #    Invokes all the scripts set for this elapsed time.
    #
    method _callScripts {} {
        if {[array names alarms $elapsedTimeMs] ne ""} {
            foreach script $alarms($elapsedTimeMs) {
                uplevel #0 $script
            }
        }
    }
}
##
# @class ElapsedTimeDisplay
#
#    UI element that shows an elapsed time.  Note the timing functions are
#    performed by a Stopwatch component which has its full user interface
#    exposed however reset is intercepted to re-set our first alarm.
#
snit::widgetadaptor ElapsedTimeDisplay {
    component clock
    delegate method * to clock
    
    variable formattedNow "0 00:00:00"
    variable nextAlarm     1
    
    ##
    #  constructor
    #    * Hull is a tk::frame.
    #    * The contents are a label that tracks formattedNow.
    #    * We establish our first alarm handler at 1 second to update now.
    #    * No configuration processing is done.
    constructor args {
        installhull using ttk::frame
        install clock using Stopwatch %AUTO%
        
        ttk::label $win.time -textvariable [myvar formattedNow]
        ttk::label $win.label -text {Active Run Time}
        grid $win.time $win.label -sticky nsew 
        
        $clock addAlarm 1 [mymethod _tick]
    }
    
    destructor {
        destroy $clock
    }
    ##
    # reset
    #
    #  Intercept the reset method so that we can kill off our nextAlarm
    #  and reset the alarm back to 1.
    #
    method reset {} {
        $clock removeAlarm $nextAlarm [mymethod _tick]
        $clock addAlarm     1         [mymethod _tick]
        set nextAlarm       1
        
        set formattedNow "0 00:00:00"
        
        $clock reset
    
    }
    #--------------------------------------------------------------------------
    # private methods
    
    ##
    # _tick
    #    Alarm handler from the stopwatch.
    #    - Compute the new formattedNow from the elapsed time.
    #    - Kill off the alarm that fired.
    #    - Set up our next alarm.
    #
    method _tick {} {
        set now [$clock elapsedTime]
        $self _formatTime $now
        
        # Reschedule:
        
        $clock removeAlarm $nextAlarm [mymethod _tick]
        incr nextAlarm
        $clock addAlarm $nextAlarm [mymethod _tick]
    }
    ##
    # _formatTime
    #     compute the new value for formatted now
    #
    # @param now - Now in ms.
    #
    method _formatTime {now} {
        
        # Convert to now in ms to seconds:
        
        set now [expr {int($now/1000)}]
        
        #  Figure out the broken down time:
        
        set seconds [expr {$now % 60}]
        set now     [expr {int($now/60)}]; # Minutes
        set min     [expr {$now % 60}]
        set now     [expr {int($now/60)}]; # Hours
        set hours   [expr {$now %24}]
        set days    [expr {int($now/24)}]
        
        # Format it:
        
        set formattedNow [format "%d %02d:%02d:%02d" $days $hours $min $seconds]
    }
}
##
# Encapsulate the elapsed time GUI element as a singleton so that
# it can be accessed consistently by the API elements in the ReadoutGUIPanel
# package.
#
namespace eval  ::ElapsedTime {
    variable theInstance ""
    namespace export attach enter leave
}

##
#  ::ElapsedTime::getInstance
#    Returns the elapsed time instance singleton:
#    *   If it does not yet exist it is created with the widget path provided.
#    *   Regardless the single instance is returned.
#
# @param  w - The widget path (optional unless this creates) to the widget.
# @return string -widget path to the singleton.
#
proc ::ElapsedTime::getInstance {{w ""}} {
    if {$::ElapsedTime::theInstance eq ""} {
        set ::ElapsedTime::theInstance [ElapsedTimeDisplay $w]
        set rstate [RunstateMachineSingleton %AUTO%]
        $rstate addCalloutBundle ElapsedTime
        $rstate destroy
    }
    return $::ElapsedTime::theInstance
}
##
# ::ElapsedTime::attach
#
#   Called at attach time.  If the run is active we're going to start
#   the clock even though the elapsed time won't be right.
#
# @param state - The state we are in at the time of attachment.
#
proc ::ElapsedTime::attach {state} {
    if {$state eq "Active"} {
        set timer [::ElapsedTime::getInstance]
        $timer start
    }
}
##
# ::ElapsedTime::enter
#
#  Called when a state is entered:
#  *  Halted -> Active : clear and start timer.
#  *  Paused  -> Active : start.
#  *  Active -> Anything other than active : Stop timer
#
# @param from - Prior state.
# @param to   - Current state.
#
proc ::ElapsedTime::enter  {from to} {
    set timer [::ElapsedTime::getInstance]
    if {($from eq "Halted") && ($to eq "Active")} {
        $timer reset
        $timer start
    } elseif {($from eq "Paused") && ($to eq "Active")} {
        $timer start
    } elseif {$from  eq "Active"} {
        $timer stop
    }
}
##
# ::ElapsedTime::leave
#
#   Called when a state is being left.  This is not used by us.
#
# @param from - state being left.
# @param to   - State we are going to enter.
proc ::ElapsedTime::leave  {from to} {}



##
# ReadoutGUIPanel::getRunTime
#
#   Returns the current time into the run in seconds.
#
# @return integer
#
proc ::ReadoutGUIPanel::getRunTime {} {
    set w [::ElapsedTime::getInstance]
    return [expr {int([$w elapsedTime]/1000)}]
}
#---------------------------------------------------------------------------
#
# Timed run controls and API
#  Note that there's the usual singleton and a registration bundle that
#  makes it possible, when a timed run is active to register an alarm with the
#  elapsed time that ends the run.
# 
#

##
# @class TimedRunControls
#
#   This control consists of a mechanism to choose a run duration along
#   with a checkbutton that is lit when a run should be timed.
#   
# LAYOUT
#   +-----------------------------------------------------+
#   |  [ ] timed run  [ ] days  [ ] hrs [ ] min [ ] sec   |
#   +-----------------------------------------------------+
#
#   Days is a spin box while hrs/min/sec are comboboxes.
#
#  OPTIONS:
#    -state  - State of the controls (disabled, normal, readonly e.g.).
#    -days   - Number of days
#    -hours  - Number of hours
#    -mins   - Number of minutes
#    -secs   - Number of seconds.
#    -timed  - If the run is timed.
#
snit::widgetadaptor TimedRunControls {
    option -state -default normal -configuremethod _changeState
    option -days  -default 0 -configuremethod _changeDays
    option -hours 0
    option -mins  0
    option -secs  0
    option -timed 0
    
    #  List of widget path tails to the widgets controlled by the
    #  -state option.
    
    variable editableWidgets [list onoff days hrs min secs]
    
    ##
    # constructor
    #    Build the widget and lay it out.
    #
    # @param args -the -option/value configuration pairs.
    #
    constructor args {
        installhull using ttk::frame
        
        # Make a list that runs 0-59
        
        for {set i 0} {$i < 60} {incr i} {
            lappend values $i
        }
        
        ttk::checkbutton $win.onoff  \
            -text {Timed Run} -onvalue 1 -offvalue 0 -variable [myvar options(-timed)]
        
        ttk::spinbox $win.days -from 0 -to 36500 -increment 1 \
            -command [mymethod _updateDays] -format "%6.0f"  -width 6
        $win.days set 0
        
        ttk::label $win.daysep -text "-"
        ttk::combobox $win.hrs -values [lrange $values 0 23] \
            -textvariable [myvar options(-hours)] -width 2
        ttk::label $win.hrsep -text ":"
        ttk::combobox $win.min -values $values -width 2 \
            -textvariable [myvar options(-mins)]
        ttk::label $win.minsep -text ":"
        ttk::combobox $win.secs -values $values -width 2 \
            -textvariable [myvar options(-secs)]
        
        
        $self configurelist $args
        
        grid $win.onoff $win.days $win.daysep $win.hrs $win.hrsep $win.min $win.minsep $win.secs
        
    }
    #---------------------------------------------------------------------------
    #
    #  Configuration operations:
    
    ##
    # _changeState
    #    Modifies the state of all of the widgets that arec changeable.
    #    This is used when e.g. the run is active, to prevent the values and
    #    enable from being modified.
    #
    # @param optname - the name of the option that is being modified (-state).
    # @param value   - the new value which is one of normal or disabled.
    #
    method _changeState {optname value} {
        set options($optname) $value
        foreach tail $editableWidgets {
            $win.$tail configure -state $value
        }
        
    }
    ##
    # _changeDays
    #   Called when the -days value was configured.  Updates the
    #   widget contents.
    #
    # @param optname - the option being configured.
    # @param value   -  new value.
    #
    method _changeDays  {optname value} {
        set options($optname) $value
        $win.days set $value
    }
    #---------------------------------------------------------------------------
    #  Action handlers.
    #
    
    ##
    # Called when the days widget is modified.  This modification is propagated
    # to options(-days)
    #
    method _updateDays {} {
        set options(-days) [$win.days get]
    }
    
}
#-----------------------------------------------------------------------------
# Singleton pattern implementation and API:

namespace eval ::TimedRun {
    variable theInstance ""
    variable lastAlarmTime 0
    namespace export attach enter leave
}

##
# TimedRun::getInstance
#    Returns the singleton instance, creating it if needed.
#
# @param win - optional parameter that is the path to the widget.
# @param args - optional parameter that are the configuration option/value pairs.
#
# @return win - Path to the object.
#
proc ::TimedRun::getInstance {{win ""} args} {
    if {$::TimedRun::theInstance eq ""} {
        set ::TimedRun::theInstance [TimedRunControls $win]
        if {[llength $args] > 0} {
            $win configure {*}$args
        }
        set sm [RunstateMachineSingleton %AUTO%]
        $sm addCalloutBundle TimedRun
    }
    return $::TimedRun::theInstance
}

#  Callback bundle:

##
# ::TimedRun::attach
#
#    Called when the singleton instance is attached to the state machine.
#    set the widget state appropriately:
#    *   {Paused, Active} -> disabled.
#    *    other           -> normal
#
proc ::TimedRun::attach {state} {
    
    set w  [::TimedRun::getInstance]
    if {$state in [list Paused Active]} {
        set state disabled
    } else {
        set state normal
    }
    $w configure -state $state
}
##
# ::TimedRun::leave
#    Called when a state is left (not used).
#
# @param from - the state we are leaving
# @param to   - The next state.
#
proc ::TimedRun::leave {from to} {}
##
#  ::TimedRun::enter
#
#    Called when a new state has been entered.
#    *  any -> {Active, Paused} -> state disabled.
#    *  any other transitions   -> state normal
#    *  Halted -> Active and timed run button set, set an alarm to end the run.
#    *  {Paused, Active} ->Halted and timed run button set, cancel the alarm
proc ::TimedRun::enter {from to} {
    
    set w [::TimedRun::getInstance]
    
    # normal/disabled state handling:
    
    if {$to in [list Active Paused]} {
        set state disabled
    } else {
        set state normal
    }
    $w configure -state $state
    
    #  Timed run handling:
    
    if {[$w cget -timed]} {
        set et [::ElapsedTime::getInstance]
        if {($from eq "Halted") && ($to eq "Active")} {
            
            # Compute the run length, save it in ::TimedRun::lastAlarmTime
            # and setup an _alarm for tha time.
            
            set secs [$w cget -secs]
            set mins [$w cget -mins]
            set hrs  [$w cget -hours]
            set days [$w cget -days]
            
            set runTime [expr {(($days*24 + $hrs)*60 + $mins)*60 + $secs}]
            
            $et addAlarm  $runTime ::TimedRun::_alarm
            set ::TimedRun::lastAlarmTime $runTime
            
        } elseif {($from in [list Paused Active]) && $to eq ("Halted")} {
            # Remove the last alarm set.
            
            $et removeAlarm $::TimedRun::lastAlarmTime ::TimedRun::_alarm
        }
    }
}

##
# ::TimedRun::_alarm
#
#   Called when a timed run time expires.  This is just going to request
#   a transition to halted.  The state machine callbacks do the rest of the work.
#
proc ::TimedRun::_alarm {} {
    set sm [RunstateMachineSingleton %AUTO%]
    $sm transition Halted
    $sm destroy
}

# API Functions for the length of the run.

##
# ReadoutGUIPanel::isTimed
#
# @return boolean - if the GUI's timed button is checked.
#
proc ::ReadoutGUIPanel::isTimed {} {
    set w [::TimedRun::getInstance]
    return [$w cget -timed]
}
##
# ReadoutGUIPanel::setTimed
#
#   Sets the state of teh GUI time checkbutton.
#
# @param state - new state (boolean)
#
proc ::ReadoutGUIPanel::setTimed {state} {
    set w [::TimedRun::getInstance]
    $w configure -timed $state
}
##
# ::ReadoutGUIPanel::getRequestedRunTime
#
# @return integer - number of seconds in the d-hh:mm:ss request.
#
proc ::ReadoutGUIPanel::getRequestedRunTime {} {
    set w [::TimedRun::getInstance]
    set secs [$w cget -secs]
    set mins [$w cget -mins]
    set hrs  [$w cget -hours]
    set days [$w cget -days]
    
    return [expr {
        ((($days*24) + $hrs * 60) + $mins *60) + $secs
    }]
}
##
# ::ReadoutGUIPanel::setRequestedRunTime
#
#   Set the length of a timed run.  This only has meaning
#   *  If the timed run is enabled (::ReadoutGUIPanel::setTimed 1)
#   *  At the start of the next run.
#
# @param secs  - number of seconds desired for the next run.
#
proc ::ReadoutGUIPanel::setRequestedRunTime secs {
    set seconds   [expr {$secs % 60}]
    set remainder [expr {int($secs / 60)}]
    set mins      [expr {$remainder % 60}]
    set remainder [expr {int($remainder /60)}]
    set hours     [expr {$remainder % 24}]
    set days      [expr {int($remainder / 24) }]
    
    set w [::TimedRun::getInstance]
    $w configure -secs $seconds -mins $mins -hours $hours -days $days
}
#------------------------------------------------------------------------------
#  Logging/output

##
# @class OutputWindow
#   Provides a widget that can be used to output lines of text. Two interesting
#   features of the output window are the history limit and logging classes.
#   The -history option allows the client to configure the number of lines of
#   text that will be retained on the widget.  This prevents the memory storage
#   of the widget from growing without bounds.
#
#  Logging classes are used with the log method.   The -logclasses option
#  defines a list of log item types that the log method will accept (an
#  error is thrown if log is called with a log type not in that list).
#  -showlog defines a list of log item types the log method will actually
#  display on the screen.  The initial set of log classes are:
#  *   output - intended to display output from some source (e.g. a data source program).
#  *   log    - intended to log some interesting event (e.g. the run started).
#  *   error  - intended to log some error condition.
#  *   warning- intended to log some condition that could be a problem.
#  *   debug  - intended for debugging output.
#
#  -showlog defines how or if items will be displayed.  It consists of a
#      list in the form accepted by array set.  The keys to the array are
#      log classes.  If a log class is not in the list it will not be displayed.
#      the elements of the array are lists of option/value pairs where each
#      option is an option accepted by the text widget tag configure operation.
#      As you might guess, log entries are given a tag that matches their logclass
#      Therefore changes to the -showlog are dynamic with the exception of the
#      addition/removal of classes from the list which only affect future log
#      operations. By default these are (note debug is not displayed):
#      *  output [list]
#      *  log    [list]
#      *  error  -foreground red -background white
#      *  warning -foreground yellow
#     @note a dict could be just as easily used as a list with keys the option names
#           and values the option values.
#
# LAYOUT:
#   +---------------------------------------------------------------+
#   | +---------------------------------------------+--+            |
#   | |  text widget                                |sb|            |
#   | +---------------------------------------------+--+            |
#   | |    scrollbar                                |               |
#   | +---------------------------------------------+
#   +---------------------------------------------------------------+
#
# OPTIONS:
#    -foreground - Widget foreground color
#    -background - Widget background color
#    -width      - Width of widget in characters.
#    -height     - Height of widget in characters.
#    -history    - Number of lines of historical text that are retained.
#    -logclasses - Defines the set of log classes accepted by the log method.
#    -showlog    - Defines which of the log classes will actually be displayed.
# METHODS:
#   puts         - puts data to the widget
#   clear        - clear the entire text and history.
#   log          - Make a timestamped log entry.
#   get          - Returns all of the characters in windowm, visible and historic
#   open         - Opens a log file for the widget.  The file is opened for
#                  append access. From this point on all data output will be logged
#                  to this file.
#   close        - closes the log file.
#
snit::widgetadaptor OutputWindow {
    component text
    
    delegate option -foreground to text
    delegate option -background to text
    delegate option -width      to text
    delegate option -height     to text

    option -history    -default 1000
    option -logclasses -default [list output log error warning debug]
    option -showlog    -default [list                         \
        output  [list]                                        \
        log     [list]                                        \
        error   [list -background white -foreground red]      \
        warning [list -foreground magenta]                     \
    ] -configuremethod _updateTagOptions
    
    # If non empty, this is the log file fd.
    
    variable logfileFd  ""
    
    ##
    # constructor
    #   Builds the text widget along with horizontal and vertical scrollbars.
    #   the widget is set to -state disabled so that users can't type at it.
    #
    # @param args - The configuration options for the text widget.
    #
    constructor args {
        installhull using ttk::frame
        
        # Widget creation
        
        install text using text $win.text -xscrollcommand [list $win.xsb set] \
            -yscrollcommand [list $win.ysb set] -wrap none -background grey
        ttk::scrollbar $win.ysb -orient vertical   -command [list $text yview]
        ttk::scrollbar $win.xsb -orient horizontal -command [list $text xview]
        
        # widget layout:
        
        grid $text $win.ysb -sticky nsew
        grid $win.xsb       -sticky new
        
        $self configurelist $args
        $self _updateTagOptions -showlog $options(-showlog)
    }
    ##
    # destructor
    #   Ensure we don't leak file descriptors:
    #
    destructor {
        if {$logfileFd ne ""} {
            close $logfileFd
        }
    }
    
    #---------------------------------------------------------------------------
    # Public methods:
    #
    
    ##
    # puts
    #   Puts simple text to the output window.
    #
    # @param args - This can be one or two parameters:
    #               *  text       - the text is output followed by a newline.
    #               * -nonewline  - The text is output with no trailing newline.
    #
    method puts  {args}       {

        if {[llength $args] == 1} {
            set line "[lindex $args 0]\n"
        } elseif {([llength $args] == 2) && ([lindex $args 0] eq "-nonewline")} {
            set line [lindex $args 1]
        } else {
            error "use puts ?-nonewline Text"
        }
        $self _output "__notag___" $line
    }
    ##
    # clear
    #    Clears the entire widget.
    #
    method clear {}           {
        $text delete 0.0 end
    }
    ##
    # log
    #    Make a log like message.  A log message gets a timestamp and is
    #   associated with log class (-logclasses).  Further more, -showlog can
    #   associate rendering options with these messages.
    #
    # @param class - The log class associated with the message (must be in
    #                -logclasses)
    # @param msg   - The messagde to log.
    method log   {class msg} {
        set timestamp [clock format [clock seconds]]
        if {$class ni $options(-logclasses)} {
            error "'$class' is not one of the known classes: {$options(-logclasses)}"
        }
        array set logRenditions $options(-showlog)
        if {$class in [array names logRenditions]} {
            $self _output $class "$timestamp : $class : $msg\n"
        }
    }
    ##
    # get
    #   Return the text in the widget.  No effor is made to return information
    #   about the rendering of the text... only the text itself is returned.
    #
    # @return - string containing all of the text in the output window.
    #
    method get   {}           {
        return [$text get 0.0 end]
    }
    ##
    # open
    #   Open a file and log subsequent output to it as well as to the window.
    #   *   If a file is already open it is closed.
    #   *   the file is opened for append access (a+).
    #   *   Output will be logged to the file until close is called or until
    #       a different file is opened.
    #
    # @param filename  - Name of the file to open.
    #
    # @throw if the file is not writable.
    #
    method open  {filename}   {
    
        #  Close any open file:
        
        if {$logfileFd ne ""} {
            $self close
        }
        #  Open the new file
        
        set logfileFd [open $filename "a+"]
    }
    ##
    # Close the log file.
    # @throw it is an error to close when no log file is active.
    #
    method close {}           {
        if {$logfileFd eq ""} {
            error "No logfile is open"
        } else {
            close $logfileFd
            set logfileFd ""
        }
    }
    
    #--------------------------------------------------------------------------
    #  Configuration processors.
    
    
    ##
    # _updateTagOptions
    #
    #  Processes a new set of -showlog values.  These get turned into tag
    #  configuration.
    #
    # @param optname  - The option being configured
    # @param value    - The new value for this option.
    #
    # @note see the class comments for more informationabout the value.
    #
    method _updateTagOptions {optname value} {
        #
        #  By putting the values into an array whose values are treated as dicts
        #  locally we
        #  - make it easy to iterate over the tags.
        #  - Syntax check the dictionary definition lists.
        
        array set tagInfo $value
        
        foreach tag [array names tagInfo] {
            set tagConfig $tagInfo($tag)
            if {([llength $tagConfig] % 2) == 1} {
                error "log option lists must be even but the one for '$tag' is not: {$tagConfig}"
            }
            dict for {tagopt tagoptValue} $tagConfig {
                $text tag configure $tag $tagopt $tagoptValue
            }
        }
        
        #
        # If we got here, everything is legal:
        #
        set options($optname) $value
        
    }
    #--------------------------------------------------------------------------
    # Private methods
    
    ##
    # _output
    #    Centralized output/file-logging/history-trimming output method:
    #
    # @param tag  - Tag to associate with the output.
    # @param text - Data to output.
    #
    method _output {tag data} {
        
        # Output the data to the widget:
        
        $text insert end $data  $tag
        $text yview -pickplace end
        
        # Limit the number of lines that can appear.
        
        set lines [$text count -lines 0.0 end]
        if {$lines > $options(-history)} {
            $text delete 0.0 "end - $options(-history) lines"
        }
        # Log to file if we must:
        
        if {$logfileFd != ""} {
            puts -nonewline $logfileFd $data
            flush $logfileFd
        }
    }
}

#
#  Singleton and bundle to ensure state transitions get logged.
#
namespace eval ::Output {
    variable theInstance ""
    namespace export enter leave attach
}


##
# Output::getInstance
#
#   Return the singleton instance of the OutputWindow creating it if needed.
#
# @param win - (Only required for the creation). Window path for the output window.
# @param args- (Optional, only seen at creation). Configuration parameters for
#               the output window.
#
proc Output::getInstance { {win {}} args} {
    if {$::Output::theInstance eq ""} {
        set ::Output::theInstance [OutputWindow $win {*}$args]
        set sm [RunstateMachineSingleton %AUTO%]
        $sm addCalloutBundle Output
        $sm destroy
    }
    return $::Output::theInstance
}
#
# Callout bundle methods.. These log stuff to the singleton OutputWindow.
#

##
# ::Output::attach
#   Outputs a debug message indicating attached and which state.
#
# @param state - The current state.
#
proc ::Output::attach {state} {
    set w [::Output::getInstance]
    $w log debug "Attached to state machine state is: $state"
}
##
# ::Output::enter
#   State machine finished a transition.
#   Make a log level entry indicating we completed a state transition.
#
# @param from - the state we left.
# @param to   - The state we are now in.
#
proc ::Output::enter  {from to} {
    set w [::Output::getInstance]
    $w log log "Run state changed: $from -> $to"
}
##
# ::Output::leave
#   Called when a state transition is beginning.
#   Make a debug entry indicating this.
#
# @param from - State we are leaving.
# @param to   - State we are headed for
#
proc ::Output::leave  {from to} {
    set w [::Output::getInstance]
    $w log debug "Run leaving state $from heading for state $to"
}

#
#  API for the output window.
#

##
#  ::ReadoutGUIPanel::isRecording
#     Set the text widget to the recording colors.
#     (dark green background with white foreground).
# 
proc ::ReadoutGUIPanel::isRecording {} {
    set w [::Output::getInstance]
    $w configure -background darkgreen -foreground white
}
##
# ::ReadoutGUIPanel::notRecording
#   set the text widget to the non-recording colors of background grey
#   foreground black.
#
proc ::ReadoutGUIPanel::notRecording {} {
    set w [::Output::getInstance]
    $w configure -background grey -foreground black
}
##
# ::ReadoutGUIPanel::normalColors
#
#  Synonym for notRecording.
#
proc ::ReadoutGUIPanel::normalColors {} {
    ::ReadoutGUIPanel::notRecording   
}
##
# ::ReadoutGUIPanel::outputText
#    Output a line of text to the output widget.
#
# @param text - text to write,  a newline is appended to it.
#
proc ::ReadoutGUIPanel::outputText {text} {
    set w [::Output::getInstance]
    $w puts $text
}
##
# ::ReadoutGUIPanel::log
#
#   Creates a log entry on on the text window.
#
# @param src  - The creator of the log.
# @param class - The log class (see -logclasses option on OutputWindow)
# @param msg   - Meat of the message.
#
proc ::ReadoutGUIPanel::Log {src class msg} {
    set w [::Output::getInstance]
    $w log $class "$src: $msg"
}

#------------------------------------------------------------------------------
# Status area -- This will be a set of very customizable lines.
#

##
# @class StatusArea
#
#   Provides a very customizable widget for displaying status information.
#   The status information is divided into several status lines.
#   Each status line is internally managed but has contents that are externally
#   controlled.
#
# METHODS:
#    addWidget   - Add an arbitrary widget to the bottom of the status area.
#    addMessage  - Add a message line to the bottom of the status area.
#    setMessage  - Set the text of a message widgetg
#    statusItems - List the set of status items that are managed.
#    messageHandles - List the message handles.
#
snit::widgetadaptor StatusArea {
    delegate option * to hull
    
    variable index 0
    variable messages [list]
    
    ##
    # constructor
    #    Construct the container
    constructor args {
        installhull using ttk::frame
        
        $self configurelist $args
    }
    ##
    #  addWidget
    #    Adds as widget and grid's it at the bottom of the status
    #    line.  The widget is made sticky nsew.
    #
    # @param args - Widget creation command line minus the window name.
    #               e.g. to add a label widget that tracks the
    #               value of some global varialbe 'theText':
    #               $sa addWidget label -textvariable ::theText
    #
    # @return widget path - the created widget.
    #    
    method addWidget args {
        set command [lindex $args 0]
        set config  [lrange $args 1 end]
        set window  $win.child[incr index]
        
        $command $window {*}$config
        grid $window -sticky nsew
        
        return $window
    }
    ##
    # addMessage
    #    Convenience method that adds a label into which
    #    status text can be placed.
    #
    # @param msg - (optional) initial message contents.
    #
    # @return integer - handle that can be used in future calls to setMessage
    #
    method addMessage {{msg {}}} {
        set win [$self addWidget ttk::label -text $msg]
        set handle [llength $messages]
        lappend messages $win
        return $handle
    }
    ##
    # setMessage
    #
    #   Sets new contents for a status message line.
    #
    # @param handle - A message handle gotten from addMessage.
    # @param text   - The new text
    # @param args   - optional additional configuration options
    #                 e.g. -foreground -background
    #
    method setMessage {handle text args} {
        if {$handle >= [llength $messages]} {
            error "Invalid message handle"
        }
        set widget [lindex $messages $handle]
        
        $widget configure -text $text {*}$args
    }
    ##
    # statusItems
    #
    #  Returns the list of widgets that are managed as top level status
    #  items.  Note that if the client creates a frame stocked with widges,
    #  this only returns that frame not the contents of the frame.
    #  Further more, message objects are no different than any other widget
    #
    # @return list - List of widget windows.
    #
    method statusItems {} {
        return [winfo children $win]
    }
    ##
    # messageHandles
    #
    # @return list - of message object handles.
    #
    method messageHandles {} {
        return $messages
    }
    
}
##
#  Singleton pattern implementation for the status bar.
#
namespace eval ::StatusBar {
    variable theInstance ""
}

##
# ::StatusBar::getInstance
#
# Return the current instance of the status bar, creating it if needed.
#
# @param widget  -Path to the widget -- only looked at in the first call.
#
proc ::StatusBar::getInstance {{widget {}}} {
    if {$::StatusBar::theInstance eq ""} {
        set ::StatusBar::theInstance [StatusArea $widget]
    }
    return $::StatusBar::theInstance
}

#-----------------------------------------------------------------------------
#
#  Full Run control GUI base.  We build using the singletons so that
#  external bits and pieces of the run control system can find us.
#

##
# @class ReadoutGUI
#    The fully assembled readout control panel.
#
# LAYOUT:
#
#    +------------------------------------------------+
#    | Run identification                             |
#    | +-----------------------+                      |
#    | | Run controls          | Elapsed time         |
#    | |                       | Timed Run            |
#    | +-----------------------+                      |
#    | +-------------------------------------------+  |
#    | |    Output window.                         |  |
#                    ...
#    | +-------------------------------------------+  |
#    |    status lines                                |
#                          ...
#    +------------------------------------------------+
#
snit::widgetadaptor ReadoutGUI {
    component menubar
    component runid
    component runcontrol
    component elapsedtime
    component timedrun
    component output
    component statusbar
    
    delegate option * to hull
    
    ##
    # constructor
    #   All the work gets done here.  The remainder of the application
    #   just connects via the singletons and the run state machine.
    #
    #
    # @param args - hull (ttk::frame) options
    #
    constructor args {
        installhull using ttk::frame
        $self configurelist $args
        
        #  Make the menubar infrastructure.  We only handle the
        #  File menu.  See _populateFileMenu for more. There is an implicit
        #  assumption that we are in the top level widget.
        #  TODO:  - figure out how to not make that true.
        
        install menubar using readoutMenubar $win.menu
        . configure -menu $menubar
        $self _populateFileMenu
 
         # Install the components
        
        
        install runid using ::ReadoutGUIPanel::getRunIdInstance $win.runid
        install runcontrol using ::RunControlSingleton::getInstance $win.rctl
        install elapsedtime using ::ElapsedTime::getInstance $win.elapsed
        install timedrun    using ::TimedRun::getInstance $win.timed
        install output      using ::Output::getInstance $win.output
        install statusbar   using ::StatusBar::getInstance $win.status
        
        # Lay them all out:
        
        grid $runid       -sticky nsew  -columnspan 2 -row 0 -column 0
        grid $runcontrol  -sticky nsew  -rowspan    2 -row 1 -column 0
        grid $elapsedtime -sticky nsew -row 1 -column 1
        grid $timedrun    -sticky nsew -row 2 -column 1
        grid $output      -sticky nsew -row 3 -column 0 -columnspan 2
        grid $statusbar   -sticky nsew -row 4 -column 0 -columnspan 2
        
        
        # configure
        
        $self configurelist $args

    }
    #--------------------------------------------------------------------------
    #  Private methods
    #
    
    ##
    # _populateFileMenu
    #   Populate the file menu which consists of the following:
    #   * Load        - Loads a script by doing a source at the global level.
    #   * Add Library - Adds a library directory to the auto_path.
    #   * <separator>
    #   * Log         - Select a log file for the output widget.
    #   * Log Disable - Turn off logging in the output wigdget.
    #   * <separator>
    #   * Exit        - Ends any active run and exits the application.
    #
    method _populateFileMenu {} {
        $menubar addMenu File
        $menubar addCommand   File Load... [mymethod _sourceFile]
        $menubar addCommand   File {Add Library...} [mymethod _extendAutoPath]
        $menubar addSeparator File
        $menubar addCommand   File Log... [mymethod _chooseLogfile]
        $menubar addCommand   File {Disable Log} [mymethod _stopLogging]
        $menubar addSeparator File
        $menubar addCommand   File {Exit...} [mymethod _Exit]
    }

    
    #--------------------------------------------------------------------------
    #  Private methods:
    #
    
    

    
    ##
    # _stopRun
    #     Ensure the run state machine is not in an non halted state
    #     If the state is Active or Paused, the machine is transitioned to
    #     Halted.
    #
    method _stopRun {} {
        set sm [RunstateMachineSingleton %AUTO%]
        if {[$sm getState] in [list Active Paused]} {
            $sm transition Halted
        }
        $sm destroy
    }
    ##
    #  Stop all data sources.
    #
    method _stopDataSources {} {
        set mgr [DataSourcemanagerSingleton %AUTO%]
        set sm [RunstateMachineSingleton %AUTO%]
        
        if {[$sm getState] ne "NotReady"} {
            $mgr stopAll
        }
        $sm destroy
        $mgr destroy
    }
    
    #--------------------------------------------------------------------------
    #  Menu bar handlers:
    #
    
    ##
    # _sourceFile
    #    Sources a file at the top level.  This can be used to add plugins
    #    to the ReadoutGUI.  Since this is a menubar function;
    #    the file sourced is gottenvia tk_getOpenFile.
    #    *  .tcl - is the normal default filetype and the default extension
    #    *  .tk  - is also an option for extension filtering.
    #    *  .*   - is an option for extension filtering.
    #
    method _sourceFile {} {
        set scriptPath [tk_getOpenFile -defaultextension .tcl -parent $win  \
            -title {Choose script file}             \
            -filetypes [list                        \
                [list {Tcl Scripts} {.tcl}]         \
                [list {Tk Scripts}  {.tk"}]         \
                [list {All Files}    *]             \
            ]]
        #
        #  The dialog returns "" if cancel is clicked
        #
        if {$scriptPath ne ""} {
            uplevel #0 source $scriptPath
        }
    }
    ##
    # _extendAutoPath
    #    Adds another directory to the ::auto_path list.  The directory
    #    is prompted fro using tcl_ChooseDirectory
    #
    method _extendAutoPath {} {
        set packageDirectory [tk_chooseDirectory -mustexist 1 -parent $win \
            -title {Choose Package directory}]
        if {$packageDirectory ne ""} {
            lappend ::auto_path $packageDirectory
        }
    }
    ##
    # _chooseLogfile
    #
    #   The output widget has the capability of logging all output to a file
    #   this menu item prompts for the log file name and initiate logging.
    #   Logging will continue until the _stopLogging menu item is invoked or
    #   alternatively some external code stops logging.   If the user
    #   requests a different log file, logging will continue in that file.
    #
    method _chooseLogfile {} {
        set logFilename [tk_getSaveFile -defaultextension log -parent $win \
            -title {Choose log file}                                        \
            -filetypes [list                                                \
                [list {Log files} .log]                                     \
                [list {Text files} .txt]                                    \
                [list {All Files}   *]                                      \
        ]]
        if {$logFilename ne ""} {
            $output open $logFilename
        }
    }
    ##
    # _stopLogging
    #    Close the output window logfile if any is active.
    #    At this point this is done just by catching a call to
    #    $output close.
    #    TODO:  The menu item should be disabled when logging is off
    #           and enabled otherwise.
    #
    method _stopLogging {} {
        catch {$output close}
    }
    ##
    # _Exit
    #   Prompt for confirmation and exit.
    #   If exit is confirmed then
    #   *  If the run state is one of the active ones (Active, Paused) the run
    #      state machine is asked to stop the run.
    #   *  Data sources are asked to exit.
    #   *  The state is then transitioned to NotReady to ensure that all
    #      state transition handlers run
    #   *  The exit command is performed with a 0 (normal) exit code.
    method _Exit {} {
        set confirm [tk_messageBox -default no -type yesno -parent $win \
            -title {Really Exit?} -icon question \
            -message {Are you sure you want to exit?} \
            ]
        if {$confirm eq "yes"} {
            $self _stopRun
            $self _stopDataSources
            set sm [RunstateMachineSingleton %AUTO%]
            if {[$sm getState] ne "NotReady"} {
                $sm transition NotReady
            }
            $sm destroy;                # though there's not much point to this:
            #
            #  Save the state
            #
            
            set state [StateManagerSingleton %AUTO%]
            $state save
            $state destroy
            
            exit 0;                     # since we're exiting now.
        }
    }
}

