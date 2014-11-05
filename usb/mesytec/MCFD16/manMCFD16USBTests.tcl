

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


tcltest::cleanupTests


