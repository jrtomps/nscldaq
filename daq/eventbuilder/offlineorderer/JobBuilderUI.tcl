

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


## \defgroup jobbuilder {The Job Builder Package}
# 
# A package for managing a set of jobs. It is responsible for providing the
# user mechanisms for adding, removing, and editing the set of jobs that are
# being readied for processing.
#
# This is implemented using the Model-View-Presenter paradigm as many of the other
# GUI components in the Offline EVB. The general idea is that the user instantiates
# presenter object and then all software deals with the presenter whereas the user
# deals with the view object, which is created by the presenter. The view does not
# provide any intelligence to the GUI and merely displays data while forwarding 
# events to the presenter.
#


## @brief The JobBuilderUI View
#
# @ingroup jobbuilder
#
# The JobBuilderUIView is a simple megawidget that provides a treeview and buttons
# to add, edit, and remove items from the treeview. The treeview holds a list of jobs
# that the user intends to manipulate. It forwards its button events to the 
# presenter object (JobBuilderUIPresenter). 
#
snit::widget JobBuilderUIView {
  
  component m_presenter         ;#< The presenter

  variable listWidget ""        ;#< The treeview widget

  
  ## @brief Construct object and build the GUI
  #
  # This doesn't make the view visible but will handle the gridding of all the 
  # widgets that make it up. Initially, the object knows nothing about a presenter
  # and is thus dumb. This is further reason why the user should not be creating 
  # one of the objects directly. It MUST be paired with a presenter object through
  # the setPresenter method if it is to do anything useful.
  # 
  # @params args  not used...
  constructor {args} {

    set m_presenter ""

    # assemble the megawidget
    $self buildGUI

    # setup some special styles
    $self configureStyles
  }

  ## @brief Destroys the widget
  #
  # Note that this has not destroy method, but rather is treated just like a normal
  # widget. It is destroyed as "destroy $widget"
  destructor {
  }


  ## @brief Build the GUI
  #
  # This instantiates all of the widgets that compose the megawidget and grids them
  # all appropriately.
  #
  method buildGUI {} {

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
  
  ## @brief Define the text style used to display the "Jobs To Process"
  # 
  # We manage the H1.TLabel style here.
  #
  method configureStyles {} {
    ttk::style configure H1.TLabel -font "helvetica 18"
  }

  ## @brief Forward "Add Job" button event to presenter
  #
  method onAddJob {} {
    $m_presenter addJob
  }

  ## @brief Forward "Edit Selected" button event to presenter
  #
  method onEditJob {} {
    $m_presenter editJob 
  }

  ## @brief Forward "Remove Selected" button event to presenter
  #
  method onRemoveJob {} {
    $m_presenter removeJob 
  }

  ## @brief Query the name of the presenter object that controls this
  #
  # @returns the name of the presenter
  method getPresenter {} {
    return $m_presenter
  }

  ## @brief Connect a presenter object to control this.
  #
  # This only knows how to talk to 1 presenter object, but multiple
  # presenter object can control it. The presenter is not owned by this
  # in any respect and is merely referenced.
  #
  # @param newPresenter a JobBuilderUIPresenter object
  method setPresenter {newPresenter} {
    set m_presenter $newPresenter
  }

  ## @brief Retrieve the name of the treeview widget
  #
  # @returns widget name
  method getTreeWidget {} {
    return $listWidget.list 
  }

  ## @brief Insert a new item into the treeview at the end of the list
  #
  # All added jobs are children of the root node and are appended to 
  # the very end.
  #
  # @param name   the name to display as text and also to use as an id
  method appendEntry {name} {
    [$self getTreeWidget] insert {} end -id $name -text $name
  }


  method clear {} {
    set tree [$self getTreeWidget] 
    # empty it (all items were children of the root node)
    $tree delete [$tree children {}]
  }
}

# -----------------------------------------------------------------------
# -----------------------------------------------------------------------
# -----------------------------------------------------------------------
#

## @brief The presenter object for the JobConfigUI Package
#
# @ingroup jobbuilder
#
# This is the class that maintains the logic of the display. It owns a 
# view object (JobBuilderUIView) and manipulates it. It is also responsible
# for launching dialogues for the user to interact with when configuring
# the gui.
#
# The model that this maintains is a dictionary of jobs. The keys of 
# the dictionary are the names of the jobs, so that it is not allowed for
# 2 jobs to have the same job name. The value that is referenced by the key
# is a dictionary whose keys are: -jobname, -inputparams, -hoistparams, 
# -evbparams, and -outputparams. These contain enough information for 
# constructing the entire 4 pipelines that will be used to process a set
# of files as a job.
#
#
snit::type JobBuilderUIPresenter {
  option -widgetname     -default ""  ;#< The name of the view widget
  
  component m_view                    ;#< The view itself

  variable m_masterJobList            ;#< The dictionary of jobs
  variable m_useParamsToCreate        ;#< Flag that can be set to determine
                                      ;# whether a job being configured is 
                                      ;# to be accepted.
  variable m_nJobs                    ;# A counter to create unique job names

  ## @brief Constructor
  #
  # THis demands that the name for the view widget is provided and will 
  # throw an error if not provided. This is important because the presenter
  # is only really a sensible thing to have if it has a view to control.
  # the view is thus owned by the presenter.
  # 
  # @param  args  option-value pairs 
  # 
  # @throws an error if the user didn't provide a value for -widgetname 
  constructor {args} {

    # allow the user to override the defaults
    $self configurelist $args

    # verify that the user provided a name for the view
    if {$options(-widgetname) eq ""} {
      set msg    "JobBuilderUIPresenter -widgetname option is mandatory "
      append msg "but was not provided!"
      return -code error $msg 
    }

    # Construct the view and introduce itself to the view for receiving
    # events
    set m_view [JobBuilderUIView $options(-widgetname)]
    $m_view setPresenter $self 

    # The initial job list is an empty dict
    set m_masterJobList [dict create]

    # Some helper variable.
    set m_useParamsToCreate 0
    set m_nJobs 0
  }

  ## @brief Destroy the view
  #
  destructor {
    catch { destroy $m_view}
  }

  ## @brief Set the model
  #
  # Replace the current model with a new model. This owns the list of jobs
  # and is allowed to destroy them all.
  method setModel {newModel} {
    set m_masterJobList $newModel
    $self updateViewDataFromModel
  }

  ## @brief Set the view that this controls
  #
  # This replaces any view that was previously being controlled by this. The
  # ownership of the old view is transfered to the caller.
  #
  method setView {theview} {
    set prevView $m_view

    set m_view $theview
    $self updateViewDataFromModel

    return $prevView
  }

  method getView {} {
    return $m_view
  }

  ## @brief Update the displayed data with the model
  #
  method updateViewDataFromModel {} {
      
    # clear the entries in the tree (none should be displayed after this)
    $m_view clear

    # reset counter 
    set m_nJobs 0

    # get the list of jobs
    dict for {name job} $m_masterJobList {
      set newName [$self constructNewJobName $job]
      $self appendNewJob $newName $job 
      incr m_nJobs
    }
     
  }

  ## Retrieve the list of jobs
  #
  method getJobsList {} {
    return $m_masterJobList
  }

  ## Retrieive the list of jobs
  #
  method setJobsList {list} {
    dict for {key val} $m_masterJobList {
      Job::destroy $val
    }
    set m_masterJobList $list
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
      Job::destroy $model
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


