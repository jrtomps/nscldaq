

package provide OfflineEVBJobBuilder 11.0

package require Tk
package require snit

package require ConfigErrorUI 
package require OfflineEVBJobConfigUI

package require OfflineEVBInputPipeline
package require OfflineEVBHoistPipeline
package require evbcallouts 
package require OfflineEVBOutputPipeline 
package require OfflineEVBGlobalConfigUI
package require OfflineEVBJob


snit::widget JobBuilderUIView {
  
  component m_presenter 

  variable listWidget ""

  constructor {args} {

    set m_presenter ""

    # assemble the megawidget
    $self buildGUI

    # setup some special styles
    $self configureStyles
  }

  destructor {

  }

  method buildGUI {} {

    # build the 
    set top [ttk::frame $win.listFrame]
    ttk::label $top.flistLbl -text "Jobs To Process" -style "H1.TLabel"

    # build the tree widget
    set listWidget $top.flistFrame
    ttk::frame $listWidget
    ttk::treeview $listWidget.list -show tree 
    $listWidget.list column #0 -stretch on -minwidth 10
    ttk::scrollbar $listWidget.xscroll -orient horizontal -command "$listWidget.list xview"
    $listWidget.list configure -xscrollcommand "$listWidget.xscroll set"

    ttk::button   $listWidget.addrun -text "Add Job"         -command [mymethod onAddJob]
    ttk::button   $listWidget.edit   -text "Edit Selected"   -command [mymethod onEditJob]
    ttk::button   $listWidget.rem    -text "Remove Selected" -command [mymethod onRemoveJob]

    grid $listWidget.list            -           -              -sticky nsew
    grid $listWidget.addrun $listWidget.edit $listWidget.rem    -sticky ew
    grid rowconfigure $listWidget    {0 1} -weight 1
    grid columnconfigure $listWidget {0 1 2} -weight 1 -minsize 100


    # grid the frame containing the widget
    grid $top.flistLbl   -sticky ew 
    grid $top.flistFrame -sticky nsew
    grid rowconfigure    $top {0} -weight 1
    grid columnconfigure $top {0} -weight 1

    # grid these things
    grid $win.listFrame -sticky ew ;# -padx 9 -pady 9 

    grid columnconfigure $win 0 -minsize 400 -weight 1
    grid rowconfigure    $win 0 -weight 1

  }
  
  ##
  #
  method configureStyles {} {
    ttk::style configure H1.TLabel -font "helvetica 18"
  }

  method onAddJob {} {
    $m_presenter addJob
  }

  method onEditJob {} {
    $m_presenter editJob 
  }

  method onRemoveJob {} {
    $m_presenter removeJob 
  }

  ##
  #
  method getPresenter {} {
    return $m_presenter
  }

  ##
  #
  method setPresenter {newPresenter} {
    set m_presenter $newPresenter
  }

  ##
  #
  method getTreeWidget {} {
    return $listWidget.list 
  }

  ##
  #
  method appendEntry {name} {
    [$self getTreeWidget] insert {} end -id $name -text $name
  }

}

# -----------------------------------------------------------------------
#

snit::type JobBuilderUIPresenter {
  option -widgetname     -default ""
  
  variable currentFile    ""
  
  component m_view

  variable m_masterJobList
  variable m_useParamsToCreate 
  variable m_nJobs

  ## Constructor
  #
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

    set m_masterJobList [dict create]

    set m_useParamsToCreate 0
    set m_nJobs 0
  }

  ## Destructor
  #
  destructor {
    catch { destroy $m_view}
  }

  ## Set the model
  #
  method setModel {newModel} {
    set m_model $newModel
    $self updateViewDataFromModel
  }

  ## Set the view that this controls
  #
  method setView {theview} {
    set m_view $theview
    $self updateViewDataFromModel
  }

  ## Retrieive the list of jobs
  #
  method getJobsList {} {
    
    return $m_masterJobList
  }

  ## Launches a configuration dialogue for the job 
  #
  method addJob {} {

    set m_useParamsToCreate 0

    set size [dict size $m_masterJobList]

    set model [dict create]

    if {$size!=0} {
      set job [lindex [dict keys $m_masterJobList] end]

      set model [Job::clone [dict get $m_masterJobList $job]]
      [dict get $model -inputparams] configure -file ""

    } else {
      set globals [GlobalConfig::getInstance]
      set model [Job::clone [$globals getModel]]
    }

    $self promptEdit $model "Create"

    # if the user pressed "Create" then the dialogue
    # called acceptCreation before reaching hear. This 
    # set the m_useParamsToCreate variable
    if {$m_useParamsToCreate} {

      set newName [$self constructNewJobName $model]
      $self appendNewJob $newName $model

      incr m_nJobs

    } else {
      dict for {key val} $model { 
        if {$key ne "-jobname"} {
          $val destroy 
        }
      }
    }


  }

  ##
  #
  #
  method createNewParameters {} {

    set iparams [OfflineEVBInputPipeParams %AUTO%]
    set hparams [OfflineEVBHoistPipeParams %AUTO%]
    set eparams [EVBC::AppOptions %AUTO%]
    set oparams [OfflineEVBOutputPipeParams %AUTO%]

    # Create the job parameters  to pass to the configuration
    # dialogue. It becomes the model that the dialogue's presenter
    # manipulates.
    set model [dict create  -jobname "Job" \
                            -inputparams $iparams  \
                            -hoistparams $hparams \
                            -evbparams   $eparams \
                            -outputparams $oparams ]
    $self configureDefaults $model

    return $model
  }


  ## @brief Simple method for creating a new unique job name
  #
  # Currently this is implemented trivially. However, the model could
  # be used to generate a job name in the future.
  #
  # @param model  the actual job parameterization 
  method constructNewJobName {model} {
    return "job$m_nJobs"
  }

  ## @brief Remove the selected job
  #
  method removeJob {} {
    set tree [$m_view getTreeWidget]

    set entries [$tree selection]
    if {[llength $entries] == 0 } {
      tk_messageBox -icon error -message "User must select files to remove from list"
    } else {
      # remove from the job list

      set m_masterJobList [dict remove $m_masterJobList $entries]

      # remove from the display
      $tree delete $entries

    } ;# end removal 


  }


  ## @brief Launch a dialogue to edit the selected job
  #
  method editJob {} {
    set m_useParamsToCreate 0

    # get the selection
    set tree [$m_view getTreeWidget]
    set selection [$tree selection]

    # check to see if the user selected anything.
    if {[llength $selection]==0} {
      tk_messageBox -icon warning -message "The user must select a job to edit."
      return 
    }

    $self promptEdit [dict get $m_masterJobList [lindex $selection 0]] "Accept"
  }



  ##
  #
  method promptEdit {job {buttontext "Accept"}} {
    # Create a new toplevel dialogue
    toplevel .jobconf
    wm title .jobconf "Configure Job"
    wm resizable .jobconf false false

    set config [JobConfigUIPresenter %AUTO% -widgetname .jobconf.ui \
                                            -ismaster 1]
    # becuase the user desires to edit an already created job, the
    # button should say "accept"
    $config setButtonText $buttontext

    # Figure out which job the user wants to configure
    $config setModel $job 
    $config setObserver $self
    grid .jobconf.ui -sticky nsew -padx 9 -pady 9
    grid rowconfigure    .jobconf.ui 0 -weight 1
    grid columnconfigure .jobconf.ui 0 -weight 1

    # grab this so that it is a modal dialogue
    grab .jobconf
    wm transient .jobconf
    wm protocol .jobconf WM_DELETE_WINDOW {
      grab release .jobconf;
      destroy .jobconf
    }

    # wait until the window is closed
    tkwait window .jobconf

    # There is nothing to do once the dialogue is closed 
    # because the dialogue's presenter was manipulating the 
    # job parameterization directly.

  }
  ##
  #
  method acceptCreation {value} {
    set m_useParamsToCreate $value
  }

  ##
  #
  method appendNewJob {name jobinfo} {
    $m_view appendEntry $name 
    dict set m_masterJobList $name $jobinfo
  }


  # Ensure that this can work
  method clearTree {} {
    $m_view clearTree

    set m_jobDict [dict create]

    set tree [$m_view getTreeWidget]
    set nodes [$tree children {}]
    $tree delete $nodes
  }

  ##
  # Populate the tree data using the a dict of job lists
  method populateTree {jobDict} {
    dict for {key value} $jobDict {
      $self appendNewJob $key $value
    }
  }

  method configureDefaults {jobParams} {
    [dict get $jobParams -inputparams] configure -inputring  OfflineEVBIn
    [dict get $jobParams -hoistparams] configure -sourcering OfflineEVBIn
    [dict get $jobParams -evbparams]   configure -destring  OfflineEVBOut
    [dict get $jobParams -outputparams] configure -ringname  tcp://localhost/OfflineEVBOut
  }
} ;# end of OfflineOrderer

namespace eval JobBuilder {

  variable widgetName  ".builder"
  variable theInstance ""

  proc getInstance {} {
    variable widgetName 
    variable theInstance

    if {$theInstance eq ""} {
      set theInstance [JobBuilderUIPresenter %AUTO% -widgetname $widgetName]
    }
    return $theInstance
  }
  

}


