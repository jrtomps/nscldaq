

package provide mcfd16_view 1.0

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

    set w $win
    ttk::checkbutton $w.remote -text Remote -variable [myvar mcfd(remote)] \
      -onvalue on -offvalue off \
      -command [mymethod RemoteLocal] 

    ttk::button $w.exit -text Exit -command Exit 

    set w $win.table
    ttk::frame $w

    ttk::label $w.na -text Name -style "Header.TLabel" -padding "3 0 3 0"
    ttk::label $w.ch -text Channel  -style "Header.TLabel" -padding "3 0 3 0"
    ttk::label $w.po -text Polarity  -style "Header.TLabel" -padding "3 0 3 0"
    ttk::label $w.ga -text Gain  -style "Header.TLabel" -padding "3 0 3 0"
    ttk::label $w.th -text Threshold  -style "Header.TLabel" -padding "3 0 3 0"
    ttk::label $w.wi -text Width -style "Header.TLabel" -padding "3 0 3 0"
    ttk::label $w.dt -text Deadtime  -style "Header.TLabel" -padding "3 0 3 0"
    ttk::label $w.dl -text Delayline -style "Header.TLabel" -padding "3 0 3 0"
    ttk::label $w.fr -text Fraction -style "Header.TLabel" -padding "3 0 3 0"
    ttk::label $w.mo -text Monitor -style "Header.TLabel" -padding "3 0 3 0"

    grid $w.na $w.th $w.po $w.ga $w.wi $w.dt $w.dl $w.fr $w.mo \
      -sticky news -pady {4 2}

    for {set ch 0} {$ch < 16} {incr ch} { 
      set row [expr $ch+1]
      set style Even
      set cc lightgreen

      ttk::entry $w.na$ch -width 8 -textvariable [myvar _mcfd(na$ch)] \
        -style "$style.TEntry" 
      ttk::spinbox $w.th$ch -textvariable [myvar _mcfd(th$ch)] -width 4 \
        -style "$style.TSpinbox" -from 0 -to 255 \
        -state readonly
      ttk::radiobutton $w.mo$ch -variable [myvar _mcfd(monitor)] -value $ch \
        -command Monitor -style "$style.TRadiobutton"

      if {($ch%2)==0} {
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
          $w.dt$pair $w.dl$pair $w.fr$pair $w.mo$ch \
          -sticky news -padx 2 -pady {4 2}
      } else {
        grid $w.na$ch $w.th$ch x x x x x x $w.mo$ch -sticky news \
          -padx 2 -pady {2 4}
        ttk::separator $w.sep$ch -orient horizontal
        grid $w.sep$ch -columnspan 9 -sticky nsew
      }
    }
    for {set pair 0} {$pair<8} {incr pair 1} {
      grid configure $w.po$pair $w.ga$pair $w.wi$pair $w.dt$pair $w.dl$pair $w.fr$pair -rowspan 2
    }

    ttk::checkbutton $w.mode -text Common -variable _mcfd(mode) -onvalue off \
      -offvalue on \
      -command CommonMode;

    ttk::spinbox $w.poc -textvariable [myvar _mcfd(poc)] -width 4
    ttk::spinbox $w.gac -textvariable [myvar _mcfd(gac)] -width 4
    ttk::spinbox $w.wic -textvariable [myvar _mcfd(wic)] -width 4
    ttk::spinbox $w.dtc -textvariable [myvar _mcfd(dtc)] -width 4
    ttk::spinbox $w.dlc -textvariable [myvar _mcfd(dlc)] -width 4
    ttk::spinbox $w.frc -textvariable [myvar _mcfd(frc)] -width 4
    ttk::spinbox $w.thc -textvariable [myvar _mcfd(thc)] -width 4

    grid $w.mode $w.poc $w.gac $w.wic $w.dtc $w.dlc $w.frc $w.thc x -sticky news -padx 2 -pady 2

    grid $w -sticky nsew
    # allow the columns to resize
    grid columnconfigure $w {0 1 2 3 4 5 6 7 8} -weight 1
    grid rowconfigure $w {1 2 4 5 7 8 10 11 13 14 16 17 19 20 22 22} -weight 1

    grid $win -sticky nsew
    # allow the columns to resize
    grid columnconfigure $win {0} -weight 1
    grid rowconfigure $win {0} -weight 1
  }

  method ConfigureStyle {} {
    if {0} {
    ttk::style configure Header.TLabel -background pink

    ttk::style configure Even.TEntry -background lightgreen
    ttk::style configure Even.TRadiobutton -background lightgreen

    ttk::style configure Odd.TEntry -background lightyellow
    ttk::style configure Odd.TRadiobutton -background lightyellow
    }
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
}; # end of the View


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

  method UpdateViewFromModel {} {

    # not sure if the names is something we need to record
    #    $self updateViewNames
    $self UpdateViewThresholds 
    #    $self UpdateViewMonitor
    $self UpdateViewPolarities
    $self UpdateViewGains
    $self UpdateViewWidths
    $self UpdateViewDeadtimes
    $self UpdateViewDelays
    $self UpdateViewFractions
  }

  method UpdateModelFromView {} {
  }



  if {0} {
  method UpdateViewNames {} {
    # need to implement
  }
  }

  method UpdateViewThresholds {} {
    for {set ch 0} {$ch<16} {incr ch} {
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
    for {set ch 0} {$ch<8} {incr ch} {
      set pol [$m_handle GetPolarity $ch]
      $m_view SetPolarity $ch $pol
    }
  }

  method UpdateViewGains {} {
    for {set ch 0} {$ch<8} {incr ch} {
      set gain [$m_handle GetGain $ch]
      $m_view SetGain $ch $gain
    }
  }

  method UpdateViewWidths {} {
    for {set ch 0} {$ch<8} {incr ch} {
      set val [$m_handle GetWidth $ch]
      $m_view SetWidth $ch $val
    }
  }

  method UpdateViewDeadtimes {} {
    for {set ch 0} {$ch<8} {incr ch} {
      set val [$m_handle GetDeadtime $ch]
      $m_view SetDeadtime $ch $val
    }
  }

  method UpdateViewDelays {} {
    for {set ch 0} {$ch<8} {incr ch} {
      set val [$m_handle GetDelay $ch]
      $m_view SetDelay $ch $val
    }
  }

  method UpdateViewFractions {} {
    for {set ch 0} {$ch<8} {incr ch} {
      set fr [$m_handle GetFraction $ch]
      $m_view SetFraction $ch $fr
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

