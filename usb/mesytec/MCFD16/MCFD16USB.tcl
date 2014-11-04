



package provide mcfd16usb 1.0

package require snit
package require Utils

snit::type MCFD16USB {

  variable m_serialFile

  constructor {serialFile args} {
    set m_serialFile [open $serialFile "w+"]

#    $self configurelist $args
  }

  destructor {
    catch {close $m_serialFile}
  }

  method SetPolarity {chanPair val} {
    # check for channel pair argument
    $self ThrowOnBadChannelPair $chanPair

    # this is the operational code and the check for the value
    switch $val {
      positive {$self Write "SP $chanPair 0"} 
      negative {$self Write "SP $chanPair 1"} 
      default  {
        set msg "Invalid value provided. Must be \"positive\" or \"negative\"."
        return -code error -errorinfo MCFD16USB::SetPolarity $msg
      } 
    }
  }


  method SetGain {chanPair val} {
    if {$val ni [list 1 3 10]} {
      set msg "Invalid gain value. Must be either 1, 3, or 10."
      return -code error -errorinfo MCFD16USB::SetGain $msg
    }

    $self ThrowOnBadChannelPair $chanPair

    # all is well with the arguments
    $self Write "SG $chanPair $val"
  }


  method EnableBandwidthLimit {on} {
    if {![string is boolean $on]} {
      set msg "Invalid argument provided. Must be a boolean type."
      return -code error -errorinfo MCFD16USB::EnableBandwidthLimit $msg
    }

    # all is well with the arguments
    $self Write "BWL [string is true $on]"

  }

  method SetDiscriminatorMode {mode} {
    switch $mode {
      led {$self Write "CFD 0"}
      cfd {$self Write "CFD 1"}
      default {
        set msg {Invalid argument provided. Must be either "led" or "cfd".}
        return -code error -errorinfo MCFD16USB::SetDiscriminatorMode $msg
      }
    }
  }

  method SetThreshold {ch thresh} {
    # check for valid channel
    if {![Utils::isInRange 0 16 $ch]} {
      set msg {Invalid channel argument. Must be in range [0,16].}
      return -code error -errorinfo MCFD16USB::SetThreshold $msg
    }

    # check for valid threshold 
    if {![Utils::isInRange 0 255 $thresh]} {
      set msg {Invalid threshold argument. Must be in range [0,255].}
      return -code error -errorinfo MCFD16USB::SetThreshold $msg
    }

    # all is well with the arguments
    $self Write "ST $ch $thresh"
  }

  #---------------------------------------------------------------------------#
  # Utility methods
  #

  method Write {script} {
    puts $m_serialFile $script
    chan flush $m_serialFile
  }

  method ThrowOnBadChannelPair {chanPair} {
    if {![Utils::isInRange 0 8 $chanPair]} {
      set msg {Invalid channel pair provided. Must be in range [0,8].}
      return -code error -errorinfo MCFD16USB::SetPolarity $msg
    }
  }




}
