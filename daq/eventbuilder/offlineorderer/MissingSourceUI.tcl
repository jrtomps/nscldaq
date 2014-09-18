
#########################################################################################################
#
# INPUT PIPELINE CONFIGURATION DIALOGUE

package provide OfflineEVBMissingSourceUI 11.0

package require OfflineEVBInputPipeline
package require OfflineEVBHoistPipeline
package require snit
package require Tk


## Overview of the OfflineEVBMissingSourcelineUI package
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
snit::widget MissingSourceConfigUIView {

  option -sourcering      -default "OfflineEVBIn"
  option -tstamplib       -default "" 
  option -id              -default 0
  option -info            -default "Data from OfflineEVBIn" 

  option -missing         -default 0  -configuremethod setMissingMode
  option -showbuttons     -default 1

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

    ttk::checkbutton $win.missing -text "Missing body headers" \
                                  -variable [myvar options(-missing)]\
                                  -onvalue 1 -offvalue 0]
    trace add variable [myvar options(-missing)] write [mymethod onMissingChange]
    set top $win.descr
    ttk::frame $top
    font create DescriptionFont -family Helvetica -size 10 -slant italic
    tk::text $top.descrLbl -bg lightgray  -relief flat -wrap word -font DescriptionFont \
                           -height 3 -width 60
    $top.descrLbl insert end  "Check this if some ring items lack body headers in the data, because additional information is required to send these items through the event builder."
    $top.descrLbl configure -state disabled

    grid $top.descrLbl -sticky nsew
    grid rowconfigure $top 0 -weight 1
    grid columnconfigure $top 0 -weight 1
          
    set top $win.params
    ttk::frame $top 

    ttk::label $top.tstampLabel   -text "Timestamp extraction library"
    ttk::entry $top.tstampEntry -textvariable [myvar options(-tstamplib)]
    ttk::button $top.tstampBrowse -text "Browse..." -command [mymethod _browseTstamp]
    ttk::label $top.idLabel   -text "Source ID"
    ttk::entry $top.idEntry -textvariable [myvar options(-id)]

    grid $top.tstampLabel $top.tstampEntry $top.tstampBrowse \
                                              -sticky ew
    grid $top.idLabel     $top.idEntry     x  -sticky ew

    set spaceFrame $win.space
    ttk::frame $spaceFrame

    set buttonFrame $win.buttons
    ttk::frame $buttonFrame 
    ttk::button $buttonFrame.cancel  -text "Cancel" -command [mymethod onCancel]
    ttk::button $buttonFrame.apply   -text "Apply"  -command [mymethod onApply]
    grid $buttonFrame.cancel $buttonFrame.apply -sticky ew -padx 9 -pady 9
    grid columnconfigure $buttonFrame {0 1 2} -weight 1


    grid $win.missing -row 0 -sticky new
    if {$options(-missing)} {
      grid $win.params  -row 1 -sticky new
    } else {
      grid $win.descr  -row 1 -sticky new
    }
    grid $spaceFrame  -row 2 -sticky nsew

    if {$options(-showbuttons)} {
      grid $win.buttons -row 3 -sticky sew -padx 9 -pady 9
    }
    grid rowconfigure    $win 2 -weight 1
    grid columnconfigure $win 0 -weight 1
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

  method _browseTstamp {} {
    set types {
      {{Shared libraries} {.so}}
      {{All files}        {*}}
    }
    
    set file [tk_getOpenFile  -filetypes $types \
                              -title "Select a timestamp extraction library" \
                              -initialdir $::env(HOME)]

    $self configure -tstamplib $file
  }

  method setMissingMode {option value} {

    if {[string is true $value]} {
      grid remove $win.descr
      grid $win.params -row 1 -sticky new
      set options(-missing) 1 
    } else {
      grid remove $win.params
      grid $win.descr -row 1 -sticky new
      set options(-missing) 0
    }
  }

  method onMissingChange {name1 name2 op} {
    if {$options(-missing) == 1} {
      $self configure -missing 1
    } else {
      $self configure -missing 0
    }
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
snit::type MissingSourceConfigUIPresenter {

  option -widgetname -default ""
  option -ismaster   -default 0
  option -ownmodel   -default 1

  component m_model     ;#< The model : OfflineEVBMissingSourceParams
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
      set msg    "MissingSourceConfigUIPresenter -widgetname option is mandatory "
      append msg "but was not provided!"
      return -code error $msg 
    }
    
    set options(-ownmodel) 1
    # Create the default model
    set m_model [dict create -inputparams [OfflineEVBInputPipeParams %AUTO%] \
                             -hoistparams [OfflineEVBHoistPipeParams %AUTO%] ]

    # Create the view and pass it the values of the model
    if {$options(-ismaster) eq ""} {
      set m_view   [MissingSourceConfigUIView $options(-widgetname) $self -showbuttons 1] 
    } else {
      set m_view   [MissingSourceConfigUIView $options(-widgetname) $self -showbuttons 0] 
    }
    $m_view setPresenter $self
    $self updateViewData $m_model

  }

  ## @brief Destroy the view 
  # 
  # Do not destroy the model because it is just a reference
  #
  destructor {
    if {$options(-ownmodel)} {
      dict for {key val} $m_model { $val destroy }
    }
    catch {$m_view destroy}
  }


  ## @brief Set the model to some user's model and synchronize
  #
  #
  # @param model  an OfflineEVBMissingSourceParams object
  #
  method setModel {model {own 0}} {

    # delete the model if we own it
    if {$options(-ownmodel)} {
      dict for {key val} $m_model { $val destroy }
    }
    
    set m_model $model
    set options(-ownmodel) $own

    $self updateViewData $m_model
  }

  ## @brief Retrieve the model
  #
  # @returns string
  # @retval the name of the OfflineEVBMissingSourceParams object this controls
  method getModel {} {
    return $m_model
  }
  
  ## @brief Synchronize the data displayed by the view with the model
  #
  # @param model  an OfflineEVBMissingSourceParams object
  #
  method updateViewData {model} {

    set tstamplib [[dict get $model -hoistparams] cget -tstamplib]
    set id        [[dict get $model -hoistparams] cget -id]
    set expectbh  [[dict get $model -hoistparams] cget -expectbheaders]

    $self synchronizeParams $tstamplib $id

    $m_view configure -tstamplib $tstamplib
    $m_view configure -id        $id
    $m_view configure -missing   [expr {!$expectbh}] 
  }

  ## @brief Set the values of the model to what are displayed
  #
  method commitViewDataToModel {} {

    set tstamplib [$m_view cget -tstamplib]
    set id        [$m_view cget -id]
    set missing   [$m_view cget -missing]

    [dict get $m_model -hoistparams] configure -expectbheaders [expr {!$missing}]
    $self synchronizeParams $tstamplib $id
  }  


  method synchronizeParams {tstamplib id} {

    [dict get $m_model -inputparams] configure -unglomid   $id

    [dict get $m_model -hoistparams] configure -tstamplib  $tstamplib 
    [dict get $m_model -hoistparams] configure -id         $id 
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
