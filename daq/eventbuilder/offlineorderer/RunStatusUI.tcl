
#########################################################################################################
#
# INPUT PIPELINE CONFIGURATION DIALOGUE

package provide OfflineEVBRunStatusUI 11.0

package require snit
package require Tk


## Overview of the OfflineEVBInputPipelineUI package


## @brief The view for the input pipeline configuration dialogue
#
# This is the actual gui component. It maintains two parameters that are correlated
# to the model in the presenter that owns it. 
#
# There are three real responsibility associated with this:
# 1. Maintain the display state of the model
# 2. Respond to the events by redirecting them to the presenter
# 3. Assemble the widget
#
# It is not likely that the user will create the view. Instead, the user
# should be creating an RunStatusUIPresenter object which would
# then create the view.
#
snit::widget RunStatusUIView {

    component m_jobDisplays
    component m_presenter
    
    ## @brief Construct the megawidget
    #
    # The user really should not
    #
    # @param presenter an RunStatusUIPresenter object
    # @param args      a list of option-value pairs for setting the options
    #
    constructor {presenter args} {
      
      set m_presenter $presenter

#     $self configurelist $args 

      # build the gui
      $self buildGUI    
    }

    ## Destroy this thing 
    #
    destructor {
      if {[catch {destroy $win} msg]} {
        puts $msg
      }
    }

    method appendNewJobDisplay {jobDisplay} {
      puts "appendNewJobDisplay \"$jobDisplay\""
      grid $jobDisplay -sticky new
      grid columnconfigure $win [expr {[lindex [grid size $win] 1]-1}] -weight 1
    }

    ## @brief Assemble the megawidget
    #
    # Because this is a snit::widget object, the hull is actually just a ttk::frame.
    # This fills that frame ($win) with a bunch of configuration widgets. These are 
    # simply some labels on text entries.
    #
    method buildGUI {} {
      grid rowconfigure $win 0 -weight 1
      grid columnconfigure $win 0 -weight 1
    }

    ## @brief Pass this a different presenter object 
    #
    # @param presenter an RunStatusUIPresenter object
    #
    method setPresenter {presenter} {
      set m_presenter $presenter
    }


    ## @brief Return the value of $win for gridding the view
    #
    method getWindowName {} {return $win}

}

# End of RunStatusUIView code

# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------

## @brief Defines the logic for the RunStatusUIView
#
# This is actually quite simple in that it responds to events from the view
# and manages the model. It is allowed to edit the model and is thus responsible
# for synchronizing the state of the model with the displayed values when an 
# apply command is given it.
#
# @important This does not own the model! Rather it just has a reference to the model
#            and the power to manipulate it.
#
snit::type RunStatusUIPresenter {

  option -widgetname -default ""

  component m_model     ;#< The model : OfflineEVBInputPipeParams
  component m_view      ;#< The view, owned by this

  variable m_parent 
  variable m_jobDisplayNames

  ## Construct the model, view, and synchronize view
  #
  # It is necessary that the user provides the name of the widget. If that is
  # not provided this cries, "Uncle!" The presenter is tightly bound to the view
  # and actually owns it. It passes the view itself on construction so that there is never
  # any confusion about who is in charge of it.
  #
  # @returns the name of this object
  # 
  # Exceptional returns
  # - Error if -widgetname is not provided.
  #
  constructor {args} {
    $self configurelist $args
   
    if {$options(-widgetname) eq ""} {
      set msg    "RunStatusUIPresenter -widgetname option is mandatory "
      append msg "but was not provided!"
      return -code error $msg 
    }
    
    # Create the default model
    set m_model [dict create processing "" queued [list] completed [list]] 

    # Create the view and pass it the values of the model
    set m_view   [RunStatusUIView $options(-widgetname) $self] 
    $m_view setPresenter $self
    $self updateViewData $m_model

    set m_jobDisplayNames [dict create]
  }

  ## @brief Destroy the view 
  # 
  # Do not destroy the model because it is just a reference
  #
  destructor {
    if {[catch {destroy $m_view} msg]} {
      puts $msg
    }
  }


  ## @brief Set the model to some user's model and synchronize
  #
  #
  # @param model  an OfflineEVBInputPipeParams object
  #
  method setModel {model} {
    set m_model $model
    $self updateViewData $m_model
  }

  ##
  #
  method transitionToNextJob {jobName} {
    # mark the current job as completed
    $self transitionCurrent
    
    # find the new job in the list of queued  and make it current
    set queuedJobs [dict get $m_model queued]
    set newJobIndex [lsearch $queuedJobs $jobName]
    if {$newJobIndex>=0} {
      dict set m_model processing [lindex $queuedJobs $newJobIndex]

      # remove the current job from queued list
      set queuedJobs [lreplace $queuedJobs $newJobIndex $newJobIndex]
      dict set m_model queued $queuedJobs
    }

    # synchronize the view with the model
    $self updateViewData $m_model
  }

  method transitionCurrent {} {

    set current [dict get $m_model processing]
    puts "Current : $current"
    if {$current ne ""} {
      dict lappend m_model completed $current
      dict set m_model processing ""

      puts [dict get $m_model completed]
    }
  }

  ## @brief Retrieve the model
  #
  # @returns string
  # @retval the name of the OfflineEVBInputPipeParams object this controls
  method getModel {} {
    return $m_model
  }
  
  method setParent {parent} {
    set m_parent $parent
  }

  method finish {} {
    $self transitionCurrent
    $self updateViewData $m_model 
  }

  ## @brief Synchronize the data displayed by the view with the model
  #
  # @param model  an OfflineEVBInputPipeParams object
  #
  method updateViewData {model} {

    $self updateDisplayDataForStatusType completed 

    $self updateDisplayDataForStatusType processing

    $self updateDisplayDataForStatusType queued

  }

  ##
  #
  method updateDisplayDataForStatusType {status} {

    # get the list of jobs for the given status
    set jobList [dict get $m_model $status]

    # handle each of the statuses
    foreach job $jobList {
      if {[$self isDisplayedJob $job]} {
        [dict get $m_jobDisplayNames $job] configure -status $status 
      } else {
        # the job doesn't exist as a display object
        # make a new one and then pass it the view to display
        set widgetName [$self findUniqueName]
#        puts "New widget: $widgetName, Existing widgets: $m_jobDisplayNames"
        set newJob [JobStatusDisplay $widgetName -status $status -jobname $job]
#        puts "New job name : $newJob"
        dict set m_jobDisplayNames $job $newJob

        $m_view appendNewJobDisplay $widgetName
      }
    }

  }

  ##
  #
  method getDisplayedJobs {} {
    return $m_jobDisplayNames
  }

  ##
  # (Primarily for testing)
  #
  method setDisplayedJobs {jobs} {
    set m_jobDisplayNames $jobs
  }

  ##
  #
  method isDisplayedJob {job} {
    return [dict exists $m_jobDisplayNames $job]
  }

  ##
  #
  method findUniqueName {} {

    # scan until find a name that doesn't exist in the list 
    set w "$options(-widgetname).jobDisplay"
    set index 0
    set name [format "%s%d" $w $index]
    set existingNames [dict values $m_jobDisplayNames]
    while {[lsearch $existingNames $name]>=0} {
      incr index
      set name [format "%s%d" $w $index]
    }

    return $name
  }

  ## @brief Retrieve the view
  #
  # @returns string
  # @retval the name of an RunStatusUIView object 
  method getViewObj {} {
    return $m_view
  }

  ## @brief Retrieve the name of the widget this is ultimately connected to
  #
  # The user actually should already have access to this information because 
  # they provided the info via the -widgetname option. However, it is trivial
  # to let them ask for the name again.
  #
  # @return string
  # @retval the widget name (i.e. .widget)
  method getViewWidget {} {
    return [$m_view getWindowName]
  }
}


snit::widget JobStatusDisplay {

  option -jobname
  option -status -configuremethod setStatus

  constructor {args} {
    $self buildGUI
    $self configurelist $args
  }

  method buildGUI {} {
    ttk::label $win.jobLbl  -textvariable [myvar options(-jobname)]
    ttk::label $win.jobStatusLbl -textvariable [myvar options(-status)]
    ttk::progressbar $win.jobProgress -orient horizontal -mode indeterminate

    grid $win.jobLbl $win.jobStatusLbl $win.jobProgress -sticky ew -padx 9 -pady 9

    grid columnconfigure $win {0 1 2} -weight 1 -minsize 81
  }

  method setStatus {option value} {
    if {$value eq "processing"} {
      grid $win.jobProgress -column 2 -row 0 -sticky ew
      $win.jobProgress start 20
      set options(-status) $value
    } elseif {$value in [list "completed" "queued"]} {
      $win.jobProgress stop
      grid forget $win.jobProgress
      grid configure $win.jobStatusLbl -columnspan 2
      set options(-status) $value
    }

 } 
}
