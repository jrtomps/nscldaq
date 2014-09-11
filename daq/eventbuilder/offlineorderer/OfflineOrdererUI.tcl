#!/usr/bin/env tclsh


package provide OfflineOrdererUI 11.0

package require Tk
package require snit

package require OfflineEVBInputPipelineUI
package require OfflineEVBHoistPipelineUI
package require OfflineEVBEVBPipelineUI
package require OfflineEVBOutputPipelineUI

package require ApplyCancelWidget 

package require OfflineEVBRunProcessor
package require OfflineEVBJobBuilder
package require OfflineEVBRunStatusUI

proc TabbedOutput {win args} {
}




#########################################################################################

snit::widget OfflineOrdererUIView {
  
  option -mode     -default "config" -configuremethod setMode

  component presenter 

  variable m_viewMap
  variable m_runabortButton

  constructor {args} {

    $self configurelist $args

    set m_viewMap [dict create config "" run ""]

    # assemble the megawidget
    $self buildGUI

  }

  destructor {
  }

  method buildGUI {} {

    ttk::frame $win.frame

    set buttons $win.buttons
    set m_runabortButton $buttons.runabort
    ttk::frame $buttons 
    ttk::button $m_runabortButton -text "Process Jobs" -command [mymethod onPress]
    grid $m_runabortButton -sticky se -padx 9 -pady 9
    grid columnconfigure $win.buttons 0 -weight 1

    # grid these things
    grid $win.frame  -padx 9 -pady 9 -sticky nsew
    grid $buttons    -padx 9 -pady 9 -sticky sew

    grid columnconfigure $win 0 -minsize 400 -weight 1
    grid rowconfigure $win 0 -minsize 200 -weight 1

  }
  
  method onPress {} {
    $presenter transition 
  }

  method getPresenter {} {
    return $presenter
  }

  method setPresenter {newPresenter} {
    variable presenter
    set presenter $newPresenter
  }

  method getCurrentWidget {} {
    return [grid slaves $win.frame] 
  }

  method setViewWidgets {widgetDict} {
    variable m_viewMap
    set m_viewMap $widgetDict
  }

  method setMode {option mode} {
    puts "setMode $option $mode"
    variable m_runabortButton
    variable m_viewMap

    set children [grid slaves $win.frame]
    foreach child $children { grid forget $child }

    if {$mode eq "config"} {

      $m_runabortButton configure -text "Process Jobs" 
      set widget [dict get $m_viewMap config]
      puts "found \"$widget\""
      if {$widget ne ""} {
        grid $widget -sticky nsew
        puts "gridded"
        puts [grid slaves $win.frame] 
      }

      set options(-mode) $mode
    } elseif {$mode eq "run"} {
      $m_runabortButton configure -text "Abort" 

      set widget [dict get $m_viewMap run]
      puts "found \"$widget\""
      if {$widget ne ""} {
        grid $widget -sticky nsew
        puts "gridded"
        puts [grid slaves $win.frame] 
      }

      set options(-mode) $mode
    } else {
      set msg    "OfflineOrdererUIView::setMode passed \"$mode\" as "
      append msg "argument instead of \"config\" or \"run\"."
      return -code error $msg 
    }
  }


  method getFrameWidget {} {
    return $win.frame
  }

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
    set jobBuilderPresenter [JobBuilderUIPresenter %AUTO% -widgetname $fr.configUI]
    $m_view setViewWidgets [dict create run $fr.runUI \
                                        config $fr.configUI ]

    # create the run processor
    set runProcessor [RunProcessor %AUTO%]


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
      $self run
      $m_view configure -mode run 
    } else {
      $m_view configure -mode config
    }
  }

  ## Handles when the run has been constructed
  #
  method run {} { 
    variable jobBuilderPresenter
    variable runProcessor 

    $jobBuilderPresenter constructJobList
    set masterJobList [$jobBuilderPresenter getJobList]
    
    if {[dict size $masterJobList]>0} {
      $runProcessor configure -jobs $masterJobList
      set status [$runProcessor run]

      # display the running progress view
      $m_view configure -mode run
    } 

  } 

} ;# end of OfflineOrderer

# ----------------------------------------------------------------------

option add *tearOff 0

menu .menu
. configure -menu .menu

set m .menu
menu $m.config
$m add command -label "Configure..." -command {launchConfigDialogue} 

#$m add cascade -menu $m.config -label "Configure"
#$m.config add command -label "Input Pipeline..."  -command {launchInputConfigDialogue}
#$m.config add command -label "Event Builder..."   -command {launchEVBDialogue}
#$m.config add command -label "Event Recording..." -command {launchEventRecordingDialoguer}



## 
#
proc launchConfigDialogue {} {
  toplevel .config
#  wm geometry .config 600x400-5+40
  global .view
  set presenter [$::orderer getJobBuilderPresenter] 
  set dialogue [ApplyCancelWidgetPresenter %AUTO% -widgetname .config.dia -ismaster 1]
  
  set inputPresenter [InputPipeConfigUIPresenter %AUTO% -widgetname .config.dia.in -ismaster 0]
  $inputPresenter setModel [$presenter cget -inputparams]
  set hoistPresenter [HoistPipeConfigUIPresenter %AUTO% -widgetname .config.dia.hoist -ismaster 0]
  $hoistPresenter setModel [$presenter cget -hoistparams]
  set evbPresenter [EVBPipeConfigUIPresenter %AUTO% -widgetname .config.dia.evb -ismaster 0]
  $evbPresenter setModel [$presenter cget -evbparams]
  set outPresenter [OutputPipeConfigUIPresenter %AUTO% -widgetname .config.dia.out -ismaster 0]
  $outPresenter setModel [$presenter cget -outputparams]

  set presenters [dict create "Input Pipeline"  $inputPresenter\
                          "Hoist Pipeline" $hoistPresenter \
                          "EVB Pipeline" $evbPresenter \
                          "Output Pipeline" $outPresenter ]
  $dialogue setPresenterMap $presenters
  grid .config.dia -sticky nsew
  grid rowconfigure .config 0 -weight 1
  grid columnconfigure .config 0 -weight 1

}

set orderer [OfflineOrdererUIPresenter %AUTO% -widgetname .view]
grid .view -sticky nsew
grid rowconfigure . 0 -weight 1
grid columnconfigure . 0 -weight 1
