
#########################################################################################################
#
# INPUT PIPELINE CONFIGURATION DIALOGUE

package provide OfflineEVBJobConfigUI 11.0

package require FileListWidget
package require OfflineEVBMissingSourceUI

package require OfflineEVBInputPipeline
package require OfflineEVBHoistPipeline
package require evbcallouts
package require OfflineEVBOutputPipeline

package require snit
package require Tk


## Overview of the OfflineEVBJoblineUI package
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
snit::widget JobConfigUIView {

  option -showbuttons     -default 1
  
  option -nsources        -default 2
  option -jobname         -default "Job"

  option -missingwidget   -default ""
  option -buildwidget     -default ""
  option -showbuttons     -default 1
  option -buttontext      -default "Create"

  component m_presenter
  variable m_fileTree

  ## @brief Construct the megawidget
  #
  # The user really should not
  #
  # @param presenter an InputPipeConfigUIPresenter object
  # @param args      a list of option-value pairs for setting the options
  #
  constructor {presenter args} {

    set m_presenter $presenter
    set m_fileTree ""

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
    variable m_paramFrame
    
    ttk::label $win.jobNameLbl    -text "Job name"
    ttk::label $win.jobNameEntry  -textvariable [myvar options(-jobname)]

    set top $win.fileFrame
    set m_fileTree $top.files

    ttk::frame $top
    ttk::label $top.addFilesLbl -text "Add run files"
    FileList $m_fileTree
    grid $top.addFilesLbl -sticky new
    grid $top.files -sticky nsew
    grid rowconfigure    $top 0 -weight 1
    grid columnconfigure $top 0 -weight 1

    set m_paramFrame $win.params
    set top $m_paramFrame
    ttk::frame $top 

    ttk::label $top.nsrcsLbl -text "Number of end runs to expect"
    ttk::entry $top.nsrcsEntry -textvariable [myvar options(-nsources)] -width 3


    set buttons $win.buttons
    ttk::frame $buttons 
    ttk::button $buttons.cancel -text "Cancel" -command [mymethod onCancel]
    ttk::button $buttons.create -textvariable [myvar options(-buttontext)]\
                                -command [mymethod onCreate]
    grid $buttons.cancel $buttons.create -sticky e -padx {9 0}

    grid $top.nsrcsLbl $top.nsrcsEntry  -sticky nw 
    if {$options(-missingwidget) ne ""} {
      $self gridMissingMissingWidget $options(-missingwidget)
    }
    if {$options(-buildwidget) ne ""} {
      $self gridBuildWidget $options(-buildwidget)
    }
    grid configure $top.nsrcsEntry -sticky ne


    grid $win.fileFrame  -row 0 -column 0 -padx {0 9} -sticky nsew
    grid $top        -row 0 -column 1 -padx {9 0} -sticky nsew
    grid x $buttons  -row 1 -padx 9 -sticky sew -pady 9
    grid columnconfigure $win {0 1} -weight 1 -minsize 300

  }

  method gridMissingWidget {name} {
    variable m_paramFrame
      grid $name - -row 1 -sticky new -pady 9 -in $m_paramFrame
  }

  method gridBuildWidget {name} {
    variable m_paramFrame
      grid $name -  -row 2 -sticky new -in $m_paramFrame
  }

  method onCancel {} {
    $m_presenter cancel
  }

  method onCreate {} {
    $m_presenter create
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


  ##
  #
  method getFileListPresenter {} {
    return $m_fileTree
  }
}

# End of JobConfigUIView code

# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------

snit::type JobConfigUIPresenter {

  option -widgetname -default ""
  option -ismaster   -default 0
  option -ownmodel   -default 1

  variable m_model     ;#< The model : OfflineEVBJobParams
  variable m_view      ;#< The view, owned by this
  variable m_missing   ;#< The view, owned by this
  variable m_build     ;#< The view, owned by this

  variable m_filelist ;#< the view's file list widget

  variable m_observer  ;#< observes whether this is to be accepted

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

    set m_observer ""
   
    if {$options(-widgetname) eq ""} {
      set msg    "JobConfigUIPresenter -widgetname option is mandatory "
      append msg "but was not provided!"
      return -code error $msg 
    }
    
#    # Create the default model 
    set options(-ownmodel) 1 ;# indicate that we can destroy this
    set iparams [OfflineEVBInputPipeParams %AUTO%] 
    set hparams [OfflineEVBHoistPipeParams %AUTO%]
    set eparams [EVBC::AppOptions %AUTO%]
    set oparams [OfflineEVBOutputPipeParams %AUTO%]
    set m_model [dict create -inputparams $iparams  \
                             -hoistparams $hparams  \
                             -evbparams   $eparams  \
                             -outputparams $oparams \
                             -jobname "Job" ]

    # Create the view and pass it the values of the model and the widgets
    # that it will display
    if {$options(-ismaster) eq ""} {
      set m_view   [JobConfigUIView $options(-widgetname) $self \
                                    -showbuttons 1 ]
    } else {
      set m_view   [JobConfigUIView $options(-widgetname) $self \
                                    -showbuttons 0 ] 
    }
    $m_view setPresenter $self


    ##
    #
    set m_filelist [$m_view getFileListPresenter]

    # Create a new dictionary containing a parameter set to pass to the 
    # the MissingSourceConfigUIPresenter to operate on.
    set m_missingModel [dict create -inputparams [dict get $m_model -inputparams] \
                                    -hoistparams [dict get $m_model -hoistparams]]
    set m_missing [MissingSourceConfigUIPresenter %AUTO% \
                                  -widgetname $options(-widgetname).missing]
    $m_missing setModel $m_missingModel

    # The build event widget is much simpler to set up because it only
    # operates on a single parameter set. We just send it the EVBC::AppOptions
    # piece
    set m_build   [BuildEventsPresenter %AUTO% \
                                  -widgetname $options(-widgetname).build]
    $m_build setModel [dict get $m_model -evbparams]

    $m_view gridMissingWidget $options(-widgetname).missing
    $m_view gridBuildWidget   $options(-widgetname).build

    # Tree the model as though we don't own it so that we prevent deleting it
    # here. If we were to delete it, then that we delete the current model and then 
    # try to use it again as the new model
    $self updateViewData $m_model

  }

  ## @brief Destroy the view 
  # 
  # Do not destroy the model because it is just a reference
  #
  destructor {
    if {$options(-ownmodel)} {
      dict for {key val} $m_model { 
        if {$key ne "-jobname"} {
          $val destroy
        }
      }
    }

    catch {$m_view destroy}
  }


  ## @brief Set the model to some user's model and synchronize
  #
  #
  # @param model  an OfflineEVBJobParams object
  #
  method setModel {model {own 0}} {
    # if we own the model, the destroy it so it doesn't hang around
    if {$options(-ownmodel)} {
      dict for {key val} $m_model {
        if {$key ne "-jobname"} {
          $val destroy
        }
      }
    }

    # Cache the new model and remember whether we own it
    set m_model $model

    $m_missing setModel $m_model $own
    $m_build   setModel [dict get $m_model -evbparams] $own
    set options(-ownmodel) $own

    $self updateViewData $m_model
  }

  ##
  #
  method setObserver {name} {
    variable m_observer 
    set m_observer $name
  }

  ## @brief Retrieve the model
  #
  # @returns string
  # @retval the name of the OfflineEVBJobParams object this controls
  method getModel {} {
    return $m_model
  }
  
  ## @brief Synchronize the data displayed by the view with the model
  #
  # @param model  an OfflineEVBJobParams object
  #
  method updateViewData {model} {

    $m_missing updateViewData $model
    $m_build   updateViewData [dict get $model -evbparams]

    $m_filelist clearTree
    $m_filelist appendFiles [[dict get $model -inputparams] cget -file]
    
    # pass the number of sources directly to the output data
    $m_view configure -nsources [[dict get $model -outputparams] cget -nsources]
  }

  ## @brief Set the values of the model to what are displayed
  #
  method commitViewDataToModel {} {
    variable m_view
    variable m_model
    variable m_missing
    variable m_build

    # update the values for the missing sources and evb 
    $m_missing commitViewDataToModel
    $m_build   commitViewDataToModel

    # set the files from the files dialogue
    [dict get $m_model -inputparams] configure -file [$m_filelist getFiles]

    # pass the number of sources directly to the output data
    [dict get $m_model -outputparams] configure -nsources [$m_view cget -nsources]
  }  

  ## Validate the data that the user provided
  #
  method validateJobOptions jobinfo {

    set errDict [dict create]

    set inputErrors  [[dict get $m_model -inputparams] validate]
    set hoistErrors  [[dict get $m_model -hoistparams] validate]
    #      set evbErrors    [[dict get $m_model -evbparams] validate]
    set outputErrors [[dict get $m_model -outputparams] validate]

    if {[llength $inputErrors]!=0} {
      dict set errDict -inputparams $inputErrors
    }
    if {[llength $hoistErrors]!=0} {
      dict set errDict -hoistparams $hoistErrors
    }
    #      if {[llength $evbErrors]!=0} {
    #         dict set jobErrDict -evbparams $evbErrors
    #      }
    if {[llength $outputErrors]!=0} {
      dict set errDict -outputparams $outputErrors
    }

    return $errDict

  }

  ## Show the error dialogue
  #
  method displayErrorGUI {errors} {
    toplevel .configerr
    set msg "Some improper configuration parameters we detected!\n"
    append msg "Please fix the errors categorized below in the tree."
    ttk::label .configerr.msg -text $msg

    set pres [ConfigErrorPresenter %AUTO% -widgetname .configerr.form]
    $pres setModel $errors 

    grid .configerr.msg -sticky nsew -padx 9 -pady 9
    grid .configerr.form -sticky nsew -padx 9 -pady 9
    grid rowconfigure .configerr 0 -weight 1
    grid columnconfigure .configerr 0 -weight 1
    wm title .configerr "Configuration Errors"

    set currgrab [grab current]

    grab .configerr
    wm transient .configerr

    if {$currgrab ne ""} {

      # prevent the user from closing out from underneath us
#      wm protocol $currgrab WM_DELETE_WINDOW { }

      wm protocol .configerr WM_DELETE_WINDOW "
      grab release .configerr ;
      grab $currgrab ;
      destroy .configerr ;
     " 
    }
  }

  ## The user wants to make this into a real job, try and do that.
  #
  method create {} {

    # commit view data 
    $self commitViewDataToModel

    set errors [$self validateJobOptions $m_model]
    if {[dict size $errors]==0} {

      # if the data is good, then let me continue
      
      # tell the observer that we want this. The observer is problably
      # whoever will store the data about this
      if {$m_observer ne ""} {
        $m_observer acceptCreation 1
      }

      # destroy this dialogue if it is ours to destroy
      if {$options(-ismaster)} {
        set widget [$m_view getWindowName]
        set top [winfo toplevel $widget]
        destroy $top
      }
    } else {

      # there were errors!, tell the user about them so they can fix
      # them
      $self displayErrorGUI [dict create "Current job" $errors]
    }
  }

  ## @brief Kill off the top level widget 
  #
  method cancel {} {
    variable m_observer
    if {$m_observer ne ""} {
      $m_observer acceptCreation 0
    }

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


  method setButtonText {text} {
    $m_view configure -buttontext $text
  }

}

# -----------------------------------------------------------------------
# -----------------------------------------------------------------------
# -----------------------------------------------------------------------


snit::widget BuildEventsWidget {
 
  option -build  -default 0
  option -window -default 1


  constructor {args} {
    $self configurelist $args

    $self buildGui
  }

  method buildGui {} {

    ttk::checkbutton $win.build -text "Enable event building" \
                                -variable [myvar options(-build)] \
                                -offvalue 0 -onvalue 1
    trace add variable [myvar options(-build)] write [mymethod onBuildChange]

    set top $win.descr
    ttk::frame $top
    if {"DescriptionFont" ni [font names]} {
      font create DescriptionFont -family Helvetica -size 10 -slant italic
    }
    tk::text $top.descrLbl -bg lightgray  -relief flat -wrap word -font DescriptionFont \
                           -height 3 -width 60
    $top.descrLbl insert end  "Check this to build events containing correlated fragments. By default, fragments are not correlated." 
    $top.descrLbl configure -state disabled

    grid $top.descrLbl -sticky nsew
    grid rowconfigure $top 0 -weight 1
    grid columnconfigure $top 0 -weight 1

    set top $win.correlate
    ttk::frame $top
    ttk::label $top.correlateLbl -text "Correlation window (ticks)"
    ttk::entry $top.correlateEntry -textvariable [myvar options(-window)]
    grid $top.correlateLbl $top.correlateEntry -row 1 -sticky new
    grid rowconfigure    $top 0     -weight 1
    grid columnconfigure $top {0 1} -weight 1

    grid $win.build -row 0 -sticky new
    
    if {[string is true $options(-build)]} {
      grid $win.correlate -row 1 -sticky new
    } else {
      grid $win.descr -row 1 -sticky new
    }

    grid columnconfigure $win 0 -weight 1
  }


  method onBuildChange {name1 name2 op} {
    if {$options(-build)} {
      grid remove $win.descr

      grid $win.correlate -row 1 -sticky new
      set options(-build) 1
    } else {
      grid remove $win.correlate

      grid $win.descr -row 1 -sticky new
      set options(-build) 0
    }
    grid columnconfigure $win 0 -weight 1
  }

 
}

snit::type BuildEventsPresenter {
 
  option -widgetname -default ""
  option -ismaster   -default 0
  option -ownmodel   -default 1

  component m_model     ;#< The model : OfflineEVBJobParams
  component m_view      ;#< The view, owned by this

  constructor {args} {
    $self configurelist $args

    puts "Checking widgetname" 
    if {$options(-widgetname) eq ""} {
      set msg    "BuildEventsPresenter -widgetname option is mandatory "
      append msg "but was not provided!"
      return -code error $msg 
    }

    puts "BuildEventsWidget ctor is next"
    set m_view [BuildEventsWidget $options(-widgetname)]

    set options(-ownmodel) 1
    puts "BuildEventsWidget ctor done"
    set m_model [EVBC::AppOptions %AUTO%]
    $self updateViewData $m_model
  }

  ##
  #
  destructor {
    if {$options(-ownmodel)} {
      $m_model destroy
    }
    catch { destroy $m_view }
  }

  ##
  #
  method setModel {model {own 0}} {
    if {$options(-ownmodel)} {
      $m_model destroy
    }
    set m_model $model

    set options(-ownmodel) $own
    $self updateViewData $m_model
  }


  method updateViewData model {

    $m_view configure -build  [$model cget -glombuild]
    $m_view configure -window [$model cget -glomdt]
  }

  method commitViewDataToModel {} {
    variable m_model
    variable m_view

    $m_model configure -glombuild  [$m_view cget -build]
    $m_model configure -glomdt     [$m_view cget -window]
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
