

package provide mcfd16gui 1.0

package require mcfd16usb
package require snit
package require Tk
package require FrameSwitcher


####### Base class MCFD16View ################

snit::type MCFD16View {

  option -presenter -default {}
  variable _mcfd

  constructor {args} {
    $self configurelist $args

    $self InitArray 
  }


  method mcfd {} {
    return [$self info vars _mcfd]
  }

  method ValidateName {name} {
    return [expr [string length $name]!=0]
  }
  method ResetChannelName {widget} {
    set str [$widget cget -textvariable]
    regexp {^.*(\d+)$} $widget match ch
    set $str "Ch$ch"
  }

  method InitArray {} { 

    # set the channel valus
    for {set i 0} {$i < 16} {incr i} {

      set _mcfd(na$i) "Ch$i" 
      set _mcfd(th$i) 127
      set _mcfd(mo$i) 4 

      if {($i%2)==0} {
        set pair [expr $i/2]
        set _mcfd(po$pair) neg
        set _mcfd(ga$pair) 1
        set _mcfd(wi$pair) 100 
        set _mcfd(dt$pair) 100 
        set _mcfd(dl$pair) 1
        set _mcfd(fr$pair) 20
      }
    }

    # set the common channels
    set _mcfd(po8) neg
    set _mcfd(ga8) 1
    set _mcfd(wi8) 50
    set _mcfd(dt8) 50
    set _mcfd(dl8) 1
    set _mcfd(fr8) 20
    set _mcfd(th16) 127
  }


  method GetName {ch} { return $_mcfd(na$ch)  }
  method SetName {ch str} { set _mcfd(na$ch) $val }

  method GetThreshold {ch} {  return $_mcfd(th$ch) }
  method SetThreshold {ch val} { set _mcfd(th$ch) $val }

  method GetMonitor {ch} {  return $_mcfd(mo$ch) }
  method SetMonitor {ch onoff} {  set _mcfd(mo$ch) $onoff }

  method GetPolarity {ch} {  return $_mcfd(po$ch) }
  method SetPolarity {ch pol} {  set _mcfd(po$ch) $pol }

  method GetGain {ch} {  return $_mcfd(ga$ch) }
  method SetGain {ch gain} {  set _mcfd(ga$ch) $gain }

  method GetWidth {ch} {  return $_mcfd(wi$ch) }
  method SetWidth {ch width} {  set _mcfd(wi$ch) $width }

  method GetDeadtime {ch} {  return $_mcfd(dt$ch) }
  method SetDeadtime {ch time} {  set _mcfd(dt$ch) $time }

  method GetDelay {ch} {  return $_mcfd(dl$ch) }
  method SetDelay {ch time} {  set _mcfd(dl$ch) $time }

  method GetFraction {ch} {  return $_mcfd(fr$ch) }
  method SetFraction {ch frac} {  set _mcfd(fr$ch) $frac }

  method GetMode {} {
    set mode ""
    if {$_mcfd(mode) eq "on"} {
      set mode "common"
    } else {
      set mode "individual"
    }
    return $mode
  }

  method SetMode {mode} {
    if {$mode eq "common"} {
      set _mcfd(mode) on
    } elseif {$mode eq "individual"} {
      set _mcfd(mode) on
    } else {
      set msg {Invalid mode. Must be either common or individual.}
      return -code error MCFD16View::SetMode $msg
    }
  }

  method Commit {} {
    $options(-presenter) Commit
  }

}; # end of the View


###############################################################################
#
# MCFD16IndividualView 
#
#  A widget for controlling the MCFD16 channels individually.
#
#
snit::widget MCFD16IndividualView {

  component m_base

  delegate method * to m_base
  delegate option * to m_base

  constructor {args} {
    install m_base using MCFD16View %AUTO%

    $self configurelist $args 

    $self BuildGUI

  }

  method BuildGUI {} {

    $self BuildHeader $win.header
    $self BuildTable $win.table

    grid $win.header -sticky nsew -padx 2 -pady 2
    grid $win.table -sticky nsew -padx 2 -pady 2

    # allow the columns to resize
    grid columnconfigure $win {0} -weight 1
    grid rowconfigure $win {0} -weight 1
  }

  method BuildHeader {name} {

    set w $name
    ttk::frame $w -style "Header.TFrame"

    ttk::label $w.na -text Name -style "Header.TLabel" -padding "3 3 3 0" \
                                -width 8     
    ttk::label $w.ch -text Channel  -style "Header.TLabel" -padding "3 3 3 0" 


    ttk::label $w.po -text Polarity  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.ga -text Gain  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.th -text Threshold  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.wi -text Width -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.dt -text Deadtime  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.dl -text Delayline -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.fr -text Fraction -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.mo -text Monitor -style "Header.TLabel" -padding "3 3 3 0"

    grid $w.na $w.th $w.po $w.ga $w.wi $w.dt $w.dl $w.fr \
      -sticky sew -padx 4 -pady 4
    grid columnconfigure $w {1 2 3 4 5 6 7} -weight 1 -uniform a
    grid columnconfigure $w {0} -weight 2 -uniform a
    grid rowconfigure $w {0} -weight 1
  }

  method BuildTable {name } {
    set w $name
    ttk::frame $w 

    set rows [list]
    for {set ch 0} {$ch < 16} {incr ch 2} { 
      if {(($ch/2)%2)==0} {
        set style Even
      } else {
        set style Odd
      }

      set name "$w.rows[expr $ch/2]"
      $self BuildGroupedRows $name $ch $style
      lappend rows $name
    }

    foreach row $rows {
      grid $row -sticky nsew -padx 2 -pady 2
    }
    grid columnconfigure $w {0} -weight 1
    grid rowconfigure $w {0 1 2 3 } -weight 1
  }

  method BuildGroupedRows {name ch style} {
    set w $name
    ttk::frame $w -style $style.TFrame

    # construct first row
    ttk::entry $w.na$ch -width 8 -textvariable "[$self mcfd](na$ch)" \
                        -style "$style.TEntry" \
                                -validate focus -validatecommand [mymethod ValidateName %P] \
                                -invalidcommand [mymethod ResetChannelName %W]

    ttk::spinbox $w.th$ch -textvariable "[$self mcfd](th$ch)" -width 4 \
                        -style "$style.TSpinbox" -from 0 -to 255 \
                        -state readonly
    ttk::radiobutton $w.mo$ch -variable "[$self mcfd](monitor)" -value $ch \
                        -command Monitor -style "$style.TRadiobutton"

    set pair [expr $ch/2]
    ttk::radiobutton $w.poneg$pair -variable "[$self mcfd](po$pair)" \
                                   -value neg -text neg \
                                   -style "$style.TRadiobutton"

    ttk::spinbox $w.ga$pair -textvariable "[$self mcfd](ga$pair)" \
      -style "$style.TSpinbox" -width 4 \
      -values {1 3 10} -state readonly

    ttk::spinbox $w.wi$pair -textvariable "[$self mcfd](wi$pair)" \
      -style "$style.TSpinbox" -width 4 \
      -from 16 -to 222 -state readonly

    ttk::spinbox $w.dt$pair -textvariable "[$self mcfd](dt$pair)" \
      -style "$style.TSpinbox" -width 4 \
      -from 27 -to 222 -state readonly

    ttk::spinbox $w.dl$pair -textvariable "[$self mcfd](fr$pair)" \
      -style "$style.TSpinbox" -width 4 \
      -from 1 -to 5 -state readonly

    ttk::radiobutton $w.fr${pair}20 -text "20%" -variable "[$self mcfd](fr$pair)" \
                                    -value 20  -style "$style.TRadiobutton"

    grid $w.na$ch $w.th$ch $w.poneg$pair $w.ga$pair $w.wi$pair \
      $w.dt$pair $w.dl$pair $w.fr${pair}20 \
      -sticky news -padx 4 -pady 4
    grid configure $w.ga$pair $w.wi$pair $w.dt$pair \
                    $w.dl$pair -rowspan 2

    # construct second row 
    incr ch
    ttk::entry $w.na$ch -width 8 -textvariable "[$self mcfd](na$ch)" \
      -style "$style.TEntry" \
                                -validate focus -validatecommand [mymethod ValidateName %P] \
                                -invalidcommand [mymethod ResetChannelName %W]
    ttk::spinbox $w.th$ch -textvariable "[$self mcfd](th$ch)" -width 4 \
      -style "$style.TSpinbox" -from 0 -to 255 \
      -state readonly

    ttk::radiobutton $w.popos$pair -variable "[$self mcfd](po$pair)" \
                                   -value pos -text pos \
                                   -style "$style.TRadiobutton"
    ttk::radiobutton $w.fr${pair}40 -text "40%" -variable "[$self mcfd](fr$pair)" \
                                    -value 40  -style "$style.TRadiobutton"
    ttk::radiobutton $w.mo$ch -variable "[$self mcfd](mo$pair)" \
      -value $ch \
      -command Monitor -style "$style.TRadiobutton"
    grid $w.na$ch $w.th$ch $w.popos$pair x x x x $w.fr${pair}40 -sticky news -padx 4 -pady 4

    # allow the columns to resize
    grid columnconfigure $w {1 2 3 4 5 6 7} -weight 1 -uniform a
    grid columnconfigure $w {0} -weight 2 -uniform a
    grid rowconfigure $w {0 1} -weight 1
  }
}

###############################################################################
#
# MCFD16CommonView
#
#  A widget for controlling common channel values in MCFD16
#
snit::widget MCFD16CommonView {

  component m_base

  delegate method * to m_base
  delegate option * to m_base

  constructor {args} {
    install m_base using MCFD16View %AUTO%

    $self configurelist $args 

    $self BuildGUI
  }

  method BuildGUI {} {

    $self BuildHeader $win.header
    $self BuildCommonControls $win.common


    grid $win.header -sticky nsew -padx 2 -pady 2
    grid $win.common -sticky nsew -padx 2 -pady 2

    # allow the columns to resize
    grid columnconfigure $win {0} -weight 1
    grid rowconfigure $win {0} -weight 1
  }

  method BuildHeader {name} {

    set w $name
    ttk::frame $w -style "Header.TFrame"

    ttk::label $w.na -text Name -style "Header.TLabel" -padding "3 3 3 0" \
                                -width 8     
    ttk::label $w.ch -text Channel  -style "Header.TLabel" -padding "3 3 3 0" 


    ttk::label $w.po -text Polarity  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.ga -text Gain  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.th -text Threshold  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.wi -text Width -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.dt -text Deadtime  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.dl -text Delayline -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.fr -text Fraction -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.mo -text Monitor -style "Header.TLabel" -padding "3 3 3 0"

    grid $w.na $w.th $w.po $w.ga $w.wi $w.dt $w.dl $w.fr \
      -sticky sew -padx 4 -pady 4
    grid columnconfigure $w {1 2 3 4 5 6 7} -weight 1 -uniform a
    grid columnconfigure $w {0} -weight 2 -uniform a
    grid rowconfigure $w {0} -weight 1
  }

  method BuildCommonControls {name} {
    set w $name

    ttk::frame $w -style "Even.TFrame"

    ttk::label $w.name -text Common  -style "Even.TLabel"

    ttk::radiobutton $w.poneg8 -text "neg" -variable [$self mcfd](po8) \
                               -value neg -style "Even.TRadiobutton"
    ttk::radiobutton $w.popos8 -text "pos" -variable [$self mcfd](po8) \
                               -value pos -style "Even.TRadiobutton"
    ttk::spinbox $w.ga8 -textvariable [$self mcfd](ga8) -width 4 \
                               -values {1 3 10} -style "Even.TSpinbox" \
                               -state readonly
    ttk::spinbox $w.wi8 -textvariable [$self mcfd](wi8) -width 4 \
                               -from 16 -to 222 -style "Even.TSpinbox" \
                               -state readonly
    ttk::spinbox $w.dt8 -textvariable [$self mcfd](dt8) -width 4 -from 27 \
                               -to 222 -style "Even.TSpinbox" -state readonly
    ttk::spinbox $w.dl8 -textvariable [$self mcfd](dl8) -width 4 -from 1 \
                               -to 5 -style "Even.TSpinbox" -state readonly
    ttk::radiobutton $w.fr820 -text "20%" -variable [$self mcfd](fr8) \
                              -value 20 -style "Even.TRadiobutton"
    ttk::radiobutton $w.fr840 -text "40%" -variable [$self mcfd](fr8) -value 40 -style "Even.TRadiobutton"
    ttk::spinbox $w.th16 -textvariable [$self mcfd](th16) -width 4 -from 0 -to 255 -style "Even.TSpinbox" -state readonly

    grid $w.name $w.th16 $w.poneg8 $w.ga8 $w.wi8 $w.dt8 $w.dl8 $w.fr820 -sticky news -padx 4 -pady 4
    grid ^       ^       $w.popos8 ^      ^      ^      ^      $w.fr840 -sticky news -padx 4 -pady 4
    grid columnconfigure $w {1 2 3 4 5 6 7} -weight 1 -uniform a
    grid columnconfigure $w {0} -weight 2 -uniform a
  }
}


###############################################################################
###############################################################################
###############################################################################

# PRESENTERS For the views

snit::type MCFD16Presenter {

  option -widgetname -default ""
  option -handle -default "" 
  
  option -view -default "" 

  constructor {args} {
    $self configurelist $args
  }

  destructor {
    catch {destroy $options(-view)}
  }



  method CommitViewToParam {param index} {
    [$self cget -handle] Set$param $index [[$self cget -view] Get$param $index]
  }

  method LoopCommitView {param begin end} {
    for {set ch $begin} {$ch<$end} {incr ch} {
      $self CommitViewToParam $param $ch
    }
  }

  method UpdateParamFromView {param index} {
    [$self cget -view] Set$param $index [[$self cget -handle] Get$param $index]
  }

  method LoopUpdateView {param begin end} {
    for {set ch $begin} {$ch<$end} {incr ch} {
      $self UpdateParamFromView $param $ch
    }
  }

}


snit::type MCFD16IndividualPresenter {

  component m_base

  delegate method * to m_base
  delegate option * to m_base

  constructor {args} {
    install m_base using MCFD16Presenter %AUTO% 

    $self configurelist $args

    $self configure -view [MCFD16IndividualView [$self cget -widgetname] -presenter $self]

    $self UpdateViewFromModel
  }

  destructor {
    $m_base destroy
  }

  if {0} {
  method UpdateViewNames {} {
    # need to implement
  }
  }

  method UpdateViewFromModel {} {

    # not sure if the names is something we need to record
    #    $self updateViewNames
    $self UpdateViewThresholds 
    update
    #    $self UpdateViewMonitor
    $self UpdateViewPolarities
    update
    $self UpdateViewGains
    update
    $self UpdateViewWidths
    update
    $self UpdateViewDeadtimes
    update
    $self UpdateViewDelays
    update
    $self UpdateViewFractions
    update
  }

  method Commit {} {
    $self UpdateModelFromView
    $self UpdateViewFromModel
  }

  method UpdateModelFromView {} {

    $self CommitViewThresholds 
    update
    $self CommitViewPolarities
    update
    $self CommitViewGains
    update
    $self CommitViewWidths
    update
    $self CommitViewDeadtimes
    update
    $self CommitViewDelays
    update
    $self CommitViewFractions
  }


  method UpdateViewThresholds {} { $self LoopUpdateView Threshold 0 16 }
  method UpdateViewPolarities {} { $self LoopUpdateView Polarity 0 8 }
  method UpdateViewGains {} { $self LoopUpdateView Gain 0 8 }
  method UpdateViewWidths {} { $self LoopUpdateView Width 0 8 }
  method UpdateViewDeadtimes {} { $self LoopUpdateView Deadtime 0 8 }
  method UpdateViewDelays {} { $self LoopUpdateView Delay 0 8 }
  method UpdateViewFractions {} { $self LoopUpdateView Fraction 0 8 }

  if {0} {
  method UpdateViewMonitor {} {
    for {set ch 0} {$ch<16} {incr ch} {
      set mon [$options(-handle) MonitorEnabled $ch]
      $m_view SetMonitor $ch $thresh
    }
  }
  }

  ## Commit data to module
  method CommitViewThresholds {} { $self LoopCommitView Threshold 0 16 }
  method CommitViewPolarities {} { $self LoopCommitView Polarity 0 8 }
  method CommitViewGains {} { $self LoopCommitView Gain 0 8 }
  method CommitViewWidths {} { $self LoopCommitView Width 0 8 }
  method CommitViewDeadtimes {} { $self LoopCommitView Deadtime 0 8 }
  method CommitViewDelays {} { $self LoopCommitView Delay 0 8 }
  method CommitViewFractions {} { $self LoopCommitView Fraction 0 8 }
}


snit::type MCFD16CommonPresenter {

  component m_base

  delegate option * to m_base
  delegate method * to m_base

  constructor {args} {
    install m_base using MCFD16Presenter %AUTO% 

    $self configurelist $args

    $self configure -view [MCFD16CommonView [$self cget -widgetname] -presenter $self]

    $self UpdateViewFromModel
  }

  destructor {
    $m_base destroy
  }

  method UpdateViewFromModel {} {

    # not sure if the names is something we need to record
    #    $self updateViewNames
    $self UpdateViewThresholds 
    update
    #    $self UpdateViewMonitor
    $self UpdateViewPolarities
    update
    $self UpdateViewGains
    update
    $self UpdateViewWidths
    update
    $self UpdateViewDeadtimes
    update
    $self UpdateViewDelays
    update
    $self UpdateViewFractions
    update
  }

  method Commit {} {
    $self UpdateModelFromView
    update
    $self UpdateViewFromModel
  }

  method UpdateModelFromView {} {
    # make sure the mode is set first
    # becuase subsequent writes may depend on it

    $self CommitViewThresholds 
    update
    $self CommitViewPolarities
    update
    $self CommitViewGains
    update
    $self CommitViewWidths
    update
    $self CommitViewDeadtimes
    update
    $self CommitViewDelays
    update
    $self CommitViewFractions
  }

  if {0} {
  method UpdateViewNames {} {
    # need to implement
  }
  }

  method UpdateViewThresholds {} { $self UpdateParamFromView Threshold 16 }

  if {0} {
  method UpdateViewMonitor {} {
    for {set ch 0} {$ch<16} {incr ch} {
      set mon [$options(-handle) MonitorEnabled $ch]
      $m_view SetMonitor $ch $thresh
    }
  }
  }

  method UpdateViewPolarities {} { $self UpdateParamFromView Polarity 8 }
  method UpdateViewGains {} { $self UpdateParamFromView Gain 8 }
  method UpdateViewWidths {} { $self UpdateParamFromView Width 8 }
  method UpdateViewDeadtimes {} { $self UpdateParamFromView Deadtime 8 }
  method UpdateViewDelays {} { $self UpdateParamFromView Delay 8 }
  method UpdateViewFractions {} { $self UpdateParamFromView Fraction 8 }


  ## Commit data to module
  method CommitViewThresholds {} { $self CommitViewToParam Threshold 16 }
  method CommitViewPolarities {} { $self CommitViewToParam Polarity 8 }
  method CommitViewGains {} { $self CommitViewToParam Gain 8 }
  method CommitViewWidths {} { $self CommitViewToParam Width 8 }
  method CommitViewDeadtimes {} { $self CommitViewToParam Deadtime 8 }
  method CommitViewDelays {} { $self CommitViewToParam Delay 8 }
  method CommitViewFractions {} { $self CommitViewToParam Fraction 8 }
}


########## Unified Controls ####################
snit::widget MCFD16ControlPanel {

  component m_frames
  component m_comPrsntr
  component m_indPrsntr

  option -handle -default {}
  variable m_mode 
  variable m_current

  constructor {args} {
    $self configurelist $args

    if {[$self cget -handle] eq ""} {
      return -code error "MCFD16ControlPanel must be provided -handle option."
    }

    install m_frames using FrameSwitcher $win.frames
    install m_comPrsntr using \
      MCFD16CommonPresenter %AUTO% -widgetname $win.com \
                                   -handle [$self cget -handle]

    install m_indPrsntr using \
      MCFD16IndividualPresenter %AUTO% -widgetname $win.ind \
                                   -handle [$self cget -handle]

    puts [$self cget -handle]
    set m_mode [[$self cget -handle] GetMode]
    puts $m_mode
    trace add variable [myvar m_mode] write [mymethod OnModeChange]

    $self BuildGUI

    # sets the m_current and updates the frame switcher
    $self OnModeChange {} {} {}

  }

  method BuildGUI {} {
    # add the frames
    $m_frames add common $win.com
    $m_frames add individual $win.ind

    ttk::label $win.modeLbl -text "Configuration Mode"
    ttk::radiobutton $win.modeCom -text "common" -variable [myvar m_mode] \
                                  -value common
    ttk::radiobutton $win.modeInd -text "individual" -variable [myvar m_mode] \
                                  -value individual

    ttk::button $win.commit -text "Commit" -command [mymethod Commit] \
                                  -style "Commit.TButton"

    grid $win.modeLbl $win.modeCom $win.modeInd -sticky new
    grid $m_frames - -  -sticky nsew

    grid $win.commit - -  -sticky ew

    grid rowconfigure $win 1 -weight 1
    grid columnconfigure $win {0 1 2} -weight 1
  }


  method OnModeChange {name0 name1 op} {
    if {$m_mode eq "common"} {
      $m_frames select common
      set m_current $m_comPrsntr
    } else {
      $m_frames select individual 
      set m_current $m_indPrsntr
    } 
  }

  method Commit {} {
    [$self cget -handle] SetMode $m_mode
    $m_current Commit
  }

  method GetCurrent {} { return $m_current }
}

#######
#

snit::widget ProtocolSelector {
  
}

################## GLOBAL STUFF #############################################
#

proc ConfigureStyle {} {
  ttk::style configure "Title.TLabel" -foreground "firebrick" \
                                      -font "helvetica 28 bold"

  ttk::style configure Header.TLabel -background {orange red} 
  ttk::style configure Header.TFrame -background {orange red}

  ttk::style configure Even.TEntry -background snow3
  ttk::style configure Even.TRadiobutton -background snow3
  ttk::style configure Even.TSpinbox -background snow3
  ttk::style configure Even.TFrame -background snow3
  ttk::style configure Even.TLabel -background snow3

  ttk::style configure Odd.TEntry -background snow3
  ttk::style configure Odd.TRadiobutton -background snow3
  ttk::style configure Odd.TSpinbox -background snow3
  ttk::style configure Odd.TFrame -background snow3
  ttk::style configure Odd.TLabel -background snow3

  ttk::style configure Commit.TButton -background orange
}

ConfigureStyle

ttk::label .title -text "MCFD-16 Controls" -style "Title.TLabel"
ttk::frame .info -style "Info.TFrame"
ttk::label .info.fwVsnLbl -text "Firmware version:"
ttk::label .info.swVsnLbl -text "Software version:"
ttk::label .info.protoLbl -text "Protocol:"

grid .info.fwVsnLbl .info.protoLbl -sticky nsew
grid .info.swVsnLbl x             -sticky nsew
grid columnconfigure .info {0 1} -weight 1
#grid rowconfigure .info {2} -weight 1



MCFD16USB dev /dev/ttyUSB0

set control [MCFD16ControlPanel .ctl -handle ::dev]

grid .title -sticky nsew -padx 8 -pady 8
grid .info -sticky nsew -padx 8 -pady 8
grid .ctl -sticky nsew -padx 8 -pady 8

grid rowconfigure . {2} -weight 1
grid columnconfigure . 0 -weight 1

