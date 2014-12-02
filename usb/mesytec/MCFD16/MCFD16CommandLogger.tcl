
package provide mcfd16commandlogger 1.0

package require snit


# A snit::type that acts like an actual device driver but
# writes commands to a file instead
snit::type MCFD16CommandLogger {
  option -name -default ::dev

  variable _logFile

  constructor {path args} {
    if {[catch {open $path w} res]} {
      set msg {MCFD16CommandLogger::constructor failed to open $path for writing}
      return -code error $msg
    } else {
      set _logFile $res
    }

    $self configurelist $args
  }
  
  destructor {
    catch {close $_logFile}
  }

  method Log {str} {
    chan puts $_logFile $str
  }

  method SetThreshold {ch val} {
    chan puts $_logFile "[$self cget -name] SetThreshold $ch $val"
  }

  method GetThreshold {ch} {
    chan puts $_logFile "[$self cget -name] GetThreshold $ch"
  }

  method SetGain {ch val} {
    chan puts $_logFile "[$self cget -name] SetGain $ch $val"
  }

  method GetGain {ch} {
    chan puts $_logFile "[$self cget -name] GetGain $ch"
  }

  method SetWidth {ch val} {
    chan puts $_logFile "[$self cget -name] SetWidth $ch $val"
  }

  method GetWidth {ch} {
    chan puts $_logFile "[$self cget -name] GetWidth $ch"
  }

  method SetDeadtime {ch val} {
    chan puts $_logFile "[$self cget -name] SetDeadtime $ch $val"
  }

  method GetDeadtime {ch} {
    chan puts $_logFile "[$self cget -name] GetDeadtime $ch"
  }

  method SetDelay {ch val} {
    chan puts $_logFile "[$self cget -name] SetDelay $ch $val"
  }

  method GetDelay {ch} {
    chan puts $_logFile "[$self cget -name] GetDelay $ch"
  }

  method SetFraction {ch val} {
    chan puts $_logFile "[$self cget -name] SetFraction $ch $val"
  }

  method GetFraction {ch} {
    chan puts $_logFile "[$self cget -name] GetFraction $ch"
  }

  method SetPolarity {ch val} {
    chan puts $_logFile "[$self cget -name] SetPolarity $ch $val"
  }

  method GetPolarity {ch} {
    chan puts $_logFile "[$self cget -name] GetPolarity $ch"
  }

  method SetMode {mode} {
    chan puts $_logFile "[$self cget -name] SetMode $mode"
  }

  method GetMode {} {
    chan puts $_logFile "[$self cget -name] GetMode"
  }

  method EnableRC {state} {
    chan puts $_logFile "[$self cget -name] EnableRC $state"
  }

  method RCEnabled {} {
    chan puts $_logFile "[$self cget -name] RCEnabled"
  }

  method SetChannelMask {mask} {
    chan puts $_logFile "[$self cget -name] SetChannelMask $mask"
  }

  method GetChannelMask {} {
    chan puts $_logFile "[$self cget -name] GetChannelMask"
  }

  method EnablePulser {ptype} {
    chan puts $_logFile "[$self cget -name] EnablePulser $ptype"
  }

  method DisablePulser {} {
    chan puts $_logFile "[$self cget -name] DisablePulser"
  }

  method PulserEnabled {} {
    chan puts $_logFile "[$self cget -name] PulserEnabled"
  }

  method Flush {} {
    chan flush $_logFile
  }

}
