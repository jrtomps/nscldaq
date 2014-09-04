#!/usr/bin/env tclsh


package provide OfflineOrdererUI 11.0

package require Tk
package require snit
package require FileListWidget
package require eventLogBundle
package require evbcallouts 
package require OfflineOrderer
package require OfflineEVBInputPipelineUI
package require OfflineEVBInputPipeline
package require OfflineEVBHoistPipeline
package require evbcallouts 
package require OfflineEVBOutputPipeline 

proc TabbedOutput {win args} {
}


snit::widget OfflineOrdererView {
  
  component presenter 

  variable listWidget ""

  constructor {args} {

    set presenter [OfflineOrderer %AUTO%]
    $presenter setView $self
    $self buildGUI
  }

  destructor {

  }

  method buildGUI {} {
    variable listWidget

    # build the 
    set top [ttk::frame $win.listFrame]
    ttk::label $top.flistLbl -text "Files to Order"
    set listWidget [FileList $top.flistFrame]
    grid $top.flistLbl   -sticky ew 
    grid $top.flistFrame -sticky nsew
    grid rowconfigure    $top {0}   -weight 1
    grid columnconfigure $top {0} -weight 1

    # build the buttons for adding 
    set top [ttk::frame $win.buttons]
    ttk::button $top.run -text "Run" -command [mymethod onRun] 
    grid $top.run -sticky e
    grid columnconfigure $top 0 -weight 1

    grid $win.listFrame  -padx 9 -pady 9 -sticky ew
    grid $win.buttons    -padx 9 -pady 9 -sticky ew

    grid columnconfigure $win 0 -minsize 200 -weight 1
    grid rowconfigure $win {0 1} -weight 1

  }
  

  method onRun {} {
    variable listWidget 

    set fileList [$listWidget cget -filelist]
    $presenter run $fileList
  }

  method getPresenter {} {
    return $presenter
  }

}





#########################################################################################


#snit::widget ConfigurationDialogue {




#}

#########################################################################################


snit::widget EVBConfigDialogueView {

  option -ringname     -default "OfflineEVBOut"
  option -glomdt       -default 1
  option -glomid       -default 0
  option -glombuild    -default false

  constructor args {

    $self buildGUI 

  }

  method buildGUI {} {

    set top $win.params
    ttk::frame $top 

    ttk::label $top.ringLabel -text "Output ring"
    ttk::entry $top.ringEntry -textvariable [myvar options(-ringname)]
    ttk::label $top.dtLabel   -text "Correlation range (ticks)"
    ttk::entry $top.dtEntry -textvariable [myvar options(-glomdt)]
    ttk::label $top.idLabel   -text "Glom source ID"
    ttk::entry $top.idEntry -textvariable [myvar options(-glomid)]

    ttk::checkbutton $top.buildCheck -textvariable [myvar options(-glombuild)] \
                                  -text "Enable building" -onvalue true \
                                  -offvalue false]

    grid $top.ringLabel $top.ringEntry -padx 9 -pady 9 -sticky ew
    grid $top.dtLabel   $top.dtEntry   -padx 9 -pady 9 -sticky ew
    grid $top.idLabel   $top.idEntry   -padx 9 -pady 9 -sticky ew
    grid $top.buildCheck  -            -padx 9 -pady 9 -sticky ew

    set buttonFrame $win.buttons
    ttk::frame $buttonFrame 
    ttk::button $buttonFrame.cancel  -text "Cancel" -command [mymethod onCancel]
    ttk::button $buttonFrame.apply   -text "Apply"  -command [mymethod onApply]
    grid $buttonFrame.cancel $buttonFrame.apply -sticky ew -padx 9 -pady 9
    grid columnconfigure $buttonFrame {0 1 2} -weight 1


    grid $win.params  -padx 9 -pady 9 -sticky nsew
    grid $win.buttons -padx 9 -pady 9 -sticky ew
  }

  method onCancel {} {
  }

  method onApply {} {
  }

}

# -----------------------------------------------------------------

snit::type OfflineOrderer {
  
  option -inputparams    -default ""
  option -hoistparams    -default ""
  option -evbparams      -default "" 
  option -outputparams   -default ""

  variable evbInitialized 0
  variable currentFile    ""
  
  component view

  constructor {args} {
    # set up the defaults
    $self configure -inputparams [$self createDefaultInputParams]
    $self configure -hoistparams [$self createDefaultHoistParams]
    $self configure -evbparams [EVBC::AppOptions %AUTO% -restart false \
                                                        -gui false \
                                                        -destring OfflineEVBOut]   ;# This is an EVBC::AppOptions
    $self configure -outputparams [$self createDefaultOutputParams]

    # allow the user to override the defaults
    $self configurelist $args
  }

  destructor {
    destroy $view
  }

  method setView {theview} {
    set view $theview
  }

  method run {fileList} { 
    set processor [OfflineOrdererProcessor %AUTO%]
    $processor configure -files $fileList
    $processor configure -inputparams  $options(-inputparams)
    $processor configure -hoistparams  $options(-hoistparams)
    $processor configure -evbparams    $options(-evbparams)
    $processor configure -outputparams $options(-outputparams)

    $processor run
  } 

  method createDefaultInputParams {} {
    return [OfflineEVBInputPipeParams %AUTO%]
  }

  method createDefaultHoistParams {} {
    return [OfflineEVBHoistPipeParams %AUTO%]
  }

  method createDefaultEVBParams {} {
    return [EVBC::AppOptions %AUTO%]
  }

  method createDefaultOutputParams {} {
    return [OfflineEVBOutputPipeParams %AUTO%]
  }

} ;# end of OfflineOrderer

# ----------------------------------------------------------------------

option add *tearOff 0
menu .menu
. configure -menu .menu

set m .menu
menu $m.config
$m add command -label "Configure..." -command {launchConfigDialogue} 
#$m add cascade -menu $m.config -label "Configure"
#$m.config add command -label "Input Pipeline..."  -command {launchInputConfigDialogue}
#$m.config add command -label "Event Builder..."   -command {launchEVBDialogue}
#$m.config add command -label "Event Recording..." -command {launchEventRecordingDialoguer}



## 
#
proc launchConfigDialogue {} {
  toplevel .inconfig
  global .view 
  set presenter [.view getPresenter]
  set dialogue [InputPipelineConfigDialogue .inconfig.dialogue]
  set diaPresenter [$dialogue getPresenter]
  $diaPresenter configure -master $presenter
  $diaPresenter setParameters [$presenter cget -inputparams]
  grid .inconfig.dialogue -sticky nsew

}

OfflineOrdererView .view 
grid .view -sticky nsew
grid rowconfigure . 0 -weight 1
grid columnconfigure . 0 -weight 1
