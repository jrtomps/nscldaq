

package provide OfflineEVBJobBuilder 11.0

package require Tk
package require snit

package require ConfigErrorUI 
package require OfflineEVBJobConfigUI

package require OfflineEVBInputPipeline
package require OfflineEVBHoistPipeline
package require evbcallouts 
package require OfflineEVBOutputPipeline 


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
  method appendEntry {name value} {
    [$self getTreeWidget] insert {} end -text $name -values $value
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

  ##
  #
  destructor {
    catch { destroy $m_view}
  }

  ##
  #
  method setModel {newModel} {
    set m_model $newModel
    $self updateViewDataFromModel
  }

  ##
  #
  method setView {theview} {
    set m_view $theview
    $self updateViewDataFromModel
  }

  ##
  #
  method getJobsList {} {
    
    return $m_masterJobList
  }

  ##
  #
  method addJob {} {

    set m_useParamsToCreate 0

    toplevel .jobconf
    set config [JobConfigUIPresenter %AUTO% -widgetname .jobconf.ui \
                                            -ismaster 1]
    $config setButtonText "Create"

    set iparams [OfflineEVBInputPipeParams %AUTO%] 
    set hparams [OfflineEVBHoistPipeParams %AUTO%]
    set eparams [EVBC::AppOptions %AUTO%]
    set oparams [OfflineEVBOutputPipeParams %AUTO%]
    set model [dict create -jobname "Job" \
                           -inputparams $iparams  \
                           -hoistparams $hparams \
                           -evbparams   $eparams \
                           -outputparams $oparams ]
    $config setModel $model
    $config setObserver $self
    grid .jobconf.ui -sticky nsew -padx 9 -pady 9
    grid rowconfigure .jobconf.ui 0 -weight 1
    grid columnconfigure .jobconf.ui 0 -weight 1
   
    # grab this so that it is a modal dialogue
    grab .jobconf
    wm transient .jobconf
    wm protocol .jobconf WM_DELETE_WINDOW {
      grab release .jobconf;
      destroy .jobconf
    }

    tkwait window .jobconf

    if {$m_useParamsToCreate} {

      puts "Appending new Job!"
      $self appendNewJob "job$m_nJobs" $model
      puts "$m_nJobs  :\n$model"

      incr m_nJobs

    } else {
      dict for {key val} $model { 
        if {$key ne "-jobname"} {
          $val destroy 
        }
      }
    }
  }


  method removeJob {} {
    set tree [$m_view getTreeWidget]

    set entries [$tree selection]
    if {[llength $entries] == 0 } {
      tk_messageBox -icon error -message "User must select files to remove from list"
    } else {
      $tree delete $entries
    }
  }


  ##
  #
  method editJob {} {
    set m_useParamsToCreate 0

    toplevel .jobconf
    set config [JobConfigUIPresenter %AUTO% -widgetname .jobconf.ui \
                                            -ismaster 1]
    $config setButtonText "Accept"

    set tree [$m_view getTreeWidget]
    set selection [$tree selection]
    $config setModel [$tree item [lindex $selection 0] -values]
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

    tkwait window .jobconf

    if {$m_useParamsToCreate} {
      puts "Job edit accepted!"
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
  method acceptCreation {value} {
    set m_useParamsToCreate $value
  }

  ##
  #
  method appendNewJob {name jobinfo} {
    $m_view appendEntry $name $jobinfo 
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

} ;# end of OfflineOrderer

