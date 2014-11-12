

package provide mcfd16gui 1.0

package require mcfd16usb
package require snit
package require Tk


snit::widget MCFD16View {

  variable _mcfd
  variable _presenter

  constructor {presenter args} {
    set _presenter $presenter

    $self InitArray 
    $self BuildGUI

    $self ConfigureStyle
  }

  method BuildGUI {} {

    $self BuildHeader $win.header
    $self BuildTable $win.table
    $self BuildCommonControls $win.common


    ttk::button $win.commit -text "Commit" -command [mymethod Commit]


    grid $win.header -sticky nsew -padx 2 -pady 2
    grid $win.table -sticky nsew -padx 2 -pady 2
    grid $win.common -sticky nsew -padx 2 -pady 2
    grid $win.commit -sticky sew -columnspan 8

    # allow the columns to resize
    grid columnconfigure $win {0} -weight 1
    grid rowconfigure $win {0} -weight 1
  }

  method BuildHeader {name} {

    set w $name
    ttk::frame $w

    ttk::label $w.na -text Name -style "Header.TLabel" -padding "3 3 3 0"
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
      -sticky news -pady {4 2}
    grid columnconfigure $w {0 1 2 3 4 5 6 7 8} -weight 1
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

  method BuildCommonControls {name} {
    set w $name

    ttk::frame $w

    ttk::checkbutton $w.mode -text Common -variable _mcfd(mode) -onvalue off \
      -offvalue on

    ttk::spinbox $w.po8 -textvariable [myvar _mcfd(po8)] -width 4 -values {neg pos}
    ttk::spinbox $w.ga8 -textvariable [myvar _mcfd(ga8)] -width 4 -values {1 3 10}
    ttk::spinbox $w.wi8 -textvariable [myvar _mcfd(wi8)] -width 4 -from 16 -to 222
    ttk::spinbox $w.dt8 -textvariable [myvar _mcfd(dt8)] -width 4 -from 27 -to 222
    ttk::spinbox $w.dl8 -textvariable [myvar _mcfd(dl8)] -width 4 -from 1 -to 5
    ttk::spinbox $w.fr8 -textvariable [myvar _mcfd(fr8)] -width 4 -values {20 40}
    ttk::spinbox $w.th16 -textvariable [myvar _mcfd(th16)] -width 4 -from 0 -to 255

    grid $w.mode $w.po8 $w.ga8 $w.wi8 $w.dt8 $w.dl8 $w.fr8 $w.th16 -sticky news -padx 2 -pady 2
    grid columnconfigure $w {0 1 2 3 4 5 6 7 8 9} -weight 1
  }


  method BuildGroupedRows {name ch style} {
    set w $name
    ttk::frame $w -style $style.TFrame

    # construct first row
    ttk::entry $w.na$ch -width 8 -textvariable [myvar _mcfd(na$ch)] \
                        -style "$style.TEntry" 
    ttk::spinbox $w.th$ch -textvariable [myvar _mcfd(th$ch)] -width 4 \
                        -style "$style.TSpinbox" -from 0 -to 255 \
                        -state readonly
    ttk::radiobutton $w.mo$ch -variable [myvar _mcfd(monitor)] -value $ch \
                        -command Monitor -style "$style.TRadiobutton"

    set pair [expr $ch/2]
    ttk::spinbox $w.po$pair -textvariable [myvar _mcfd(po$pair)] \
      -style "$style.TSpinbox" -width 4 \
      -values {neg pos} -state readonly

    ttk::spinbox $w.ga$pair -textvariable [myvar _mcfd(ga$pair)] \
      -style "$style.TSpinbox" -width 4 \
      -values {1 3 10} -state readonly

    ttk::spinbox $w.wi$pair -textvariable [myvar _mcfd(wi$pair)] \
      -style "$style.TSpinbox" -width 4 \
      -from 16 -to 222 -state readonly

    ttk::spinbox $w.dt$pair -textvariable [myvar _mcfd(dt$pair)] \
      -style "$style.TSpinbox" -width 4 \
      -from 27 -to 222 -state readonly

    ttk::spinbox $w.dl$pair -textvariable [myvar _mcfd(dl$pair)] \
      -style "$style.TSpinbox" -width 4 \
      -from 1 -to 5 -state readonly

    ttk::spinbox $w.fr$pair -textvariable [myvar _mcfd(fr$pair)] \
      -style "$style.TSpinbox" -width 4 \
      -values {20 40} -state readonly

    grid $w.na$ch $w.th$ch $w.po$pair $w.ga$pair $w.wi$pair \
      $w.dt$pair $w.dl$pair $w.fr$pair \
      -sticky news -padx 2 -pady {6 2}
    grid configure $w.po$pair $w.ga$pair $w.wi$pair $w.dt$pair \
                    $w.dl$pair $w.fr$pair -rowspan 2

    # construct second row 
    incr ch
    ttk::entry $w.na$ch -width 8 -textvariable [myvar _mcfd(na$ch)] \
      -style "$style.TEntry" 
    ttk::spinbox $w.th$ch -textvariable [myvar _mcfd(th$ch)] -width 4 \
      -style "$style.TSpinbox" -from 0 -to 255 \
      -state readonly
    ttk::radiobutton $w.mo$ch -variable [myvar _mcfd(monitor)] -value $ch \
      -command Monitor -style "$style.TRadiobutton"
    grid $w.na$ch $w.th$ch x x x x x x -sticky news -padx 2 -pady {2 6}

    # allow the columns to resize
    grid columnconfigure $w {0 1 2 3 4 5 6 7 8} -weight 1
    grid rowconfigure $w {0 1} -weight 1
  }

  method ConfigureStyle {} {
    ttk::style configure Header.TLabel -background pink

    ttk::style configure Even.TEntry -background lightgreen
    ttk::style configure Even.TRadiobutton -background lightgreen
    ttk::style configure Even.TSpinbox -background lightgreen
    ttk::style configure Even.TFrame -background lightgreen

    ttk::style configure Odd.TEntry -background lightyellow
    ttk::style configure Odd.TRadiobutton -background lightyellow
    ttk::style configure Odd.TSpinbox -background lightyellow
    ttk::style configure Odd.TFrame -background lightyellow
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


  method GetName {ch} {
    return $_mcfd(na$ch)
  }

  method SetName {ch str} {
    set _mcfd(na$ch) $val
  }

  method GetThreshold {ch} {
    return $_mcfd(th$ch)
  }

  method SetThreshold {ch val} {
    set _mcfd(th$ch) $val
  }

  method GetMonitor {ch} {
    return $_mcfd(mo$ch)
  }

  method SetMonitor {ch onoff} {
    set _mcfd(mo$ch) $onoff
  }

  method GetPolarity {ch} {
    return $_mcfd(po$ch)
  }

  method SetPolarity {ch pol} {
    set _mcfd(po$ch) $pol
  }

  method GetGain {ch} {
    return $_mcfd(ga$ch)
  }

  method SetGain {ch gain} {
    set _mcfd(ga$ch) $gain
  }

  method GetWidth {ch} {
    return $_mcfd(wi$ch)
  }

  method SetWidth {ch width} {
    set _mcfd(wi$ch) $width
  }

  method GetDeadtime {ch} {
    return $_mcfd(dt$ch)
  }

  method SetDeadtime {ch time} {
    set _mcfd(dt$ch) $time
  }

  method GetDelay {ch} {
    return $_mcfd(dl$ch)
  }

  method SetDelay {ch time} {
    set _mcfd(dl$ch) $time
  }

  method GetFraction {ch} {
    return $_mcfd(fr$ch)
  }

  method SetFraction {ch frac} {
    set _mcfd(fr$ch) $frac
  }

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
    $_presenter Commit
  }

}; # end of the View



###############################################################################
###############################################################################
###############################################################################



snit::type MCFD16Presenter {

  option -widgetname -default ""

  variable m_handle
  variable m_view

  constructor {args} {
    $self configurelist $args
    
    set m_handle [MCFD16USB %AUTO% /dev/ttyUSB0]

    if {$options(-widgetname) eq ""} {
      set msg "MCFD16Presenter::constructor -widgetname must be provided."
      return -code error $msg 
    }

    set m_view [MCFD16View $options(-widgetname) $self]

    $self UpdateViewFromModel
  }

  destructor {

    destroy $m_view
  }

  method GetComHandle {} {
    return $m_handle
  }

  method UpdateViewFromModel {} {

    $m_view SetMode [$m_handle GetMode]

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
    set mode [$m_view GetMode]
    $m_handle SetMode $mode

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

  method UpdateViewThresholds {} {
    for {set ch 0} {$ch<=16} {incr ch} {
      set thresh [$m_handle GetThreshold $ch]
      $m_view SetThreshold $ch $thresh
    }
  }

  if {0} {
  method UpdateViewMonitor {} {
    for {set ch 0} {$ch<16} {incr ch} {
      set mon [$m_handle MonitorEnabled $ch]
      $m_view SetMonitor $ch $thresh
    }
  }
  }

  method UpdateViewPolarities {} {
    for {set ch 0} {$ch<=8} {incr ch} {
      set pol [$m_handle GetPolarity $ch]
      $m_view SetPolarity $ch $pol
    }
  }

  method UpdateViewGains {} {
    for {set ch 0} {$ch<=8} {incr ch} {
      set gain [$m_handle GetGain $ch]
      $m_view SetGain $ch $gain
    }
  }

  method UpdateViewWidths {} {
    for {set ch 0} {$ch<=8} {incr ch} {
      set val [$m_handle GetWidth $ch]
      $m_view SetWidth $ch $val
    }
  }

  method UpdateViewDeadtimes {} {
    for {set ch 0} {$ch<=8} {incr ch} {
      set val [$m_handle GetDeadtime $ch]
      $m_view SetDeadtime $ch $val
    }
  }

  method UpdateViewDelays {} {
    for {set ch 0} {$ch<=8} {incr ch} {
      set val [$m_handle GetDelay $ch]
      $m_view SetDelay $ch $val
    }
  }

  method UpdateViewFractions {} {
    for {set ch 0} {$ch<=8} {incr ch} {
      set fr [$m_handle GetFraction $ch]
      $m_view SetFraction $ch $fr
    }
  }


  method CommitViewThresholds {} {
    for {set ch 0} {$ch<16} {incr ch} {
      set thresh [$m_view GetThreshold $ch]
      $m_handle SetThreshold $ch $thresh
    }
  }

  method CommitViewPolarities {} {
    for {set pair 0} {$pair<8} {incr pair} {
      set pol [$m_view GetPolarity $pair]
      $m_handle SetPolarity $pair $pol
    }
  }

  method CommitViewGains {} {
    for {set pair 0} {$pair<8} {incr pair} {
      set val [$m_view GetGain $pair]
      $m_handle SetGain $pair $val
    }
  }

  method CommitViewWidths {} {
    for {set pair 0} {$pair<8} {incr pair} {
      set val [$m_view GetWidth $pair]
      $m_handle SetWidth $pair $val
    }
  }

  method CommitViewDeadtimes {} {
    for {set pair 0} {$pair<8} {incr pair} {
      set val [$m_view GetDeadtime $pair]
      $m_handle SetDeadtime $pair $val
    }
  }

  method CommitViewDelays {} {
    for {set pair 0} {$pair<8} {incr pair} {
      set val [$m_view GetDelay $pair]
      $m_handle SetDelay $pair $val
    }
  }
  
  method CommitViewFractions {} {
    for {set pair 0} {$pair<8} {incr pair} {
      set val [$m_view GetFraction $pair]
      $m_handle SetFraction $pair $val
    }
  }
}




set pres [MCFD16Presenter %AUTO% -widgetname .view]
proc print {dir} {
  puts "$dir"
}

set m 123
#Arrows .view -background pink -textvariable ::m -upcommand [list print up]


grid .view -sticky nsew

grid rowconfigure . 0 -weight 1
grid columnconfigure . 0 -weight 1

