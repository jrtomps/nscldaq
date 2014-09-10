#!/usr/bin/env tclsh


package provide OfflineOrdererUI 11.0

package require Tk
package require snit
package require FileListWidget

package require OfflineEVBInputPipelineUI
package require OfflineEVBHoistPipelineUI
package require OfflineEVBEVBPipelineUI
package require OfflineEVBOutputPipelineUI
package require ApplyCancelWidget 
package require ConfigErrorUI 

package require OfflineEVBInputPipeline
package require OfflineEVBHoistPipeline
package require evbcallouts 
package require OfflineEVBOutputPipeline 

package require OfflineEVBRunProcessor

proc TabbedOutput {win args} {
}


snit::widget OfflineOrdererView {
  
  component presenter 

  variable listWidget ""

  constructor {args} {

    set presenter [OfflineOrderer %AUTO%]
    $presenter setView $self
    $self buildGUI
  }

  destructor {

  }

  method buildGUI {} {
    variable listWidget

    # build the 
    set top [ttk::frame $win.listFrame]
    ttk::label $top.flistLbl -text "Files to Order"
    set listWidget [FileList $top.flistFrame]
    grid $top.flistLbl   -sticky ew 
    grid $top.flistFrame -sticky nsew
    grid rowconfigure    $top {0}   -weight 1
    grid columnconfigure $top {0} -weight 1

    # build the buttons for adding 
    set top [ttk::frame $win.buttons]
    ttk::button $top.run -text "Run" -command [mymethod onRun] 
    grid $top.run -sticky e
    grid columnconfigure $top 0 -weight 1

    grid $win.listFrame  -padx 9 -pady 9 -sticky ew
    grid $win.buttons    -padx 9 -pady 9 -sticky ew

    grid columnconfigure $win 0 -minsize 400 -weight 1
    grid rowconfigure $win {0 1} -weight 1

  }
  

  method onRun {} {
    $presenter run
  }

  method getPresenter {} {
    return $presenter
  }

  method getTreeWidget {} {
    variable listWidget 
    return $listWidget 
  }

}





#########################################################################################


#snit::widget ConfigurationDialogue {




# -----------------------------------------------------------------

snit::type OfflineOrderer {
  
  option -inputparams    -default ""
  option -hoistparams    -default ""
  option -evbparams      -default "" 
  option -outputparams   -default ""

  variable evbInitialized 0
  variable currentFile    ""
  
  component view
  component runProcessor 

  constructor {args} {
    # set up the defaults
    $self configure -inputparams [$self createDefaultInputParams]
    $self configure -hoistparams [$self createDefaultHoistParams]
    $self configure -evbparams [EVBC::AppOptions %AUTO% -restart false \
                                                        -gui false \
                                                        -destring OfflineEVBOut]   ;# This is an EVBC::AppOptions
    $self configure -outputparams [$self createDefaultOutputParams]

    # create the run processor
    set runProcessor [RunProcessor %AUTO%]

    # allow the user to override the defaults
    $self configurelist $args
  }

  destructor {
    destroy $view
    $runProcessor $view
  }

  method setView {theview} {
    set view $theview
  }


  ## Handles when the run has been constructed
  #
  method run {} { 

    set listWidget [$view getTreeWidget] 
    set jobFiles [$listWidget getJobs]
    
    set masterJobList [$self buildJobList $jobFiles]

    set errors [$self validateJobOptions $masterJobList]
    if {[dict size $errors]==0} {
      $runProcessor configure -jobs $masterJobList
      $runProcessor run
    } else {
      $self displayErrorGUI $errors
    }

  } 

  ##
  #
  # @param jobs a dict of key to file list
  #
  method buildJobList jobFiles {
    set masterJobList [dict create]

    foreach job [dict keys $jobFiles] {
      set files [dict get $jobFiles $job]
      $options(-inputparams) configure -file $files
      dict append masterJobList $job [dict create  -inputparams [list $options(-inputparams)] \
                                      -hoistparams [list $options(-hoistparams)] \
                                      -evbparams   [list $options(-evbparams)]   \
                                      -outputparams [list $options(-outputparams)]]

    }

    return $masterJobList
  }

  method displayErrorGUI {errors} {
    toplevel .configerr
    set msg "Some improper configuration parameters we detected!\n"
    append msg "Please fix the errors categorized below in the tree."
    ttk::label .configerr.msg -text $msg

    set pres [ConfigErrorPresenter %AUTO% -widgetname .configerr.form]
    $pres setModel $errors 

    grid .configerr.msg -sticky nsew -padx 9 -pady 9
    grid .configerr.form -sticky nsew -padx 9 -pady 9
    grid rowconfigure .configerr 0 -weight 1
    grid columnconfigure .configerr 0 -weight 1
    wm title .configerr "Configuration Errors"
  }

  method createDefaultInputParams {} {
    return [OfflineEVBInputPipeParams %AUTO%]
  }

  method createDefaultHoistParams {} {
    return [OfflineEVBHoistPipeParams %AUTO%]
  }

  method createDefaultEVBParams {} {
    return [EVBC::AppOptions %AUTO%]
  }

  method createDefaultOutputParams {} {
    return [OfflineEVBOutputPipeParams %AUTO%]
  }


  method validateJobOptions joblist {

    set errDict [dict create]

    dict for {job config} $joblist {
      set jobErrDict   [dict create]
      set inputErrors  [[dict get $config -inputparams] validate]
      set hoistErrors  [[dict get $config -hoistparams] validate]
#      set evbErrors    [[dict get $config -evbparams] validate]
      set outputErrors [[dict get $config -outputparams] validate]

      if {[llength $inputErrors]!=0} {
         dict set jobErrDict -inputparams $inputErrors
      }
      if {[llength $hoistErrors]!=0} {
         dict set jobErrDict -hoistparams $hoistErrors
      }
#      if {[llength $evbErrors]!=0} {
#         dict set jobErrDict -evbparams $evbErrors
#      }
      if {[llength $outputErrors]!=0} {
         dict set jobErrDict -outputparams $outputErrors
      }
      if {[dict size $jobErrDict]>0} {
        dict set errDict $job $jobErrDict
      }
    }

    return $errDict

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
  set presenter [.view getPresenter]
  set dialogue [ApplyCancelWidgetPresenter %AUTO% -widgetname .config.dia]
  
  set inputPresenter [InputPipeConfigUIPresenter %AUTO% -widgetname .config.dia.in]
  $inputPresenter setModel [$presenter cget -inputparams]
  set hoistPresenter [HoistPipeConfigUIPresenter %AUTO% -widgetname .config.dia.hoist]
  $hoistPresenter setModel [$presenter cget -hoistparams]
  set evbPresenter [EVBPipeConfigUIPresenter %AUTO% -widgetname .config.dia.evb]
  $evbPresenter setModel [$presenter cget -evbparams]
  set outPresenter [OutputPipeConfigUIPresenter %AUTO% -widgetname .config.dia.out]
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

OfflineOrdererView .view 
grid .view -sticky nsew
grid rowconfigure . 0 -weight 1
grid columnconfigure . 0 -weight 1
