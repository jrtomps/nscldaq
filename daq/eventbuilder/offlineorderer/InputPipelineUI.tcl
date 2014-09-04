
#########################################################################################################
#
# INPUT PIPELINE CONFIGURATION DIALOGUE

package provide OfflineEVBInputPipelineUI 11.0

package require OfflineEVBInputPipeline
package require snit
package require Tk


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
snit::widget InputPipeConfigUIView {

    option -inputring   -default "OfflineEVBIn"
    option -unglomid    -default 0 

    component m_presenter
    
    
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

      ttk::label $win.ringLabel -text "Input ring"
      ttk::entry $win.ringEntry -textvariable [myvar options(-inputring)]

      ttk::label $win.unglomIDLabel -text "Unglom source ID"
      ttk::entry $win.unglomIDEntry -textvariable [myvar options(-unglomid)] \
                                    -validate key \
                                    -validatecommand [mymethod validateUnglomID]


      set buttonFrame $win.buttons
      ttk::frame $buttonFrame 
      ttk::button $buttonFrame.cancel  -text "Cancel" -command [mymethod onCancel]
      ttk::button $buttonFrame.apply   -text "Apply"  -command [mymethod onApply]
      grid x $buttonFrame.cancel $buttonFrame.apply -sticky ew -padx 9 -pady 9
      grid columnconfigure $buttonFrame {0 1 2} -weight 1

      grid $win.ringLabel     $win.ringEntry     -sticky ew -padx 9 -pady 9
      grid $win.unglomIDLabel $win.unglomIDEntry -sticky ew -padx 9 -pady 9
      grid $buttonFrame        -                 -sticky ew -padx 9 -pady 9

    }

    ## @brief Ensure that the unglom id is actually an integer
    #
    method validateUnglomID {} {
      return [expr {![string is integer $options(-unglomid)]}]
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
snit::type InputPipeConfigUIPresenter {

  option -widgetname -default ""

  component m_model     ;#< The model : OfflineEVBInputPipeParams
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
      set msg    "InputPipeConfigUIPresenter -widgetname option is mandatory "
      append msg "but was not provided!"
      return -code error $msg 
    }
    
    # Create the default model
    set m_model [OfflineEVBInputPipeParams %AUTO%]

    # Create the view and pass it the values of the model
    set m_view   [InputPipeConfigUIView $options(-widgetname) $self] 
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
  # @param model  an OfflineEVBInputPipeParams object
  #
  method setModel {model} {
    set m_model $model
    $self updateViewData $m_model
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
     $m_view configure -inputring [$model cget -inputring] 
     $m_view configure -unglomid  [$model cget -unglomid]
  }

  ## @brief Set the values of the model to what are displayed
  #
  method commitViewDataToModel {} {
    $m_model configure -inputring [$m_view cget -inputring]
    $m_model configure -unglomid  [$m_view cget -unglomid]
  }

  ## @brief Synchronize the model to the view data
  #
  # This is what is intended to be called when the user is done configuring 
  # values and is ready to apply the changes.
  #
  method apply {} {
    $self commitViewDataToModel
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
