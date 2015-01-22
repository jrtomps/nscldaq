#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#      NSCLDAQ Development Group
#
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

# This software is Copyright by the Board of Trustees of Michigan
# State University (c) Copyright 2297884. 

# @file  JobConfigUI.tcl 
# @author Jeromy Tompkins 

#########################################################################################################
#

package provide OfflineEVBJobConfigUI 11.0

package require FileListWidget
package require OfflineEVBMissingSourceUI

package require OfflineEVBInputPipeline
package require OfflineEVBHoistPipeline
package require evbcallouts
package require OfflineEVBOutputPipeline

package require snit
package require Tk



## @defgroup jobconfigui  The OfflineEVBJobConfigUI Package
#
# This package follows the Model-View-Presenter pattern for implementing a
# dialogue users can edit existing or new Jobs with. The JobConfigUI is intended
# to provide a simplified configuration utility that abstracts the details of
# all the various independent parameters in the pipeline. Instead, the user is
# presented with just the necessary parameters that they expect to control. Many
# of the parameters controlled in the JobConfigUI have meaning in the different
# pipelines. This dialogue actually binds together the pipeline in such a way
# that prevents the user from screwing up the instantiation of the 4 pipelines.
# That isn't to say that the user can't select poor parameter values that
# cause the processing to fail. 
#
# The JobConfigUI package operates on the idea of a Job. A Job is a dict
# with the following structure:
#
# key               value type
# -jobname          string
# -inputparams      OfflineEVBInputPipeParams
# -hoistparams      OfflineEVBHoistPipeParams
# -evbparams        EVBC::AppOptions
# -outputparams     OfflineEVBOutputPipeParams
#
# There two pieces to the package: the view and the presenter. The view is made
# up of the JobConfigUIView type. It is once again a humble view that merely
# presents information and forwards events to its presenter to handle. The
# presenter portion of the package is the JobConfigUIPresenter. this is in
# charge of providing the logic for the package. It alone can manipulate the
# model (ie. the job it is configuring) and dictates what the view displays
# to the user via synchronization. Any software that uses this package should
# instantiate a presenter and interact wit that alone rather than interacting
# with the view directly. 
#



## @brief The view in the JobConfigUI package
#
# @ingroup jobconfigui
#
# This is the megawidget that the user will ultimately interact with. It is a
# megawidget whose hull is a ttk::frame. Other software should not directly
# instantiate one of these unless it knows what it is doing because these are
# only useful when attached to a JobConfigUIPresenter.
#
# There are a few useful options that can provided to configure the way that
# this appears to the user:
# 
# -missingwidget    a MissingSourceUIView object
# -buildwidget      a BuildEventsWidget object
# -showbuttons      boolean value indicating whether to show buttons or not
#                   (basically this determines whether this is its own master)
# -buttontext       the text to put on the "Accept" button
#
# Otherwise, the other options contain the display state of view. These are the
# value of the data that could be written into the model if the presenter
# decides to synchronize to the view state.
snit::widget JobConfigUIView {

  
  option -nsources        -default 2        ;#< number of end runs to expect
  option -jobname         -default "Job"    ;#< name of job (not used)
  option -missingwidget   -default ""       ;#< name of missing source widget
  option -buildwidget     -default ""       ;#< name of buildevents widget

  option -showbuttons     -default 1        ;#< show buttons or not? 
  option -buttontext      -default "Create" ;#< Label to put on button

  component m_presenter                     ;#< The presenter
  variable m_fileTree                       ;#< FileList widget name

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

  ## @brief Destroy this thing 
  #
  destructor {
    catch {destroy $win}
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
    FileList $m_fileTree -sort 1
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

  ## @brief Grid the missing sources widget
  # @param name name of the widget
  method gridMissingWidget {name} {
    variable m_paramFrame
    grid $name - -row 1 -sticky new -pady 9 -in $m_paramFrame
  }

  ## @brief Grid the build event widget
  # @param name of the widget
  method gridBuildWidget {name} {
    variable m_paramFrame
    grid $name -  -row 2 -sticky new -in $m_paramFrame
  }

  ## @brief Forward button press event to presenter
  #
  # This does not check whether the presenter exists. It is assumed that the
  # user has already set this up.
  method onCancel {} {
    $m_presenter cancel
  }

  ## @brief Forward button press event to presenter
  #
  # This does not check whether the presenter exists. It is assumed that the
  # user has already set this up.
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
  # @return the name of the hull
  method getWindowName {} {return $win}


  ## @brief Return the name of the file list 
  # 
  # The FileList is not currently implemented as an MVP paradigm and so this 
  # is really just returning the name of the megawidget itself. This could
  # change in the future though...
  #
  # @returns name of the object in control of the filelist widget
  method getFileListPresenter {} {
    return $m_fileTree
  }
}

# End of JobConfigUIView code

# ------------------------------------------------------------------------------
################################################################################
################################################################################
################################################################################
################################################################################
################################################################################
# ------------------------------------------------------------------------------

## @brief The presenter component of the JobConfigUI package.
#
# @ingroup jobconfigui
#
# This object controls the model the is ultimately being configured by this
# package. It also owns a view that is associated with it. So the presenter is
# the sole object that can manipulate the model and it is therefore responsible
# for synchronizing the state of the view with state of the model and vice
# versa. It is not necessarily the owner of the model that it controls and
# typically operates on models that have been leased to it. The idea is that
# this is simply a manipulator of something given to it. 
#
#
snit::type JobConfigUIPresenter {

  option -widgetname -default ""      ;#< name of view
  option -ismaster   -default 0       ;#< forward events to a parent?
  option -ownmodel   -default 1       ;#< whether to delete model

  variable m_model     ;#< The model  a job 
  variable m_view      ;#< The view, owned by this
  variable m_missing   ;#< The missing source presenter 
  variable m_build     ;#< The build event widgetpresenter 

  variable m_filelist ;#< the view's file list widget

  variable m_observer  ;#< observes whether this is to be accepted

  ## Construct the model, view, and synchronize view
  #
  # It is necessary that the user provides the name of the widget. If that is
  # not provided this cries, "Uncle!" The presenter is tightly bound to the view
  # and actually owns it. It passes the view itself on construction so that there is never
  # any confusion about who the view should deal with.
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
      Job::destroy $m_model
    }

    catch {$m_view destroy}
  }


  ## @brief Set the model to some user's model and synchronize
  #
  # There a bunch of things that happen when a new model is provided.
  # 1. Destroy the model if we own it
  # 2. Store the new model and record whether we own it
  # 3. Pass the new model to the MissingSourceUI presenter
  # 4. Pass the new model to the BuildEventsUI presenter
  #
  # @param model  a job
  #
  method setModel {model {own 0}} {
    # if we own the model, the destroy it so it doesn't hang around
    if {$options(-ownmodel)} {
      Job::destroy $m_model
    }

    # Cache the new model and remember whether we own it
    set m_model $model
    set options(-ownmodel) $own

    # update the missing source and build event presenter's models too
    $m_missing setModel $m_model $own
    $m_build   setModel [dict get $m_model -evbparams] $own

    # sync the view to the model
    $self updateViewData $m_model
  }

  ## @brief Add an object to receive info when events happen
  #
  # This is useful for indicating whether or not to accept changes 
  # to the model or to discard them.
  method setObserver {name} {
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

    # build the display info from scratch for simplicity 
    $m_filelist clearTree
    $m_filelist appendFiles [[dict get $model -inputparams] cget -file]
    
    # pass the number of sources directly to the output data
    $m_view configure -nsources [[dict get $model -outputparams] cget -nsources]
  }

  ## @brief Set the values of the model to what are displayed
  #
  # Synchronize the model to the view. This is the only way that we manipulate 
  # the model.
  method commitViewDataToModel {} {

    # update the values for the missing sources and evb 
    $m_missing commitViewDataToModel
    $m_build   commitViewDataToModel

    # set the files from the files dialogue
    [dict get $m_model -inputparams] configure -file [$m_filelist getFiles]

    # pass the number of sources directly to the output data
    [dict get $m_model -outputparams] configure -nsources [$m_view cget -nsources]
  }  

  ## @brief Validate the data that the user provided
  #
  # Ensure that the parts of the job that are actually controlled by this are
  # being set to valid values. If they are not, then this will list all of the
  # reasons why it thinks they are bad. 
  #
  # @result dict of error messages indexed by type of parameter set
  method validateJobOptions jobinfo {


    # Create a dictionary to fill with error messages
    set errDict [dict create]

    set inputErrors  [[dict get $m_model -inputparams] validate]
    set hoistErrors  [[dict get $m_model -hoistparams] validate]
    #      set evbErrors    [[dict get $m_model -evbparams] validate]
    set outputErrors [[dict get $m_model -outputparams] validate]

    # Only create a key for the parameter set if there are errors to report
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

  ## @brief Show the error dialogue
  #
  # This is a modal dialogue that steals the focus from the JobConfigUI.
  # However, when this dialogue is destroyed, the JobConfigUI will gain the 
  # focus again as a model dialogue. The idea of this dialogue is to 
  # lay out the error messages to the user is a very easy to understand fashion.
  #
  method displayErrorGUI {errors} {

    # create the top level window
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

    # steal the grab 
    grab .configerr
    wm transient .configerr

    if {$currgrab ne ""} {

      # schedule the JobConfigUI to receive grab when this is destroyed
      wm protocol .configerr WM_DELETE_WINDOW "
      grab release .configerr ;
      grab $currgrab ;
      destroy .configerr ;
     " 
    }
  }

  ## @brief The user wants to make this into a real job, try and do that.
  #
  # Synchronizes the view data with the widget and then checks to see if it is 
  # a reasonalbe set of parameters by validating it. If the validation discovers
  # errors, then the error dialogue is launched. Otherwise, this informs the
  # observer that this should be created and tries to destroy the window if it
  # is owned by this.
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
  # Signal to the oberver that this is not worth keeping and then
  # destroy the view if there is not master that controls this.
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

  ## @brief Changes the displyed text on the "accept" button
  method setButtonText {text} {
    $m_view configure -buttontext $text
  }

}

#------------------------------------------------------------------------------
###############################################################################
###############################################################################
###############################################################################
###############################################################################
###############################################################################
#------------------------------------------------------------------------------

## @brief A package for managing the build events widget in the JobConfigUI
#
# @defgroup buildeventsui Build Events Widget
#
# This is implemented with the Model-View-Presenter paradigm and this there are
# two separate parts to it: the view and the presenter. The view is defined in
# the BuildEventsWidget and the presenter is defined in the
# BuildEventsPreseneter. Together these manipulate some select features of the 
# event builder pipeline and the hoist pipeline.
#
#

## @brief the View for the Build Events Widget
# 
# @ingroup buildeventsui
#
# This provides a widgets that attempts to hide some complexity from the user.
# The user will always see a checkbutton that determines whether or not to
# enable correlation of data into built events. If the checkbutton is not
# selected an info text is displayed that describes what the checkbutton is
# aimed at. If the checkbutton is selected, then a text entry is provided that
# allows the user to select the correlation window in glom.
#
snit::widget BuildEventsWidget {
 
  option -build  -default 0         ;#< whether to build data or not
  option -window -default 1         ;#< default glomdt


  ## @brief Constructor
  #
  # @param args   option-value pairs
  #
  constructor {args} {
    $self configurelist $args

    $self buildGui
  }

  ## @brief Build the megawidget
  #
  # This merely assembles and grid the components that make up the megawidget
  #
  #
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
    set descrText "Check this to build events containing correlated fragments. "
    append descrText "By default, fragments are not correlated." 
    $top.descrLbl insert end $descrText
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


  ## @brief Callback for when the checkbutton selection changes
  #
  # If the checkbutton is selected, then this will show the extra text
  # entry widget. If it is unselected, the description text is displayed.
  #
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



## @brief The presenter for the BuildEventsWidget
#
# @ingroup buildeventsui
#
# This is the presenter that manages the model and the view for the build events
# widget. It is responsible for synchronizing the view with the model and vice
# versa. 
#
snit::type BuildEventsPresenter {
 
  option -widgetname -default "" ;#< name of the view widget
  option -ismaster   -default 0  ;#< whether this answers to a higher entity
  option -ownmodel   -default 1  ;#< can it destroy its model?

  component m_model     ;#< The model: EVBC::AppOptions
  component m_view      ;#< The view, owned by this


  ## @brief Constructor 
  #
  # It is demanded that the user provide the name of the view via the
  # -widgetname option. This will create the view and also a default
  # EVBC::AppOptions object to use as the model. It therefore owns the model
  # initially. The view is brought into sync with the model by the end of this.
  # 
  # @params args  option-value pairs
  #
  constructor {args} {
    $self configurelist $args

    if {$options(-widgetname) eq ""} {
      set msg    "BuildEventsPresenter -widgetname option is mandatory "
      append msg "but was not provided!"
      return -code error $msg 
    }

    set m_view [BuildEventsWidget $options(-widgetname)]

    set options(-ownmodel) 1
    set m_model [EVBC::AppOptions %AUTO%]
    $self updateViewData $m_model
  }

  ## @brief Destroy the model and view if possible
  #
  destructor {
    if {$options(-ownmodel)} {
      $m_model destroy
    }
    catch { destroy $m_view }
  }

  ## @brief Replace the model with a new one and sync
  #
  # The previous model is destroyed if this object owns it. Otherwise,
  # it is simply forgotten. The new model is then used to update the view data.
  #
  # @param model  a EVBC::AppOptions object
  # @param own    boolean for whether this can destroy the model
  method setModel {model {own 0}} {
    if {$options(-ownmodel)} {
      $m_model destroy
    }
    set m_model $model

    set options(-ownmodel) $own
    $self updateViewData $m_model
  }


  ## @brief Synchronize the view data to the model
  #
  # @param model  the model to sync data to
  #
  method updateViewData model {

    $m_view configure -build  [$model cget -glombuild]
    $m_view configure -window [$model cget -glomdt]
  }

  ## @brief Sync the model to the view data
  #
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

  ## @brief Kill off the top level widget if possible
  #
  #  If the presenter is set as the master of itself, then it can close the
  # toplevel window that holds it. This is useful if the view will live in a
  # dialogue all by itself. Otherwise, can is just a dummy button. 
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
