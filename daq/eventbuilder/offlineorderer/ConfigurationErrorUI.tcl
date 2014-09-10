
package provide ConfigErrorUI 11.0

package require Tk
package require snit

snit::widget ConfigErrorView {
  option -errors

  component m_presenter
  component m_errorTree

  constructor {presenter args} {

    set m_presenter $presenter

    $self configurelist $args
    $self buildGUI
  }


  method buildGUI {} {

    # install the treeview
    install m_errorTree using ttk::treeview $win.errors 

    set top $win.buttons
    ttk::frame $top
    ttk::button $top.okay -text "Okay" -command [mymethod onOkay]
    grid $top.okay -sticky se -padx 9 -pady 9
    grid columnconfigure $top 0 -weight 1 
    
    grid $win.errors -sticky nsew -padx 9 -pady 9
    grid $win.buttons -sticky sew -padx 9 -pady 9
    grid rowconfigure $win 0 -weight 1
    grid columnconfigure $win 0 -weight 1

  }

  method onOkay {} {
    $m_presenter okay
  }

  method addJobEntry {name errorDict} {
    $m_errorTree insert {} end -id $name -text $name 
    dict for {key errors} $errorDict {
      if {[llength $errors]>0} {
        $m_errorTree insert $name end -id "$name$key" -text $key
        foreach err $errors { 
          $m_errorTree insert "$name$key" end -text $err
        }
      }
    } 
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



snit::type ConfigErrorPresenter {

  option -widgetname -default ""

  component m_model
  component m_view

  constructor {args} {
    $self configurelist $args
   
    if {$options(-widgetname) eq ""} {
      set msg    "ConfigErrorPresenter -widgetname option is mandatory "
      append msg "but was not provided!"
      return -code error $msg 
    }
    
    # Create the default model (an empty dict)
    set m_model [dict create] 

    # Create the view and pass it the values of the model
    set m_view   [ConfigErrorView $options(-widgetname) $self] 
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
    dict for {job errDict} $model {
      $m_view addJobEntry $job $errDict
    }
  }

  ## @brief Set the values of the model to what are displayed
  #
  method commitViewDataToModel {} {
  }

  ## @brief Synchronize the model to the view data
  #
  # This is what is intended to be called when the user is done configuring 
  # values and is ready to apply the changes.
  #
  method okay {} {
    set top [winfo toplevel [$m_view getWindowName]]
    destroy $top
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


