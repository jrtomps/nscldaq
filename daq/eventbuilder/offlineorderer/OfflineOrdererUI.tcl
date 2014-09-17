#!/usr/bin/env tclsh


package provide OfflineOrdererUI 11.0

package require Tk
package require snit

package require OfflineEVBInputPipelineUI
package require OfflineEVBHoistPipelineUI
package require OfflineEVBEVBPipelineUI
package require OfflineEVBOutputPipelineUI
package require OfflineEVBMissingSourceUI

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
    grid $m_runabortButton -sticky se ;# -padx 9 -pady 9
    grid columnconfigure $win.buttons 0 -weight 1

    # grid these things
    grid $win.frame  -padx 9 -pady 9 -sticky nsew
    grid $buttons    -padx 9 -pady 9 -sticky sew

    grid columnconfigure $win 0 -minsize 400 -weight 1
    grid rowconfigure    $win 0 -minsize 200 -weight 1

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

set orderer [OfflineOrdererUIPresenter %AUTO% -widgetname .view]
grid .view -sticky nsew
grid rowconfigure . 0 -weight 1
grid columnconfigure . 0 -weight 1
