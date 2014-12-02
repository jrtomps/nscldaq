
package provide mcfd16statelogger 1.0

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

  method SetThreshold {ch val} {
    chan puts $_logFile "[$self cget -name] SetThreshold $ch $val"
  }

  method SetGain {ch val} {
    chan puts $_logFile "[$self cget -name] SetGain $ch $val"
  }

  method SetWidth {ch val} {
    chan puts $_logFile "[$self cget -name] SetWidth $ch $val"
  }

  method SetDeadtime {ch val} {
    chan puts $_logFile "[$self cget -name] SetDeadtime $ch $val"
  }

  method SetDelay {ch val} {
    chan puts $_logFile "[$self cget -name] SetDelay $ch $val"
  }

  method SetFraction {ch val} {
    chan puts $_logFile "[$self cget -name] SetFraction $ch $val"
  }

  method SetPolarity {ch val} {
    chan puts $_logFile "[$self cget -name] SetPolarity $ch $val"
  }

  method SetMode {mode} {
    chan puts $_logFile "[$self cget -name] SetMode $mode"
  }

  method EnableRC {state} {
    chan puts $_logFile "[$self cget -name] EnableRC $state"
  }

  method SetChannelMask {mask} {
    chan puts $_logFile "[$self cget -name] SetChannelMask $mask"
  }

  method EnablePulser {ptype} {
    chan puts $_logFile "[$self cget -name] EnablePulser $ptype"
  }

  method Flush {} {
    chan flush $_logFile
  }

}
