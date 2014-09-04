
#########################################################################################################
#
# INPUT PIPELINE CONFIGURATION DIALOGUE

package provide OfflineEVBInputPipelineUI 11.0

package require OfflineEVBInputPipeline
package require snit
package require Tk



## The view for the input pipeline configuration dialogue
#
# This is the actual gui component.
#
#
snit::widget InputPipeConfigUIView {

    option -inputring   -default "OfflineEVBIn"
    option -unglomid    -default 0 

    component m_presenter
    
    
    constructor {presenter args} {
      set m_presenter $presenter

      $self configurelist $args 

      $self buildGUI    
    }

    destructor {
      destroy $win
    }

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


    method validateUnglomID {} {
      return [expr {![string is integer $options(-unglomid)]}]
    }

    method setPresenter {presenter} {
      set m_presenter $presenter
    }

    method getWindowName {} {return $win}
}


##
#
snit::type InputPipeConfigUIPresenter {

  option -widgetname -default ""

  component m_model
  component m_view

  ## Construct the model, view, and synchronize view
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

  destructor {
    catch {$m_view destroy}
  }


  ## Set the model to some user's model and synchronize
  #
  method setModel {model} {
    set m_model $model
    $self updateViewData $m_model
  }

  ## Get the model
  #
  method getModel {} {
    return $m_model
  }
  
  ## Synchronize the data displayed by the view with the model
  #
  method updateViewData {model} {
     $m_view configure -inputring [$model cget -inputring] 
     $m_view configure -unglomid  [$model cget -unglomid]
  }

  ## 
  #
  method commitViewDataToModel {} {
    $m_model configure -inputring [$m_view cget -inputring]
    $m_model configure -unglomid  [$m_view cget -unglomid]
  }

  ## Commit the view data to the model
  #
  method apply {} {
    $self commitViewDataToModel
  }

  method getViewObj {} {
    return $m_view
  }

  method getViewWidget {} {
    return [$m_view getWindowName]
  }
}
