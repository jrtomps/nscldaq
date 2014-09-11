

package provide OfflineEVBJobBuilder 11.0

package require Tk
package require snit

package require FileListWidget
package require ConfigErrorUI 

package require OfflineEVBInputPipeline
package require OfflineEVBHoistPipeline
package require evbcallouts 
package require OfflineEVBOutputPipeline 


snit::widget JobBuilderUIView {
  
  component presenter 

  variable listWidget ""

  constructor {args} {

    set presenter ""

    # assemble the megawidget
    $self buildGUI

    # setup some special styles
    $self configureStyles
  }

  destructor {

  }

  method buildGUI {} {
    variable listWidget

    # build the 
    set top [ttk::frame $win.listFrame]
    ttk::label $top.flistLbl -text "Jobs To Process" -style "H1.TLabel"
    set listWidget [FileList $top.flistFrame]
    grid $top.flistLbl   -sticky ew 
    grid $top.flistFrame -sticky nsew
    grid rowconfigure    $top {0}   -weight 1
    grid columnconfigure $top {0} -weight 1

    # build the buttons for adding 
#    set top [ttk::frame $win.buttons]
#    ttk::button $top.run -text "Run" -command [mymethod onRun] 
#    grid $top.run -sticky e
#    grid columnconfigure $top 0 -weight 1

    # grid these things
    grid $win.listFrame  -padx 9 -pady 9 -sticky ew
#    grid $win.buttons    -padx 9 -pady 9 -sticky ew

    grid columnconfigure $win 0 -minsize 400 -weight 1
    grid rowconfigure $win 0 -weight 1
#    grid rowconfigure $win {0 1} -weight 1

  }
  
  method configureStyles {} {
    ttk::style configure H1.TLabel -font "helvetica 18"
  }

  method onRun {} {
    $presenter run
  }

  method getPresenter {} {
    return $presenter
  }

  method setPresenter {newPresenter} {
    variable presenter
    set presenter $newPresenter
  }

  method getTreeWidget {} {
    variable listWidget 
    return $listWidget 
  }

}

# -----------------------------------------------------------------------
#

snit::type JobBuilderUIPresenter {
  option -widgetname     -default ""
  
  option -inputparams    -default ""
  option -hoistparams    -default ""
  option -evbparams      -default "" 
  option -outputparams   -default ""

  variable evbInitialized 0
  variable currentFile    ""
  
  component m_view

  variable m_masterJobList

  constructor {args} {

    # allow the user to override the defaults
    $self configurelist $args

    if {$options(-widgetname) eq ""} {
      set msg    "JobBuilderUIPresenter -widgetname option is mandatory "
      append msg "but was not provided!"
      return -code error $msg 
    }

    set m_view [JobBuilderUIView $options(-widgetname)]
    $m_view setPresenter $self 

    # set up the defaults
    $self configure -inputparams [$self createDefaultInputParams]
    $self configure -hoistparams [$self createDefaultHoistParams]
    $self configure -evbparams [EVBC::AppOptions %AUTO% -restart false \
                                                        -gui false \
                                                        -destring OfflineEVBOut]   ;# This is an EVBC::AppOptions
    $self configure -outputparams [$self createDefaultOutputParams]

    set m_masterJobList [dict create]
  }

  destructor {
    destroy $m_view
  }

  method setModel {newModel} {
    set m_model $newModel
    $self updateViewDataFromModel
  }

  method setView {theview} {
    set m_view $theview
    $self updateViewDataFromModel
  }

  ## Handles when the run has been constructed
  #
  method constructJobList {} { 

    set listWidget [$m_view getTreeWidget] 
    set jobFiles [$listWidget getJobs]
    
    set masterJobList [$self buildJobList $jobFiles]

    set errors [$self validateJobOptions $masterJobList]
    if {[dict size $errors]==0} {
      set m_masterJobList $masterJobList
    } else {
      $self displayErrorGUI $errors
    }
  } 

  method getJobList {} {
    return $m_masterJobList
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

