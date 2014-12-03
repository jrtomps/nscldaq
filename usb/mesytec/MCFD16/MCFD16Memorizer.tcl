#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#    Jeromy Tompkins
#	   NSCL
#	   Michigan State University
#	   East Lansing, MI 48824-1321

package provide mcfd16memorizer 1.0

package require snit
package require Utils 

snit::type MCFD16Memorizer {

  component _memory

  constructor {} {
    install _memory using dict create

    $self _SetUpMemory
  }

  method SetThreshold {ch val} { dict set _memory threshold $ch $val }
  method GetThreshold {ch} { return [dict get $_memory threshold $ch]}

  method SetGain {ch val} { dict set _memory gain $ch $val }
  method GetGain {ch} { return [dict get $_memory gain $ch]}

  method SetWidth {ch val} { dict set _memory width $ch $val }
  method GetWidth {ch} { return [dict get $_memory width $ch]}

  method SetDeadtime {ch val} { dict set _memory deadtime $ch $val }
  method GetDeadtime {ch} { return [dict get $_memory deadtime $ch]}

  method SetDelay {ch val} { dict set _memory delay $ch $val }
  method GetDelay {ch} { return [dict get $_memory delay $ch]}

  method SetFraction {ch val} { dict set _memory fraction $ch $val }
  method GetFraction {ch} { return [dict get $_memory fraction $ch]}

  method SetPolarity {ch val} { dict set _memory polarity $ch $val }
  method GetPolarity {ch} { return [dict get $_memory polarity $ch]}

  method SetMode {mode} { dict set _memory mode $mode }
  method GetMode {} { return [dict get $_memory mode]}

  method EnableRC {on} { dict set _memory rc [string is true $on] }
  method RCEnabled {} { return [dict get $_memory rc]}

  method SetChannelMask {mask} { dict set _memory mask $mask}
  method GetChannelMask {} { return [dict get $_memory mask]}

  method EnablePulser {ptype} { dict set _memory pulser $ptype}
  method DisablePulser {} { dict set _memory pulser 0}
  method PulserEnabled {} { return [dict get $_memory pulser]}

  method _SetUpMemory {} {
    # add the dict for threshold and fill with defaults
    $self _AddParamToMemory threshold 0 16

    # add the dict for gain and fill with defaults
    $self _AddParamToMemory gain 0 8

    # add the dict for width and fill with defaults
    $self _AddParamToMemory width 0 8

    # add the dict for deadtime and fill with defaults
    $self _AddParamToMemory deadtime 0 8

    # add the dict for delay and fill with defaults
    $self _AddParamToMemory delay 0 8

    # add the dict for fraction and fill with defaults
    $self _AddParamToMemory fraction 0 8

    # add the dict for polarity and fill with defaults
    $self _AddParamToMemory polarity 0 8

    dict set _memory mode NA
    dict set _memory rc NA
    dict set _memory mask NA
    dict set _memory pulser NA
  }

  method _AddParamToMemory {name low high} {
    dict set _memory $name [dict create]
    foreach ch [Utils::sequence $low $high 1] {
      dict set _memory $name $ch NA
    }

  }
}


