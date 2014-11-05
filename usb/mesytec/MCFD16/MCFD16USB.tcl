


package provide mcfd16usb 1.0

package require snit
package require Utils




snit::type MCFD16USB {

  variable m_serialFile ;# the communication channel to the device 
  variable m_needsUpdate ;# determines whether device state has changed since last update
  variable m_moduleState ;# the dict storing the state


  constructor {serialFile args} {
    set m_serialFile [open $serialFile "w+"]

    chan configure $m_serialFile -blocking 0

    # we don't know anything about the state of the module
    # so we are certainly in need of an update
    set m_needsUpdate 1
    set m_moduleState [dict create]

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
      pos {$self Transaction "SP $chanPair 0"} 
      neg {$self Transaction "SP $chanPair 1"} 
      default  {
        set msg {Invalid value provided. Must be "pos" or "neg".}
        return -code error -errorinfo MCFD16USB::SetPolarity $msg
      } 
    }
  }

  method GetPolarity {chanPair} {
    return [$self GetChannelPairParam Polarity $chanPair]
  }


  method SetGain {chanPair val} {
    if {$val ni [list 1 3 10]} {
      set msg "Invalid gain value. Must be either 1, 3, or 10."
      return -code error -errorinfo MCFD16USB::SetGain $msg
    }

    $self ThrowOnBadChannelPair $chanPair

    # all is well with the arguments
    $self Transaction "SG $chanPair $val"
  }

  method GetGain {chanPair} {
    return [$self GetChannelPairParam Gain $chanPair]
  }

  method EnableBandwidthLimit {on} {
    if {![string is boolean $on]} {
      set msg "Invalid argument provided. Must be a boolean type."
      return -code error -errorinfo MCFD16USB::EnableBandwidthLimit $msg
    }

    # all is well with the arguments
    $self Transaction "BWL [string is true $on]"

  }

  method GetBandwidthLimit {} {
    return [$self GetBooleanParam {Bandwidth limit} {Off 0 On 1}]
  }

  method SetDiscriminatorMode {mode} {
    switch $mode {
      led {$self Transaction "CFD 0"}
      cfd {$self Transaction "CFD 1"}
      default {
        set msg {Invalid argument provided. Must be either "led" or "cfd".}
        return -code error -errorinfo MCFD16USB::SetDiscriminatorMode $msg
      }
    }
  }

  method GetDiscriminatorMode {} {
    set convTable {{Constant fraction} cfd {Leading edge} led}
    return [$self GetBooleanParam Discrimination $convTable]
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
    $self Transaction "ST $ch $thresh"
  }

  method GetThreshold {ch} {
    # check for valid channel
    if {![Utils::isInRange 0 16 $ch]} {
      set msg {Invalid channel argument. Must be in range [0,16].}
      return -code error -errorinfo MCFD16USB::GetThreshold $msg
    }

    if {$m_needsUpdate} {
      $self Update
    }

    return [lindex [dict get $m_moduleState {Threshold}] $ch]
  }

  method SetWidth {chanPair value} {
    $self ThrowOnBadChannelPair $chanPair

    if {![Utils::isInRange 16 222 $value]} {
      set msg {Invalid width argument provided. Must be in range [16,222].}
      return -code error -errorinfo MCFD16USB::SetWidth $msg
    }

    $self Transaction "SW $chanPair $value"
  }

  method GetWidth {chanPair} {
    return [$self GetChannelPairParam Width $chanPair]
  }


  method SetDeadtime {chanPair value} {
    $self ThrowOnBadChannelPair $chanPair

    if {![Utils::isInRange 27 222 $value]} {
      set msg {Invalid deadtime argument provided. Must be in range [27,222].}
      return -code error -errorinfo MCFD16USB::SetDeadtime $msg
    }


    $self Transaction "SD $chanPair $value"
  }

  method GetDeadtime {chanPair} {
    return [$self GetChannelPairParam Deadtime $chanPair]
  }


  method SetDelay {chanPair value} {

    $self ThrowOnBadChannelPair $chanPair

    if {![Utils::isInRange 1 5 $value]} {
      set msg {Invalid delay argument provided. Must be in range [1,5].}
      return -code error -errorinfo MCFD16USB::SetDelay $msg
    }

    $self Transaction "SY $chanPair $value"
  }

  method GetDelay {chanPair} {
    return [$self GetChannelPairParam Delay $chanPair]
  }

  method SetFraction {chanPair value} {

    $self ThrowOnBadChannelPair $chanPair

    if {$value ni [list 20 40]} {
      set msg "Invalid fraction argument provided. Must be either 20 or 40."
      return -code error -errorinfo MCFD16USB::SetFraction $msg
    }

    $self Transaction "SF $chanPair $value"
  }

  method GetFraction {chanPair} {
    return [$self GetChannelPairParam Fraction $chanPair]
  }

  method SetChannelMask {bank mask} {

    if {$bank ni [list 0 1]} {
      set msg "Invalid bank parameter provided. Must be either 0 or 1."
      return -code error -errorinfo MCFD16USB::SetChannelMask $msg
    }

    if {![Utils::isInRange 0 255 $mask]} {
      set msg {Invalid mask argument provided. Must be in range [0,255].}
      return -code error -errorinfo MCFD16USB::SetChannelMask $msg
    }

    $self Transaction "SK $bank $mask"
  }

  # not sure how to handle this b/c I don't understand the response from the module
  method GetChannelMask {bank} {}

  method EnablePulser {index} {
    if {$index ni [list 1 2]} {
      set msg {Invalid pulser index provided. Must be either 1 or 2.}
      return -code error -errorinfo MSCF16::EnablePulser $msg
    }

    $self Transaction "P$index"
  }

  method DisablePulser {} {
    $self Transaction "P0"
  }

  method PulserEnabled {} { 
    return [$self GetBooleanParam {Test pulser} {Off 0 On 1}]
  }

  method ReadFirmware {} {
    $self Transaction "V"
  }

  method EnableRC {on} {
    if {![string is boolean $on]} {
      set msg "Invalid argument provided. Must be a boolean value."
      return -code error -errorinfo MCFD16USB::EnableRC $msg
    }

    if {[string is true $on]} {
      $self Transaction "ON"
    } else {
      $self Transaction "OFF"
    }
  }

  method RCEnabled {} {
    return [$self GetBooleanParam {Remote Control} {Off 0 On 1}]
  }

  #---------------------------------------------------------------------------#
  # Utility methods
  #

  method Write {script} {

    # check that the command name (the first element of the script list) is
    # going to modify the state of the module
    if {[lindex $script 0] ni [list DP DT DS V ? H]} { 
      # we will need to update our state the next time the user queries a value
      set m_needsUpdate 1
    }

    puts $m_serialFile $script
    chan flush $m_serialFile

  }

  method Read {} {
    set totalResponse ""
#    set response [chan read -nonewline $m_serialFile]
    set response [chan read $m_serialFile]
    append totalResponse $response

    while {![string match "mcfd-16>" [string range $totalResponse end-7 end]]} {
      after 25 
#      set response [chan read -nonewline $m_serialFile]
      set response [chan read $m_serialFile]
      append totalResponse $response
    }
    return $totalResponse
  }

  method Transaction {script} {
    $self Write $script

    return [$self Read]
  }

  method ThrowOnBadChannelPair {chanPair} {
    if {![Utils::isInRange 0 8 $chanPair]} {
      set msg {Invalid channel pair provided. Must be in range [0,8].}
      return -code error -errorinfo MCFD16USB::SetPolarity $msg
    }
  }

  method CacheModuleState {stateDict} {
    set m_moduleState $stateDict
  }

  # ------ PARSING UTILITIES ---------------------------------------------#

  method ParseDSResponse {response } {
    set lines [split $response "\n"]

    set parsedResponse [list]

    # first line should be DS
    if {[lindex $lines 0] ne "DS"} {
      set msg "Response being processed does not correspond to DS parameter"
      return -code error -errorinfo MCFD16USB::ParseDSResponse $msg
    }

    # line 1 is a space

    # parse threhsolds
    lappend parsedResponse [$self ParseThresholds [lrange $lines 2 3]] 

    # parse the gain
    lappend parsedResponse [$self ParseChanPairLine [lindex $lines 4]]
    # parse the width
    lappend parsedResponse [$self ParseChanPairLine [lindex $lines 5] 1]
    # parse the deadtime
    lappend parsedResponse [$self ParseChanPairLine [lindex $lines 6] 1]
    # parse the delay
    lappend parsedResponse [$self ParseChanPairLine [lindex $lines 7] 1]
    # parse the fraction
    lappend parsedResponse [$self ParseChanPairLine [lindex $lines 8] 1]
    # parse the polarity
    lappend parsedResponse [$self ParseChanPairLine [lindex $lines 9]]

    # line 10 is a space
    
    # parse the mask
    lappend parsedResponse [$self ParseMaskReg [lindex $lines 11]]
    # parse the discrim mode
    lappend parsedResponse [$self ParseSimpleLine [lindex $lines 12]]

    # we don't really care about the gate/delay gen state
    # lappend parsedResponse [$self ParseSimpleLine [lindex $lines 13]]
    # we don't really care about the coinc time
    # lappend parsedResponse [$self ParseSimpleLine [lindex $lines 14]]
    
    # operating mode
    lappend parsedResponse [$self ParseSimpleLine [lindex $lines 15]]
    # bandwidth
    lappend parsedResponse [$self ParseSimpleLine [lindex $lines 16]]
    # rc mode
    lappend parsedResponse [$self ParseSimpleLine [lindex $lines 17]]

    # # frequency monitor
    # lappend parsedResponse [$self ParseSimpleLine [lindex $lines 18]]

    # pulser
    lappend parsedResponse [$self ParseSimpleLine [lindex $lines 19]]

    ## hardware settings
    #    lappend parsedResponse [$self ParseSimpleLine [lindex $lines 20]]
    
    return [$self TransformToDict $parsedResponse]
    
  }


  method SplitAndTrim {line del} {

    # split at the colon
    set split [split $line $del]

    # trim each part
    set name [string trim [lindex $split 0]]
    set valStr [string trim [lindex $split 1]]

    return [list $name $valStr]
  }

  method RemoveDash {line} {

    # basically we just split at all of the dashes and then replace it with a space
    return [join [$self SplitAndTrim $line "-"] " "]
  }

  method ParseSimpleLine {line {stripUnit 0}} {
    set split [$self SplitAndTrim $line ":"]

    set name [lindex $split 0]
    if {$stripUnit} {
      set name [$self StripUnit $name]
    }

    return [dict create name $name values [lindex $split 1]]
  } 

  method StripUnit str {
    return [lindex [split $str " "] 0]
  }

  method ParseChanPairLine {line {stripUnit 0}} {

    set splitLine [$self SplitAndTrim $line ":"]

    set name [lindex $splitLine 0]
    if {$stripUnit} {
      set name [$self StripUnit $name]
    }

    set valStr [lindex $splitLine 1]

    set valStr [$self RemoveDash $valStr]

    # locate the dash and remove it with its trailing space
    # convert to a list
    set vals [split $valStr " "]

    # form the response
    return [dict create name $name values $vals]
  }


  method ParseThresholds {lines} {
    
    set line0 [lindex $lines 0]
    set splitLine [$self SplitAndTrim $line0 ":"]

    set name [lindex $splitLine 0]
    set valStr0 [lindex $splitLine 1]

    set line1 [string trim [lindex $lines 1]]
    set valStr1 [$self RemoveDash $line1]

    set vals [concat [split $valStr0 " "] [split $valStr1 " "]]

    return [dict create name $name values $vals]
  }


  method ParseMaskReg {line} {
    set splitLine [split $line ":"]
    if {[llength $splitLine]!=3} {
      set msg "Failed to understand parse : \"$line\""
      return -code error -errorinfo MCFD16USB::ParseMaskReg $msg
    }

    # name is the first eleement of list
    set name [string trim [lindex $splitLine 0]]

    # we still have some work to do to get the value of the mask
    # it should be something like " 00000000 (0)" at this point. 
    # Trim it and split it at all spaces. The value is in the first element
    set valStr [string trim [lindex $splitLine 2]]
    set valStr [lindex [$self SplitAndTrim $valStr " "] 0]

    set valStr [string reverse $valStr]
    binary scan [binary format b32 $valStr] i val
    return [dict create name $name values $val]
  }

  method TransformToDict {list} {

    # assume that we have a bunch of dicts with two keys each: name and values

    set flatDict [dict create]
    foreach d $list {
      dict set flatDict [dict get $d name] [dict get $d values]
    }

    return $flatDict
  }

  method Update {} {
    set response [$self Transaction "DS"]

    set m_moduleState [$self ParseDSResponse $response]
    set m_needsUpdate 0
  }


  method GetChannelPairParam {key chpair} {
    $self ThrowOnBadChannelPair $chpair

    if {$m_needsUpdate} {
      $self Update
    }

    return [lindex [dict get $m_moduleState $key] $chpair]
  }

  method GetBooleanParam {key convTable} {
    if {$m_needsUpdate} {
      $self Update
    }

    set state [dict get $m_moduleState $key]

    if {$state ni [dict keys $convTable]} {
      set msg "Device state ($state) for $key is not understood by this driver."
      return -code error $msg
    } else {
      return [dict get $convTable $state]
    }
  }

}
