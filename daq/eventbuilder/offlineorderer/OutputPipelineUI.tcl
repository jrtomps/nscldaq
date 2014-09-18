
#########################################################################################################
#
# INPUT PIPELINE CONFIGURATION DIALOGUE

package provide OfflineEVBOutputPipelineUI 11.0

package require OfflineEVBOutputPipeline
package require Configuration
package require snit
package require Tk


## Overview of the OfflineEVBOutputPipelineUI package
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
snit::widget OutputPipeConfigUIView {

  option -showbuttons 1

  component m_presenter

  variable m_form

  ## @brief Construct the megawidget
  #
  # The user really should not
  #
  # @param presenter an InputPipeConfigUIPresenter object
  # @param args      a list of option-value pairs for setting the options
  #
  constructor {presenter args} {

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
    variable m_form

    set m_form $win.params
    EventLog::ParameterPrompter $m_form

    set spaceFrame $win.space
    ttk::frame $spaceFrame

    set buttonFrame $win.buttons
    ttk::frame $buttonFrame 
    ttk::button $buttonFrame.cancel  -text "Cancel" -command [mymethod onCancel]
    ttk::button $buttonFrame.apply   -text "Apply"  -command [mymethod onApply]
    grid $buttonFrame.cancel $buttonFrame.apply -sticky ew -padx 9 -pady 9
#    grid $buttonFrame.apply -sticky ew -padx 9 -pady 9
    grid columnconfigure $buttonFrame {0 1 2} -weight 1


    grid $m_form  -padx 9 -pady 9 -sticky new
    grid $spaceFrame  -padx 9 -pady 9 -sticky nsew

    if {$options(-showbuttons)} {
      grid $win.buttons -padx 9 -pady 9 -sticky sew
    }
    grid rowconfigure $win 1 -weight 1
  }


  ## @brief Pass this a different presenter object 
  #
  # @param presenter an InputPipeConfigUIPresenter object
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

  method onCancel {} {
    $m_presenter cancel 
  }


  method getFormObj {} { 
    variable m_form
    return $m_form
  }
}

# End of InputPipeConfigUIView code

# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------

## @brief Defines the logic for the InputPipeConfigUIView
#
# This is actually quite simple in that it responds to events from the view
# and manages the model. It is allowed to edit the model and is thus responsible
# for synchronizing the state of the model with the displayed values when an 
# apply command is given it.
#
# @important This does not own the model! Rather it just has a reference to the model
#            and the power to manipulate it.
#
snit::type OutputPipeConfigUIPresenter {

  option -widgetname -default ""
  option -ismaster   -default 1

  component m_model     ;#< The model : OfflineEVBOutputPipeParams
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
      set msg    "OutputPipeConfigUIPresenter -widgetname option is mandatory "
      append msg "but was not provided!"
      return -code error $msg 
    }
    
    # Create the default model
    set m_model [OfflineEVBOutputPipeParams %AUTO%]

    # Create the view and pass it the values of the model
    if {$options(-ismaster)} {
      set m_view   [OutputPipeConfigUIView $options(-widgetname) $self -showbuttons 1] 
    } else {
      set m_view   [OutputPipeConfigUIView $options(-widgetname) $self -showbuttons 0] 
    }
    $m_view setPresenter $self
    $self updateViewData $m_model

  }

  ## @brief Destroy the view 
  # 
  # Do not destroy the model because it is just a reference
  #
  destructor {
    catch {$m_view destroy}
  }


  ## @brief Set the model to some user's model and synchronize
  #
  #
  # @param model  an OfflineEVBOutputPipeParams object
  #
  method setModel {model} {
    set m_model $model
    $self updateViewData $m_model
  }

  ## @brief Retrieve the model
  #
  # @returns string
  # @retval the name of the OfflineEVBOutputPipeParams object this controls
  method getModel {} {
    return $m_model
  }
  
  ## @brief Synchronize the data displayed by the view with the model
  #
  # @param model  an OfflineEVBOutputPipeParams object
  #
  method updateViewData {model} {
    set form [$m_view getFormObj]
    $form configure -logger             [Configuration::get EventLogger] 
    $form configure -ring               [$model cget -ringname]
    $form configure -usensrcs           1
    $form configure -additionalsources  [expr [$model cget -nsources]-1] 
    $form configure -forcerun           0
    $form configure -usechecksum        1
  }

  ## @brief Set the values of the model to what are displayed
  #
  method commitViewDataToModel {} {
    set form [$m_view getFormObj]
    $m_model configure -logger             [$form cget -logger]
    $m_model configure -ringname           [$form cget -ring]
    $m_model configure -usensrcs           [$form cget -usensrcs]
    
    $m_model configure -nsources           [expr {[$form cget -additionalsources]+1}]
    $m_model configure -forcerun           [$form cget -forcerun]
    $m_model configure -usechecksum        [$form cget -usechecksum] 
  }  

  ## @brief Synchronize the model to the view data
  #
  # This is what is intended to be called when the user is done configuring 
  # values and is ready to apply the changes.
  #
  method apply {} {
    $self commitViewDataToModel
  }

  ## @brief Kill off the top level widget 
  #
  method cancel {} {
    if {$options(-ismaster)} {
      set widget [$m_view getWindowName]
      set top [winfo toplevel $widget]
      destroy $top
    } 
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
}
