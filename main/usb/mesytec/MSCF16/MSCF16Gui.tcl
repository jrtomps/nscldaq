#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2015.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#    Jeromy Tompkins
#	   NSCL
#	   Michigan State University
#	   East Lansing, MI 48824-1321



package provide mscf16gui 1.0

package require snit
package require Tk
package require ChannelLabel
package require TransientLabel

# the original implementation used the convention of 
# starting counting at 1 rather than 0. For programming
# simplicity, the code uses the C convention of counting from
# 0. However, the user only ever cares about what we show them. 
# So we will just display the numbers as though we used a counting
# scheme starting from 1.
namespace eval MSCF16ChannelNames {
  variable chan0 Ch1
  variable chan1 Ch2
  variable chan2 Ch3
  variable chan3 Ch4
  variable chan4 Ch5
  variable chan5 Ch6
  variable chan6 Ch7
  variable chan7 Ch8
  variable chan8 Ch9
  variable chan9 Ch10
  variable chan10 Ch11
  variable chan11 Ch12
  variable chan12 Ch13
  variable chan13 Ch14
  variable chan14 Ch15
  variable chan15 Ch16
}

## @brief The view for the MSCF16Presenter
#
# This is not a fully stupid view. It not only assembles the widgets into 
# the major gui, but also sets up all of the traces for the variables it
# manages. The trace callbacks trigger methods in this snit::widget that
# figure out what channel index the callback is associated with. Only then is the event
# forwarded to the MSCF16Presenter. In the end, the MSCF16Presenter handles the
# interaction with the device, but the process described ties this view to
# a scheme in which each update of the variables in manages cause the presenter
# to get a commit trigger. You will notice that there is no "commit" button.
# That is because every widget is in essence its own commit button.
#
# It is important to understand that there is a mechanism for turning on and off
# the trace callbacks via the -committable option. The presenter of this should
# turn off the trace callbacks when updating the state because if it didn't, the
# updating of the values in this would trigger further commits. The result would
# be an infinitely repeating update cycle. 
#
# 
snit::widget MSCF16Form {

  option -committable -default 1;#!< changing widget vals schedules commit
  variable _scheduledCommitOpId -1  ;# the last op id schedule to commit

  # there are a lot of variable managed by this...
  variable monitor
  variable remote
  variable single

  variable th0
  variable th1
  variable th2
  variable th3
  variable th4
  variable th5
  variable th6
  variable th7
  variable th8
  variable th9
  variable th10
  variable th11
  variable th12
  variable th13
  variable th14
  variable th15
  variable th15
  variable th16

  variable pz0
  variable pz1
  variable pz2
  variable pz3
  variable pz4
  variable pz5
  variable pz6
  variable pz7
  variable pz8
  variable pz9
  variable pz10
  variable pz11
  variable pz12
  variable pz13
  variable pz14
  variable pz15
  variable pz16

  variable ga0
  variable ga1
  variable ga2
  variable ga3
  variable ga4

  variable sh0
  variable sh1
  variable sh2
  variable sh3
  variable sh4

  option -presenter  -default {}  ;# the presenter
  component _statusLbl ;# transient label to communicating success/fail to user

  ## @brief Constructor 
  # 
  #  Initializes the variables and builds the gui
  constructor {args} {
    $self configurelist $args

    $self InitVariables
    $self SetupGUI
    
    $self SetStateForMode
  }

  ## @brief Initialize the variables
  #
  # Sets the default value for all of the widgets... the should be overwritten
  # immediately after being connected to a presenter.
  #
  method InitVariables {} {
    set monitor 0
    set remote on
    set single common
    for {set i 0} {$i < 5} {incr i} {
      set ga$i 0
      set sh$i 0
    }

    for {set i 0} {$i < 17} {incr i} {
      set th$i 0
      set pz$i 0
    }
  }

  ## @brief Assemble the widgets 
  #
  # This is a monolithic method. It should be broken down into pieces but will
  # remain as it is in the interest of time. In general, it builds the entire
  # system. There is a table of individual channel settings grouped into 4 rows
  # each and then a bottom row with the common settings. At any given time,  the
  # either the widgets for common configuration or individual configuration are
  # enabled. Any interaction with a widget triggers callback to execute after a
  # slightly dealyed period of time. This allows for the user to hold down a
  # spinbox button and then have only the resulting value committed.
  method SetupGUI {} {

    #---------
    # build the table
    set w $win.table
    ttk::frame $w

    # build the header row
    set w $win.table.header
    ttk::frame $w -style Header.TFrame
    ttk::label $w.na -text Name -padding 4 -style Header.TLabel
    ttk::label $w.ch -text Channel -padding 4 -style Header.TLabel
    ttk::label $w.ga -text Gain -padding 4 -style Header.TLabel
    ttk::label $w.sh -text Shaping -padding 4 -style Header.TLabel
    ttk::label $w.pz -text "Pole zero" -padding 4 -style Header.TLabel
    ttk::label $w.th -text Threshold -padding 4 -style Header.TLabel
    ttk::label $w.mo -text Monitor -padding 4 -style Header.TLabel
    grid $w.na $w.ga $w.sh $w.pz $w.th $w.mo -sticky sew
    grid columnconfigure $w {0 1 2 3 4 5} -weight 1 -uniform a

    # grid rows 4 at a time
    for {set group 0} {$group < 4} {incr group} {

      # every 4 rows should be grouped...
      set w $win.table.group$group
      ttk::frame $w -style Group.TFrame

      # these are the widgets that only happen every 4 rows
      ttk::spinbox $w.ga$group -textvariable [myvar ga$group] \
                   -width 4 -from 0 -to 15 \
                    -style Group.TSpinbox
      trace add variable [myvar ga$group] write [mymethod OnGainChanged]
      ttk::spinbox $w.sh$group -textvariable [myvar sh$group] \
                   -width 4 -from 0 -to 3 \
                    -style Group.TSpinbox
      trace add variable [myvar sh$group] write [mymethod OnShapingTimeChanged]

      # construct widgets that happen every single row
      for {set subrow 0} {$subrow < 4} {incr subrow} {
        set i [expr $group*4+$subrow]
        ChannelLabel $w.na$i -width 8 -textvariable MSCF16ChannelNames::chan$i \
          -style Group.TEntry -defaultstring "Ch[expr $i+1]"
        ttk::spinbox $w.pz$i -textvariable [myvar pz$i] -width 4 \
          -from 0 -to 255 \
          -style Group.TSpinbox
        trace add variable [myvar pz$i] write [mymethod OnPoleZeroChanged]

        ttk::spinbox $w.th$i -textvariable [myvar th$i] -width 4 \
          -from 0 -to 255 \
          -style Group.TSpinbox
        trace add variable [myvar th$i] write [mymethod OnThresholdChanged]

        ttk::radiobutton $w.mo$i -variable [myvar monitor] \
          -value $i -style Group.TRadiobutton -command [mymethod OnMonitorChanged $i]

        # within this loop, we will grid... the first row should have
        # the widgets that only appear once every 4 rows...
        if {$subrow==0} {
          grid $w.na$i $w.ga$group $w.sh$group $w.pz$i $w.th$i $w.mo$i \
            -sticky news -padx 4 -pady 4
        } else {
        # the remaining 3 rows in the group
          grid $w.na$i ^ ^ $w.pz$i $w.th$i $w.mo$i \
            -sticky news -padx 4 -pady 4
        }
      }

      grid columnconfigure $w {0 1 2 3 4 5} -weight 1 -uniform a
      grid rowconfigure $w {0 1 2 3} -weight 1
    }

    # common widgets
    set w $win.table.group4
    ttk::frame $w
    ttk::checkbutton $w.si -text Common -variable [myvar single]\
                           -onvalue common -offvalue individual \
                           -command [ mymethod OnModeChanged]

    ttk::spinbox $w.gac -textvariable [myvar ga4] -width 4 \
                        -from 0 -to 15 
    trace add variable [myvar ga4] write [mymethod OnGainChanged]

    ttk::spinbox $w.shc -textvariable [myvar sh4] -width 4 \
                        -from 0 -to 15 
    trace add variable [myvar sh4] write [mymethod OnShapingTimeChanged]

    ttk::spinbox $w.pzc -textvariable [myvar pz16] -width 4 \
                        -from 0 -to 255 
    trace add variable [myvar pz16] write [mymethod OnPoleZeroChanged]

    ttk::spinbox $w.thc -textvariable [myvar th16] -width 4 \
                        -from 0 -to 255 
    trace add variable [myvar th16] write [mymethod OnThresholdChanged]

    grid $w.si  $w.gac $w.shc $w.pzc $w.thc x -sticky news
    grid columnconfigure $w {0 1 2 3 4 5} -weight 1 -uniform a

    grid $win.table.header -sticky nsew -padx 4
    grid $win.table.group0 -sticky nsew -padx 4 -pady 4
    grid $win.table.group1 -sticky nsew -padx 4 -pady 4
    grid $win.table.group2 -sticky nsew -padx 4 -pady 4
    grid $win.table.group3 -sticky nsew -padx 4 -pady 4
    grid $win.table.group4 -sticky nsew -padx 4 -pady 4
    grid columnconfigure $win.table 0 -weight 1
    grid rowconfigure $win.table {0 1 2 3 4 5} -weight 1

    # end of building the table
    #--------------------------
    
    # build remote frame
    #  - contains the status lable and remote checkbutton
    set w $win.remote
    ttk::frame $w
    ttk::checkbutton $w.remote -text Remote -variable [myvar remote] -onvalue 1 \
                     -offvalue 0 -command [mymethod OnRemoteChanged]
    set _statusLbl [TransientLabel $w.status -text {}]
    grid $_statusLbl $w.remote -sticky ew
    grid rowconfigure $w 0 -weight 1
    grid columnconfigure $w 0 -weight 1

    # grid the large chunks together
    grid $win.table -sticky nsew -padx 4 -pady 4
    grid $win.remote -sticky nsew -padx 4 -pady 4
    grid columnconfigure $win 0 -weight 1
    grid rowconfigure $win 0 -weight 1
  }

  ## @brief Enables/disables the group of widgets associated with individual
  # config
  #
  # @param state    the state to set (typically "disabled" or "!disabled")
  #
  method SetStateOfIndividualControls {state} {
    for {set grp 0} {$grp < 4} {incr grp} {

      $win.table.group$grp state $state
      $win.table.group$grp.ga$grp state $state
      $win.table.group$grp.sh$grp state $state

      for {set subgrp 0} {$subgrp<4} {incr subgrp} {
        set i [expr $grp*4+$subgrp]
        $win.table.group$grp.pz$i state $state
        $win.table.group$grp.th$i state $state
      }
    }
  }

  ## @brief Sets the state of all the monitor radiobuttons
  #
  # @param state  the state to set (typically "disabled" or "!disabled")
  method SetStateOfMonitorControls {state} {
    for {set grp 0} {$grp < 4} {incr grp} {
      for {set subgrp 0} {$subgrp<4} {incr subgrp} {
        set i [expr $grp*4+$subgrp]
        $win.table.group$grp.mo$i state $state
      }
    }
  }

  ## @brief Enables/disables the group of widgets associated with common
  # config
  #
  # @param state    the state to set (typically "disabled" or "!disabled")
  #
  method SetStateOfCommonControls {state} {
    $win.table.group4 state $state
    $win.table.group4.gac state $state
    $win.table.group4.shc state $state
    $win.table.group4.pzc state $state
    $win.table.group4.thc state $state
  }

  ## @brief Set the state of _all_ widgets!
  #
  # @param state    the state to set (typically "disabled" or "!disabled")
  method SetStateOfAllControls {state} {
    $self SetStateOfIndividualControls $state
    $self SetStateOfCommonControls $state
    $self SetStateOfMonitorControls $state
  }

  ## @brief Set the state of widgets properly for the config mode
  #
  # For example, if the "common" checkbutton indicates that common mode is 
  # selected,
  # the all of the widgets associated with individual control are disabled.
  method SetStateForMode {} {
    if {$single eq "common"} {
      $self SetStateOfIndividualControls disabled
      $self SetStateOfCommonControls !disabled
    } else {
      $self SetStateOfIndividualControls !disabled
      $self SetStateOfCommonControls disabled
    }
  }

  ## @brief Callback for when the "remote" checkbutton is pressed
  #
  # This triggers an immediate commit.
  #
  method OnRemoteChanged {} {
    if {[$self cget -presenter] ne {}} {
      [$self cget -presenter] CommitSingle EnableRC $remote
      $_statusLbl configure -text "Transitioned to remote $remote"
    }
  }

  # Setter/Getter interface
  method SetThreshold {index val} {set th$index $val }
  method GetThreshold index {return [set th$index] }

  method SetPoleZero {index val} { set pz$index $val }
  method GetPoleZero index { return [set pz$index] }

  method SetMonitor {val} { set monitor $val }
  method GetMonitor {} { return $monitor }

  method SetGain {index val} { set ga$index $val  }
  method GetGain {index} { return [set ga$index]}

  method SetShapingTime {index val} { set sh$index $val }
  method GetShapingTime {index} { return [set sh$index] }

  method SetMode {val} {
    set single $val 
  }
  method GetMode {} { return $single }

  method EnableRC {on} { set remote $on } 
  method RCEnabled {} { return $remote }

  # -- Trace callback code --
  

  ## Utility method for extracting channel name from a fully qualified varname
  #
  # This is intended to get the integer that forms that very end of the variable
  # name. For example, the index 12 will be extracted from a string 
  # "::Type1::Inst_1::ga12" if the pattern is "ga".
  #
  # @param string   the fully qualified varname
  # @param pattern  pattern that begin unqualified portion of name
  #
  # @returns index at the end of the varname
  method ExtractEndingIndex {string pattern} {
    set index [string last $pattern $string]
    set index [expr $index+[string length $pattern]]
    return [string range $string $index end]
  }


  ## @brief Trace callback for gain variables
  #
  # Triggers a delayed commit.
  #
  # @param name1  first var name of traced var
  # @param name2  second var name of traced var
  # @param op     operation triggering trace callback
  method OnGainChanged {name1 name2 op} {
    set index [$self ExtractEndingIndex $name1 ga]
    $self DelayedChanCommit Gain $index [set $name1]
  }

  ## @brief Trace callback for shaping time variables
  #
  # Triggers a delayed commit.
  #
  # @param name1  first var name of traced var
  # @param name2  second var name of traced var
  # @param op     operation triggering trace callback
  method OnShapingTimeChanged {name1 name2 op} {
    set index [$self ExtractEndingIndex $name1 sh]
    $self DelayedChanCommit ShapingTime $index [set $name1]
  }

  ## @brief Trace callback for pole zero variables
  #
  # Triggers a delayed commit.
  #
  # @param name1  first var name of traced var
  # @param name2  second var name of traced var
  # @param op     operation triggering trace callback
  method OnPoleZeroChanged {name1 name2 op} {
    set index [$self ExtractEndingIndex $name1 pz]
    $self DelayedChanCommit PoleZero $index [set $name1]
  }

  ## @brief Trace callback for threshold variables
  #
  # Triggers a delayed commit.
  #
  # @param name1  first var name of traced var
  # @param name2  second var name of traced var
  # @param op     operation triggering trace callback
  method OnThresholdChanged {name1 name2 op} {
    set index [$self ExtractEndingIndex $name1 th]
    $self DelayedChanCommit Threshold $index [set $name1]
  }

  ## @brief Trace callback for monitor variable
  #
  # Triggers a delayed commit.
  #
  # @param name1  first var name of traced var
  # @param name2  second var name of traced var
  # @param op     operation triggering trace callback
  method OnMonitorChanged {index} {
    if {[$self cget -presenter] ne {}} {
      [$self cget -presenter] CommitSingle SetMonitor $index
    }
  }
  
  ## @brief Callback for the configuration mode changes
  #
  # The "common" checkbutton determines whether the user can interact with the
  # widgets associated with common or individual configuration. When the button
  # changes, this gets triggered. It handles the logic for forwarding the event
  # to the presenter, but also updates the state of the widgets accordingly.
  #
  # @param name1  first var name of traced var
  # @param name2  second var name of traced var
  # @param op     operation triggering trace callback
  method OnModeChanged {} {

    if {[$self cget -presenter] ne {}} {
       [$self cget -presenter] CommitSingle SetMode $single
    }

  }
  
  ## @brief Utility method for scheduling a delayed commit 
  #
  # Given a param name, channel index, and value, this triggers a commit to
  # occur after 350 ms. Previously scheduled updates are cancelled if they have
  # not yet occurred. it is assumed that the user is not fast enough to navigate
  # between two different widgets and modify them both within 350 ms. That would
  # be moving very quickly.
  #
  # @param param  name of parameter to set (this should be captialized because
  #               it is used to form a "Set" command like "SetThreshold")
  # @param chan   the channel index
  # @param val    value to write
  method DelayedChanCommit {param chan val} {
    # it doesn't matter what changed... we will just schedule the commit
    if {([$self cget -committable]==1) && ([$self cget -presenter] ne {})} {
      if {$_scheduledCommitOpId != -1} {
        after cancel $_scheduledCommitOpId
        set _scheduledCommitOpId -1
      }
      set _scheduledCommitOpId [after 301 \
           [list [$self cget -presenter] CommitSingleChan Set$param $chan $val]]
    }
  }

  ## @brief Same as DelayedChanCommit but for parameters with no index
  #
  # This should be used for parameters like Monitor.
  #
  # @param param  name of parameter to set (this should be captialized because
  #               it is used to form a "Set" command like "SetThreshold")
  # @param val    value to write
  method DelayedCommit {param val} {
    # it doesn't matter what changed... we will just schedule the commit
    if {([$self cget -committable]==1) && ([$self cget -presenter] ne {})} {
      if {$_scheduledCommitOpId != -1} {
        after cancel $_scheduledCommitOpId
        set _scheduledCommitOpId -1
      }
      set _scheduledCommitOpId [after 301 \
        [list [$self cget -presenter] CommitSingle Set$param $val]]

    }
  }

  ## @brief Set the status message 
  #
  # This will form transient message. It is useful for indicating success or
  # failure of previous commits. There is no manipulation of the string to
  # ensure a proper length. The caller should make sure that they don't write a
  # long message, because it may not all be visible if too long.
  #
  # @param  message
  method SetStatus {message} {
    $_statusLbl configure -text $message
  }
}

# --------------------------------------------------------------------------- #

## @brief Presenter for the MSCF16Form
#
# This forms the direct communication mechanism for the device. The update logic
# and the commit logic are in this snit::type, however, the MSCF16Form has
# already determined that a commit should occur by the time this is interacted
# with. 
#
# By nature, this does not _demand_ a view and handle to be contructed, but it
# will fail when trying to do just about anything. For that reason, the user
# should either set the -view and -handle options at construction or before
# trying to do anything.
#
snit::type MSCF16Presenter {

  option -view -default {} -configuremethod SetView
  option -handle -default {} -configuremethod SetHandle

  option -autoupdate -default 1

  ## @brief Construct and parse options
  #
  # @param args   list of option-value pairs
  constructor {args} {
    $self configurelist $args
  }

  ## @brief Commit all view state to device
  #
  # At the moment, this is not utilized because it is way too slow. Instead
  # the CommitSingle and CommitSingleChan method are used. If for some reason,
  # the system moves to a "commit" button approach where the gui and the device
  # are coupled more loosely, then this would be the method to use.
  method Commit {} {
    set view [$self cget -view]
    if {$view ne {}} {
      $view SetStatus "Communicating with device"
      $view SetStateOfAllControls disabled
      update

      $self CommitViewToModel
      $self UpdateViewFromModel

      $view SetStateOfMonitorControls !disabled
      $view SetStateForMode 
      $view SetStatus "Successfully updated"
    }
  }

  ## @brief Commit a single parameter that is indexed
  #
  # If there is no handle register to this, the method does nothing!
  #
  # When the handle is present, the single parameter value is committed and then
  # the entire view is updated.
  #
  # @param param    name of parameter (used to form a Set command)
  # @param index    index of parameter
  # @param val      value to write
  method CommitSingleChan {param index val} {
    set handle [$self cget -handle]
    set view [$self cget -view]
    if {($handle ne {}) && ($view ne {})} {
        $view SetStatus "Communicating with device"
        $view SetStateOfAllControls disabled
        update
        
        if {[catch {$handle $param $index $val} msg]} {
          tk_messageBox -icon error -message $msg
        }
        $self UpdateViewFromModel

        $view SetStateOfMonitorControls !disabled
        $view SetStateForMode 
        $view SetStatus "Successfully updated $param $index"
    }
  }

  ## @brief Commit a single parameter that has no index
  #
  # If there is no handle register to this, the method does nothing!
  #
  # When the handle is present, the single parameter value is committed and then
  # the entire view is updated.
  #
  # @param param    name of parameter (used to form a Set command)
  # @param val      value to write
  method CommitSingle {param val} {
    set handle [$self cget -handle]
    set view [$self cget -view]
    if {($handle ne {}) && ($view ne {})} {

      $view SetStatus "Communicating with device"
      $view SetStateOfAllControls disabled
      update

      if {[catch {$handle $param $val} msg]} {
        tk_messageBox -icon error -message $msg
      }
      $self UpdateViewFromModel

      $view SetStateOfMonitorControls !disabled
      $view SetStateForMode 
      $view SetStatus "Successfully updated $param"
    }
  }

  ## @brief Update the entire view with the state of the handle
  #
  # This gets calls for every commit regardless. The important part of the logic
  # is that it turns off the trace calbbacks in the view so that no subsequent
  # commits are scheduled while the update process is underway. 
  method UpdateViewFromModel {} {
    set view [$self cget -view]
    set handle [$self cget -handle]

    if {($view ne {}) && ($handle ne {})} {

      # make sure the following manipulation of variables does not 
      # schedule a subsequent commit. If it did, this would cause an infinite
      # loop... not a good thing
      $view configure -committable 0

      set val [$handle RCEnabled]
      if {$val ne {NA}} {
          $view EnableRC $val
      }

      $self SyncParam Mode $handle $view
      $self SyncParam Monitor $handle $view

      for {set ch 0} {$ch < 17} {incr ch} {
        $self SyncIndexedParam Threshold $ch $handle $view
        $self SyncIndexedParam PoleZero $ch $handle $view
      }
      for {set grp 0} {$grp < 5} {incr grp} {
        $self SyncIndexedParam Gain $grp $handle $view
        $self SyncIndexedParam ShapingTime $grp $handle $view
      }

      # now we are done updating. If the user wants to manipulate the widgets,
      # then it will schedule a later commit.
      $view configure -committable 1
    }
  }

  ## @brief Utility for passing a value from one component to another
  #
  # Because the handle and the view both implement mirror Get/Set type 
  # interfaces, transferring a particular parameter from one to the other is the
  # same in both directions. The only difference is which is used for the Get
  # and which for the Set. 
  #
  # This is also a common location for all of the transfer operations. It checks
  # for values NA and if found, it doesn't transfer. The NA value is special b/c
  # it is the default value for the MSCF16Memorizer snit::type that is used to
  # transfer state into the GUI.
  #
  # @param name   parameter name
  # @param from   object from which to Get
  # @param to     object to Set
  method SyncParam {name from to} {
    set val [$from Get$name]
    if {$val ne "NA"} {
      $to Set$name $val
    }
  }

  ## @brief Utility for passing an indexed value between handle and view
  #
  # This is the same as SyncParam except it is to be used for Setters and
  # Getters that have an associated index.
  #
  # @param name   parameter name
  # @param from   object from which to Get
  # @param to     object to Set
  method SyncIndexedParam {name index from to} {
    set val [$from Get$name $index]
    if {$val ne "NA"} {
      $to Set$name $index $val
    }
  }

  ## @brief Commit entire view state to the handle
  #
  # Not utilized at the moment. However, it is the logic of the Commit method.
  #
  method CommitViewToModel {} {
    set view [$self cget -view]
    set handle [$self cget -handle]

    if {($view ne {}) && ($handle ne {})} {
      set val [$view RCEnabled]
      if {$val ne "NA"} { $handle EnableRC $val }

      $self SyncParam Mode $view $handle
      $self SyncParam Monitor $view $handle

      for {set ch 0} {$ch < 17} {incr ch} {
        $self SyncIndexedParam Threshold $ch $view $handle
        $self SyncIndexedParam PoleZero $ch $view $handle
      }

      for {set grp 0} {$grp < 5} {incr grp} {
        $self SyncIndexedParam Gain $grp $view $handle
        $self SyncIndexedParam ShapingTime $grp $view $handle
      }
    }
  }

  
  ## @brief Callback for associating a view with this
  #
  # Handles the intitial handshake between the view and presenter.
  # If a handle exists, this also will trigger an update of the view.
  #
  # @param opt  option name (-view)
  # @param val  value to set 
  method SetView {opt val} {
    set options($opt) $val
    $val configure -presenter $self

    if {([$self cget -handle] ne {}) && [$self cget -autoupdate]} {
      $self UpdateViewFromModel
      $val SetStateForMode
    }
  }

  ## @brief Callback for associating a handle with this
  #
  # Handles the intitial handshake between the view and presenter. If a view is
  # present as well, then the view is updated from the newly registered handle.
  #
  # @param opt  option name (-handle)
  # @param val  value to set 
  method SetHandle {opt val} {
    set options($opt) $val

    if {([$self cget -view] ne {}) && [$self cget -autoupdate]} {
      $self UpdateViewFromModel
      [$self cget -view] SetStateForMode
    }
  }

}
