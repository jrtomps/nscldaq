



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
    if {![Utils::isInRange 0 8 $chanPair]} {
      set msg {Invalid channel pair provided. Must be in range [0,8].}
      return -code error -errorinfo MCFD16USB::SetPolarity $msg
    }

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

    if {![Utils::isInRange 0 8 $chanPair]} {
      set msg {Invalid channel pair provided. Must be in range [0,8].}
      return -code error -errorinfo MCFD16USB::SetGain $msg
    }

    $self Write "SG $chanPair $val"
  }


  #---------------------------------------------------------------------------#
  # Utility methods
  #

  method Write {script} {
    puts $m_serialFile $script
    chan flush $m_serialFile
  }

  




}
