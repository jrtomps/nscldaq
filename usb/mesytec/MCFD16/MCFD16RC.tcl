
package provide mcfd16rc 1.0

package require snit
package require Utils


snit::type MCFD16RC {

  variable _proxy

  constructor {proxy} {
    set _proxy $proxy
  }

  method SetThreshold {ch val} {
    if {![Utils::isInRange 0 255 $val]} {
      set msg "MCFD16RC::SetThreshold Invalid value provided. Must be in range"
      append msg " \[0,255\]."
      return -code error $msg
    }

    set adr [$type ComputeAddress threshold $ch]
    return [$_proxy Transaction $adr $val]
  }

  method SetPolarity {ch negpos} {
    set val 0
    switch $negpos {
      pos { set val 0}
      neg { set val 1}
      default { 
        set msg {Invalid value provided. Must be "pos" or "neg".}
        return -code error -errorinfo MCFD16RC::SetPolarity $msg
      }
    }

    set adr [$type ComputeAddress polarity $ch]
    return [$_proxy Transaction $adr $val]
  }

  method SetGain {chpair val} {
    if {![Utils::isInRange 0 2 $val]} {
      set msg "MCFD16RC::SetGain Invalid value provided. Must be in range"
      append msg " \[0,2\]."
      return -code error $msg
    }

    set adr [$type ComputeAddress gain $chpair]
    return [$_proxy Transaction $adr $val]
  }

  method SetWidth {chpair val} {
    if {![Utils::isInRange 5 255 $val]} {
      set msg "MCFD16RC::SetWidth Invalid value provided. Must be in range"
      append msg " \[5,255\]."
      return -code error $msg
    }

    set adr [$type ComputeAddress width $chpair]
    return [$_proxy Transaction $adr $val]
  }


  method SetDeadtime {chpair val} {
    if {![Utils::isInRange 5 255 $val]} {
      set msg "MCFD16RC::SetDeadtime Invalid value provided. Must be in range"
      append msg " \[5,255\]."
      return -code error $msg
    }

    set adr [$type ComputeAddress deadtime $chpair]
    return [$_proxy Transaction $adr $val]
  }

  method SetDelay {chpair val} {
    if {![Utils::isInRange 0 4 $val]} {
      set msg "MCFD16RC::SetDelay Invalid value provided. Must be in range"
      append msg " \[0,4\]."
      return -code error $msg
    }

    set adr [$type ComputeAddress delay $chpair]
    return [$_proxy Transaction $adr $val]
  }

  method SetFraction {chpair val} {

    switch $val {
      20 { set val 0 }
      40 { set val 1 }
      default {
        set msg "MCFD16RC::SetFraction Invalid value provided. Must be either "
        append msg "20 or 40."
        return -code error $msg
      }
    }

    set adr [$type ComputeAddress fraction $chpair]
    return [$_proxy Transaction $adr $val]

  }


  method SetMode {mode} {
    set val 0
    switch $mode {
      common { set val 0 }
      individual { set val 1 }
      default {
        set msg "MCFD16RC::SetMode Invalid value provided. Must be either "
        append msg {"common" or "individual".}
        return -code error $msg
      }
    }

    set adr [dict get $offsetsMap mode]
    return [$_proxy Transaction $adr $val]
  }

  typevariable offsetsMap
  typevariable paramRangeMap
  typeconstructor {
    # the offsets map stores the offset for the 
    set offsetsMap [dict create threshold {indiv  0 common 64} \
                                gain      {indiv 16 common 65} \
                                width     {indiv 24 common 66} \
                                deadtime  {indiv 32 common 67} \
                                delay     {indiv 40 common 68} \
                                fraction  {indiv 48 common 69} \
                                polarity  {indiv 56 common 70} \
                                mode      72]

    set paramRangeMap [dict create threshold {low 0 high 16} \
                                   gain      {low 0 high  8}\
                                   width     {low 0 high  8}\
                                   deadtime  {low 0 high  8}\
                                   delay     {low 0 high  8}\
                                   fraction  {low 0 high  8}\
                                   polarity  {low 0 high  8}]

  }

  typemethod ComputeAddress {param ch} {
    # initialize a value to return
    set addr 0

    # get parameter info
    set range [dict get $paramRangeMap $param]
    set low [dict get $range low]
    set high [dict get $range high]

    set offsets [dict get $offsetsMap $param]
    if {$ch == $high} {
      # at max channel value --> common setting
      set addr [dict get $offsets common]
    } elseif {($ch>=$low) && ($ch<$high)} {
      # individual chan
      set offset [dict get $offsets indiv]
      set addr [expr $offset+$ch]
    } else {
      # bad channel
      set msg "Invalid channel provided. Must be in range \[$low,$high\]."
      return -code error $msg
    }

    return $addr
  }
}

snit::type MXDCRCProxy {

  variable _comObject

  constructor {comObject} {
  }


}
