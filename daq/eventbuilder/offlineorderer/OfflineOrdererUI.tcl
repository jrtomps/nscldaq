#!/usr/bin/env tclsh


package provide OfflineOrdererUI 11.0

package require Tk
package require snit

package require OfflineEVBRunProcessor
package require OfflineEVBJobBuilder
package require OfflineEVBRunStatusUI
package require OfflineEVBGlobalConfigUI
package require FrameSequencer

proc TabbedOutput {win args} {
}




#########################################################################################

## @brief View for the OfflineOrderer widget
#
# This is really a very simple widget. It merely maintains a content window
# and a button that causes a transition between two states. The two states 
# are the configuration state and the processing state. The configuration state 
# is where the JobBuilderUI is displayed and the user can configure his/her 
# job parameters. During the run, the RunStatusUI is displayed so that the 
# user can see that something is happening. The RunStatusUI is not perfect
# because all of this is sequential at the moment besides the various pieces that
# are launched as pipelines. 
#
# The implementation of this is intended to be fairly empty and void of logic.
# It merely knows the widgets and how to display them. All events are directed
# to the presenter.
snit::widget OfflineOrdererUIView {
  
  option -mode     -default "config" -configuremethod setMode

  component presenter 

  variable m_viewMap
  variable m_runabortButton

  constructor {args} {

    $self configurelist $args

    # There are two views that can be shown in the content window
    set m_viewMap [dict create config "" run ""]

    # assemble the megawidget
    $self buildGUI

  }

  # we don't own the presenter and the widgets will be destoyed 
  # automatically when this frame is destroyed.
  destructor {
  }

  ## @brief Build the gui
  #
  method buildGUI {} {

    ttk::frame $win.frame

    set buttons $win.buttons
    set m_runabortButton $buttons.runabort
    ttk::frame $buttons 
    ttk::button $m_runabortButton -text "Process Jobs" -command [mymethod onPress]
    grid $m_runabortButton -sticky se ;# -padx 9 -pady 9
    grid columnconfigure $win.buttons 0 -weight 1

    # grid these things
    grid $win.frame  -padx 9 -pady 9 -sticky nsew
    grid $buttons    -padx 9 -pady 9 -sticky sew

    grid columnconfigure $win 0 -minsize 400 -weight 1
    grid rowconfigure    $win 0 -minsize 200 -weight 1

  }
  
  ## @brief The button was pressed.
  #
  method onPress {} {
    $presenter transition 
  }

  ## @brief Get the presenter that this responds to
  #
  # @returns the presenter
  method getPresenter {} {
    return $presenter
  }

  ## @brief Pass the view a new presenter
  #
  # @param newPresenter   a OfflineOrdererUIPresenter object
  #
  method setPresenter {newPresenter} {
    set presenter $newPresenter
  }

  ## @brief Retrieve which widget is currently gridded
  #
  # @returns name of gridded widget
  #
  method getCurrentWidget {} {
    return [grid slaves $win.frame] 
  }

  ## @brief  Set the dict of the view widgets
  # 
  # Currently there are only two different modes that are 
  # supported. The idea is that each mode has a corresponding
  # GUI to display. The dict has keys that are the mode names
  # and the associated widgets are the values.
  #
  # @param widgetDict   a dict of of {mode0 widget0 mode1 widget1}
  method setViewWidgets {widgetDict} {
    set m_viewMap $widgetDict
  }

  ## @brief Callback for configure -mode
  #
  # This is the configuremethod for the -mode option. The
  # main idea of this is to synchronize the displayed
  # widget with the mode.
  # 
  # @param option name of the option (in this case -mode)
  # @param mode   the new mode to set
  #
  method setMode {option mode} {
    variable m_runabortButton
    variable m_viewMap

    set children [grid slaves $win.frame]
    foreach child $children { grid forget $child }

    if {$mode eq "config"} {

      $m_runabortButton configure -text "Process Jobs" 
      set widget [dict get $m_viewMap config]
      if {$widget ne ""} {
        grid $widget -sticky nsew
        grid columnconfigure [$self getFrameWidget] 0 -weight 1
        grid rowconfigure    [$self getFrameWidget] 0 -weight 1
      }

      set options(-mode) $mode
    } elseif {$mode eq "run"} {
      $m_runabortButton configure -text "Back" 

      set widget [dict get $m_viewMap run]
      if {$widget ne ""} {
        grid $widget -sticky nsew
      }

      set options(-mode) $mode
    } else {
      set msg    "OfflineOrdererUIView::setMode passed \"$mode\" as "
      append msg "argument instead of \"config\" or \"run\"."
      return -code error $msg 
    }
  }


  ## @brief Getter for the content frame widget name
  #
  # @returns name of content frame
  method getFrameWidget {} {
    return $win.frame
  }

  ## @brief Retrieve the name of this megawidget
  #
  # @returns name of megawidget
  method getWindowName {} {
    return $win
  }
}



# -----------------------------------------------------------------

snit::type OfflineOrdererUIPresenter {
  option -widgetname     -default ""
  
  component m_view

  component runProgressPresenter
  component jobBuilderPresenter
  component globalConfigPresenter

  component runProcessor 

  constructor {args} {

    # allow the user to override the defaults
    $self configurelist $args

    if {$options(-widgetname) eq ""} {
      set msg    "OfflineOrdererUI -widgetname option is mandatory "
      append msg "but was not provided!"
      return -code error $msg 
    }

    # setup the view
    set m_view [OfflineOrdererUIView $options(-widgetname)]
    $m_view setPresenter $self

    # setup the stuff the running and configuration frames
    set fr [$m_view getFrameWidget]
    set runProgressPresenter [RunStatusUIPresenter %AUTO% -widgetname $fr.runUI]

    # this uses the singleton adapater for the JobBuilder
    set JobBuilder::widgetName $fr.configUI
    set jobBuilderPresenter [JobBuilder::getInstance] 

    $m_view setViewWidgets [dict create run $fr.runUI \
                                        config $fr.configUI ]

    # create the run processor
    set runProcessor [RunProcessor %AUTO%]

    $runProcessor addRunStatusObserver $runProgressPresenter


    # set the display mode
    $m_view configure -mode config

  }

  destructor {
    catch {$m_view destroy}
    catch {$runProcessor destroy}
  }


  method getJobBuilderPresenter {} {
    variable jobBuilderPresenter
    return $jobBuilderPresenter
  }

  method setView {theview} {
    set m_view $theview
  }


  method transition {} {
    set state [$m_view cget -mode]

    if {$state eq "config"} {
      $m_view configure -mode run 
      $self run
    } else {
      $m_view configure -mode config
    }
  }

  ## Handles when the run has been constructed
  #
  method run {} { 
    variable jobBuilderPresenter
    variable runProcessor 

    set masterJobList [$jobBuilderPresenter getJobsList]
    if {[dict size $masterJobList]>0} {
      puts $masterJobList
      $runProcessor configure -jobs $masterJobList
      set status [$runProcessor run]

      # display the running progress view
      $m_view configure -mode run
    } else {
      $m_view configure -mode config 
    }

  } 

} ;# end of OfflineOrderer

# ----------------------------------------------------------------------

option add *tearOff 0

menu .m 
#.m add command -label "Config" -command { .seq select config ; .m delete 0 }
. configure -menu .m

wm title . "NSCLDAQ Offline Event Builder"
wm resizable . false false


FrameSequencer .seq

set GlobalConfig::win .glblConfig
set GlobalConfig::theInstance [GlobalConfigUIPresenter %AUTO% -widgetname .globalConfig]

set orderer [OfflineOrdererUIPresenter %AUTO% -widgetname .view]

.seq add main   .view { .m add command -label "Config" -command { .seq select config ; .m delete 0} }
.seq add config .globalConfig
.seq select main

grid .seq -sticky nsew
grid rowconfigure . 0 -weight 1
grid columnconfigure . 0 -weight 1


