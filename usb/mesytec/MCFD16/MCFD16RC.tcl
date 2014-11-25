
package provide mcfd16rc 1.0

package require snit
package require Utils
package require usbcontrolclient

## A class that formulates RC commands for the MCFD-16 and initializes a
#  transaction
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


  method EnablePulser {pulser} {
    if {$pulser ni [list 1 2]} {
      set msg {MCFD16RC::EnablePulser Invalid value provided. Must be either}
      append msg { 1 or 2.}
      return -code error $msg
    }

    set adr [dict get $offsetsMap pulser]
    return [$_proxy Transaction $adr $pulser]
  }

  method DisablePulser {} {
    set adr [dict get $offsetsMap pulser]
    return [$_proxy Transaction $adr 0]
  }


  method SetChannelMask {bank mask} {

    set maskName masklow

    switch $bank {
      0 { set maskName masklow}
      1 { set maskName maskhigh}
      default {
        set msg "MCFD16RC::SetChannelMask Invalid bank value. "
        append msg "Must be either 0 or 1."
        return -code error $msg
      }
        
    }
    set adr [dict get $offsetsMap $maskName]
    return [$_proxy Transaction $adr $mask]
  }

  method EnableRC {on} {
    if {![string is boolean $on]} {
      set msg {MCFD16RC::EnableRC Invalid value provided. Must be a boolean.}
      return -code error $msg
    }

    set on [string is true $on]
    set adr [dict get $offsetsMap rc]
    return [$_proxy Transaction $adr $on]
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
                                mode      72 \
                                rc        73 \
                                masklow   83 \
                                pulser    118 ]

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

  option -module 
  option -devno 

  component _comObject
  delegate option * to _comObject

  constructor {args} {
    install _comObject using controlClient %AUTO% 
    $self configurelist $args
  }

  destructor {
  }

  method Transaction {args} {
    set paramAddr [lindex $args 0]
    set param [$self _formatParameter [$self cget -devno] $paramAddr]

    set result ""
    if {[llength $args]==2} {
      set result [$_comObject Set [$self cget -module] $param [lindex $args 1]]
    } elseif {[llength $args]==1} {
      set result [$_comObject Get [$self cget -module] $param]
    } else {
      set msg "MXDCRCProxy::Transaction Incorrect number of arguments "
      append msg "provided. Only 1 or 2 arguments allowed."
      return -code error $msg
    }

    return $result
  }

  method getComObject {} { return $_comObject}

  method _formatParameter {devNo paramAddr} {
    return "d${devNo}a${paramAddr}"
  }

}
