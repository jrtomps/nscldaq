

package require mcfd16usb
package require tcltest

set serialFile /dev/ttyUSB0

MCFD16USB dev $serialFile


tcltest::test polarity-0 { Writing polarity should be effective
} {
  for {set chpr 0} {$chpr<9} {incr chpr} {
    if {($chpr%2)==0} {
      ::dev SetPolarity $chpr neg
    } else {
      ::dev SetPolarity $chpr pos
    }
  }

  set state [list]
  for {set chpr 0} {$chpr<9} {incr chpr} {
    lappend state [::dev GetPolarity $chpr]
  }
  set state
} {neg pos neg pos neg pos neg pos neg}




tcltest::test gain-0 { Writing gain should work
} {
  for {set ch 0} {$ch<9} {incr ch} {
    set gain [lindex {1 3 10} [expr $ch%3]]
    ::dev SetGain $ch $gain
  }

  set state [list]
  for {set ch 0} {$ch<9} {incr ch} {
    lappend state [::dev GetGain $ch]
  }
  set state
} {1 3 10 1 3 10 1 3 10}


tcltest::test bwl-0 { Set bandwidth should be responsive
} {
  # all fails if not in RC mode
  ::dev EnableRC 1

  set state [list]
  ::dev EnableBandwidthLimit on
  lappend state [::dev GetBandwidthLimit]
  ::dev EnableBandwidthLimit false
  lappend state [::dev GetBandwidthLimit]
  set state
} {1 0}

tcltest::test discriminator-0 {We should be able to set the disc mode
} {
  # this is necessary for it to work
  ::dev EnableRC 1

  set state [list]
  set res [catch {
  ::dev SetDiscriminatorMode cfd
  lappend state [::dev GetDiscriminatorMode]
  ::dev SetDiscriminatorMode led
  lappend state [::dev GetDiscriminatorMode]
  ::dev SetDiscriminatorMode cfd
  lappend state [::dev GetDiscriminatorMode]
  } msg]

  if {$res == 1} {
    puts $msg
  }

  set state
} {cfd led cfd}

tcltest::test threshold-0 { Writing thresholds should work
} {
  for {set ch 0} {$ch<17} {incr ch} {
      ::dev SetThreshold $ch $ch
  }

  set state [list]
  for {set ch 0} {$ch<17} {incr ch} {
    lappend state [::dev GetThreshold $ch]
  }
  set state
} {0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16}

tcltest::test width-0 { We should be able to write widths
} {
  
  for {set ch 0} {$ch<9} {incr ch} {
    ::dev SetWidth $ch [expr $ch+100]
  }

  set state [list]
  for {set ch 0} {$ch<9} {incr ch} {
    lappend state [::dev GetWidth $ch]
  }
  set state
} {160 162 165 167 170 172 174 177 179}

tcltest::test delay-0 { make sure we can set the delay and read it back
} {
  set res [catch {
  for {set ch 0} {$ch<9} {incr ch} {
    set delay [lindex {1 2 3 4 5} [expr $ch%5]]
    ::dev SetDelay $ch $delay
  }

  set state [list]
  for {set ch 0} {$ch<9} {incr ch} {
    lappend state [::dev GetDelay $ch]
  }
  } msg]
  if {$res} {
    puts $msg
  }
  set state
} {1 2 3 4 5 1 2 3 4}

tcltest::test fraction-0 { make sure we can set the delay and read it back
} {
  for {set ch 0} {$ch<9} {incr ch} {
    set frac [lindex {20 40} [expr $ch%2]]
    ::dev SetFraction $ch $frac
  }

  set state [list]
  for {set ch 0} {$ch<9} {incr ch} {
    lappend state [::dev GetFraction $ch]
  }

  set state
} {20 40 20 40 20 40 20 40 20}

tcltest::test pulser-0 {Make sure we can enable/disable pulser
} {
  set state [list]
  ::dev EnablePulser 1
  lappend state [::dev PulserEnabled]
  ::dev DisablePulser
  lappend state [::dev PulserEnabled]
  ::dev EnablePulser 2
  lappend state [::dev PulserEnabled]

  set state
} {1 0 1}

tcltest::test rc-0 { Set rc mode should work
} {
  set state [list]
  ::dev EnableRC 1
  lappend state [::dev RCEnabled]
  ::dev EnableRC 0
  lappend state [::dev RCEnabled]
  set state
} {1 0}

tcltest::cleanupTests


