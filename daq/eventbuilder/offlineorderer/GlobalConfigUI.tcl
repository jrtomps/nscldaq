
#########################################################################################################
#
# INPUT PIPELINE CONFIGURATION DIALOGUE

package provide OfflineEVBGlobalConfigUI 11.0

#package require EventLog 
package require snit
package require Tk
package require OfflineEVBInputPipeline
package require OfflineEVBHoistPipeline
package require evbcallouts
package require OfflineEVBOutputPipeline
package require OfflineEVBJobBuilder
package require OfflineEVBJob
package require ring
package require ExpFileSystem 
package require Configuration

## Overview of the OfflineEVBInputPipelineUI package
#
# The package implements a configuration utility that configures two parameters
# in a fully testable way. The implementation of is based on the Model-View-Presenter
# paradigm in which there is a single presenter object that handles the interaction
# between the view (i.e. the UI), and the model (the parameters). In this implementation,
# the view is extremely simple and basically maintains the display state of the parameters
# and responds to events by routing them to the presenter object. The 
# presenter is in charge of all the logic for responding to events. In fact, the presenter
# owns the view that it is bound to. 
#
# When the presenter is told to apply the display values, it retrieves the display state of the object
# and then sets the model it holds accordingly. The presenter can be provided a model 
# by an outside entity such that it can specifically configure a specific set of parameters.
#
# One of the design considerations here is that there is no apply or cancel button
# that belong to this widget. Instead, it is expected that the apply command will be
# called by something external to this. Imagine a separate widget that owns this and two
# buttons: apply and cancel. On apply, it will simply call the presenter's apply command.




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
# should be creating an InputPipeConfigUIPresenter object which would
# then create the view.
#
snit::widget GlobalConfigUIView {

    option -inputring   -default "OfflineEVBIn"
    option -outputring  -default "OfflineEVBOut" 
    option -stagearea 

    component m_presenter
    
    
    ## @brief Construct the megawidget
    #
    # The user really should not
    #
    # @param presenter an GlobalConfigUIPresenter object
    # @param args      a list of option-value pairs for setting the options
    #
    constructor {presenter args} {
      
      set options(-stagearea) [file join $::env(HOME) offlinestagearea]

      set m_presenter $presenter

      $self configurelist $args 

      # build the gui
      $self buildGUI    
    }

    ## Destroy this thing 
    #
    destructor {
      destroy $win
    }

     
    ## @brief Assemble the megawidget
    #
    # Because this is a snit::widget object, the hull is actually just a ttk::frame.
    # This fills that frame ($win) with a bunch of configuration widgets. These are 
    # simply some labels on text entries.
    #
    method buildGUI {} {


      set top $win.ringFrm
      ttk::labelframe $top -text "Data Flow" -padding 9
      ttk::label $top.iringLbl  -text "Input ring"
      ttk::entry $top.iringEntry -textvariable [myvar options(-inputring)] -width 40
      ttk::label $top.oringLbl  -text "Output ring"
      ttk::entry $top.oringEntry -textvariable [myvar options(-outputring)] -width 40
      grid $top.iringLbl $top.iringEntry -sticky new -pady {0 9}
      grid $top.oringLbl $top.oringEntry -sticky new
      grid columnconfigure $top 0 -weight 1

      set top $win.stageFrm
      ttk::labelframe $top -text "Output Directory " -padding 9 
      tk::text $top.descr -height 6 -width 40 -wrap word -bg lightgray -relief flat
      set msg    "The output of the offline orderer needs a clean output "
      append msg "directory to start processing a job. For that reason, "
      append msg "this program moves its output files to a standard location "
      append msg "following the completion of a job. It requires a valid directory "
      append msg "it can manage as an experimental stagearea."
      $top.descr insert end $msg
      $top.descr configure -state disabled
      ttk::label $top.stageareaLbl   -text "Stagearea Directory" 
      ttk::entry $top.stageareaEntry -textvariable [myvar options(-stagearea)] -width 40
      grid $top.descr -  -sticky new -pady {0 9}
      grid $top.stageareaLbl $top.stageareaEntry -sticky new
      grid columnconfigure $top {0 1} -weight 1

      ttk::button $win.apply -text "Apply"  -command [mymethod onApply]


      grid $win.ringFrm -sticky nsew -pady 9
      grid $win.stageFrm -sticky nsew -pady 9

      grid $win.apply -sticky se -padx 9 -pady 9

      grid columnconfigure $win 0 -weight 1
    }

    ## @brief Pass this a different presenter object 
    #
    # @param presenter an GlobalConfigUIPresenter object
    #
    method setPresenter {presenter} {
      set m_presenter $presenter
    }


    ## @brief Return the value of $win for gridding the view
    #
    method getWindowName {} {return $win}

    method onApply {} {
      $m_presenter apply
    }
}

# End of GlobalConfigUIView code

# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------

## @brief Defines the logic for the GlobalConfigUIView
#
# This is actually quite simple in that it responds to events from the view
# and manages the model. It is allowed to edit the model and is thus responsible
# for synchronizing the state of the model with the displayed values when an 
# apply command is given it.
#
# @important This does not own the model! Rather it just has a reference to the model
#            and the power to manipulate it.
#
snit::type GlobalConfigUIPresenter {

  option -widgetname -default ""
  option -ismaster   -default 1
  option -ownsmodel  -default 1

  component m_model     ;#< The model : OfflineEVBGlobalParams
  component m_view      ;#< The view, owned by this

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
      set msg    "GlobalConfigUIPresenter -widgetname option is mandatory "
      append msg "but was not provided!"
      return -code error $msg 
    }
    
    # Create the default model
    set m_model [dict create -jobname "Job" \
                             -inputparams [OfflineEVBInputPipeParams %AUTO%] \
                             -hoistparams [OfflineEVBHoistPipeParams %AUTO%] \
                             -evbparams   [EVBC::AppOptions %AUTO% -destring OfflineEVBOut] \
                             -outputparams [OfflineEVBOutputPipeParams %AUTO%]]

    set options(-ownsmodel) 1

    # Create the view and pass it the values of the model
    set m_view   [GlobalConfigUIView $options(-widgetname) $self]
    $m_view setPresenter $self
    $self updateViewData $m_model

  }

  ## @brief Destroy the view 
  # 
  # Do not destroy the model because it is just a reference
  #
  destructor {
    catch {destroy $m_view}
  }

  method setInputRing {ring} {
    [dict get $m_model -inputparams] configure -inputring $ring
    $self updateViewData  $m_model
  }

  method setOutputRing {ring} {
    [dict get $m_model -evbparams] configure -destring $ring
    $self updateViewData  $m_model
  }

  method setStagearea {path} {
    [dict get $m_model -outputparams] configure -stagearea $path
    $self updateViewData  $m_model
  }
  ## @brief Set the model to some user's model and synchronize
  #
  #
  # @param model  an OfflineEVBInputPipeParams object
  #
  method setModel {model {own 0}} {
    if {$options(-ownsmodel)} {
      $self destroyModel $m_model
    }
    set m_model $model
    set options(-ownsmodel) $own
    $self updateViewData $m_model
  }

  method destroyModel {model} {
    dict for {key val} $model {
      if {$key ne "-jobname"} {
        $val destroy
      }
    }
  }
  ## @brief Retrieve the model
  #
  # @returns string
  # @retval the name of the OfflineEVBInputPipeParams object this controls
  method getModel {} {
    return $m_model
  }
  
  ## @brief Synchronize the data displayed by the view with the model
  #
  # @param model  an OfflineEVBInputPipeParams object
  #
  method updateViewData {model} {
     $m_view configure -inputring  [[dict get $model -inputparams]  cget -inputring] 
     $m_view configure -outputring [[dict get $model -evbparams] cget -destring]
     $m_view configure -stagearea  [[dict get $model -outputparams] cget -stagearea]
  }

  ## @brief Set the values of the model to what are displayed
  #
  method commitViewDataToModel {} {
    # input ring
    [dict get $m_model -inputparams] configure -inputring  [$m_view cget -inputring] 
    [dict get $m_model -hoistparams] configure -sourcering  [$m_view cget -inputring] 

    # output ring
    [dict get $m_model -evbparams]   configure -destring  [$m_view cget -outputring] 
    [dict get $m_model -outputparams] configure -ringname  "tcp://localhost/[$m_view cget -outputring]"

    # stagearea
    [dict get $m_model -outputparams] configure -stagearea  [$m_view cget -stagearea]
  }

  method copySelectParametersToJob {model} {
    # input ring
    [dict get $model -inputparams] configure -inputring  [[dict get $m_model -inputparams] cget -inputring] 
    [dict get $model -hoistparams] configure -sourcering [[dict get $m_model -hoistparams] cget -sourcering] 

    # output ring
    [dict get $model -evbparams]   configure -destring   [[dict get $m_model -evbparams] cget -destring]
    [dict get $model -outputparams] configure -ringname  [[dict get $m_model -outputparams] cget -ringname]

    # stagearea
    [dict get $model -outputparams] configure -stagearea  [[dict get $m_model -outputparams] cget -stagearea]
  }

  ## @brief Synchronize the model to the view data
  #
  # This is what is intended to be called when the user is done configuring 
  # values and is ready to apply the changes.
  #
  method apply {} {

    # clone our current config in case we need to fall back to it
    set tmpModel [Job::clone $m_model]

    # update our model from view
    $self commitViewDataToModel

    # make the rings if they don't exist
    $self ensureRingsExist

    $self createStagearea

    # check to see if there are problems with it
    set errors [$self validateModel]
    
    if {[llength $errors]==0} {
      # there were no problems...keep the params and 
      # destroy the temporary clone
      $self destroyModel $tmpModel

      set jobList [[JobBuilder::getInstance] getJobsList]
      dict for {key val} $jobList {
        $self copySelectParametersToJob $val
      }

    } else {
      set msg "Found the following errors in the configuration:\n"
      append msg [join $errors "\n"]
      tk_messageBox -icon error -message $msg

      # this was bad view data... revert our model to 
      # its former state before synchronizing
      $self destroyModel $m_model
      set m_model $tmpModel
    }
  }


  ## Check to see whether the ring buffers in the model exist
  # and create them if necessary
  #
  method ensureRingsExist {} {
    # input ring
    set ring [[dict get $m_model -inputparams] cget -inputring]
    $self ensureRingExists $ring

    # output ring
    set ring [[dict get $m_model -evbparams] cget -destring]
    $self ensureRingExists $ring

  }

  ## Check whether the ring buffer exists and create it if it doesn't
  #
  # @param name   name of the ring buffer to check on
  #
  method ensureRingExists {name} {
    if {[catch {ringbuffer usage $name} msg]} {
      ringbuffer create $name
    }
  }


  ## Try to create stagearea if it doesn't exist 
  #
  method createStagearea {} {
    Configuration::Set StageArea [[dict get $m_model -outputparams] cget -stagearea]
    ExpFileSystem::CreateHierarchy
  }

  ## @brief Retrieve the view
  #
  # @returns string
  # @retval the name of an InputPipeConfigUIView object 
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


  ## @brief Valideate only the pieces of this that need to be validated
  #
  method validateModel {} {

    set errors [list]

    [dict get $m_model -inputparams] validateInputRing errors
    [dict get $m_model -hoistparams] validateSourceRing errors
    #      set evbErrors    [[dict get $m_model -evbparams] validate]
    [dict get $m_model -outputparams] validateRing errors
    [dict get $m_model -outputparams] validateStagearea errors

    return $errors

  }
}


namespace eval GlobalConfig {

  variable win ".gblConfig"
  variable theInstance ""

  proc getInstance {} {
    variable win
    variable theInstance

    if {$theInstance eq ""} {
      set theInstance [GlobalConfigUIPresenter %AUTO% -widgetname $win]
    }

    return $theInstance
  }

}
