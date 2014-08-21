

package provide ccusbcamac 1.0

package require cccusb


## @namespace the namespace
#
#
namespace eval ccusbcamac {
  
  variable connectionInfo ""

  ## @brief Check whether f maps to a read function
  #
  # @param f the f value of an NAF command
  # 
  # @return bool
  # @retval 0 - is not a read command
  # @retval 1 - is a read command
  proc _isRead {f} {
    return [expr {$f>=0 && $f<8}]
  }

  ## @brief Check whether f maps to a write function
  #
  # @param f the f value of an NAF command
  # 
  # @return bool
  # @retval 0 - is not a write command
  # @retval 1 - is a write command
  proc _isWrite {f} {
    return [expr {$f>=16 && $f<24}]
  }


  ## @brief Compute the absolute crate index 
  #
  # 
  proc _computeIndex {b c} {
  
    _checkValidBAndC $b $c

    return [expr {$b*7+$c}]
  }


  proc _isValidBranchIndex b {
    return [expr {$b>=0 && $b<8}]
  }

  proc _isValidCrateIndex c {
    return [expr {$c>0 && $c<8}]
  }

  proc _checkValidBAndC {b c} {
    # Check that the branch is in range
    set bIsGood [_isValidBranchIndex $b]
    set cIsGood [_isValidCrateIndex $c]

    if {!$bIsGood && !$cIsGood} {
      set msg "ccusbcamac::_checkValidBAndC : branch and crate indices out of range. "
      append msg {Branch should be in range [0,7] and crate in range [1,7]} 
      return -code error $msg
    }

    if {! $bIsGood} {
      return -code error {ccusbcamac::_checkValidBAndC : branch index out of range [0,7]}
    }

    # check that the branch is out of range
    if {! $cIsGood} {
      return -code error {ccusbcamac::_checkValidBAndC : crate index out of range [1,7]}
    }
  }


}

proc ccusbcamac::cdconn {b c host port name} {
  variable connectionInfo
  ::ccusbcamac::_checkValidBAndC $b $c

  set id [::ccusbcamac::_computeIndex $b $c]
  dict set connectionInfo $id [list $host $port $name]

}


##
#
#
proc ccusbcamac::cdreg {b c n} {
  variable connectionInfo
  
  set id [::ccusbcamac::_computeIndex $b $c]
  set connInfo [dict get $connectionInfo $id]
  
  set host [lindex $connInfo 0]
  set port [lindex $connInfo 1]
  set name [lindex $connInfo 2]

  set dev [cccusb::CCCUSBRemote %AUTO $name $host $port]
  return [list $dev $n]  
} 


##
#
#
proc ccusbcamac::cfsa {reg f a {d ""}} {

  if {[::ccusbcamac::_isRead $f]} {
    return [ccusbcamac::_doRead24 $reg $f $a ]

  } elseif {[::ccusbcamac::_isWrite $f]} {
    if {$d ne ""} {
      return [ccusbcamac::_doWrite24 $reg $f $a $d]
    } else {
      return -code error "ccusbcamac::cfsa not provided data to write"
    }
  } else {

    return [ccusbcamac::_doControl $reg $f $a]
  } 
}

##
#
#
proc ccusbcamac::cssa {reg f a {d ""}} {

  if {[::ccusbcamac::_isRead $f]} {
    return [ccusbcamac::_doRead16 $reg $f $a ]

  } elseif {[::ccusbcamac::_isWrite $f]} {
    return [ccusbcamac::_doWrite16 $reg $f $a $d]
  } else {

    return [ccusbcamac::_doControl $reg $f $a]
  } 
}


proc ccusbcamac::qstop {reg f a {maxn ""}} {

}


proc ccusbcamac::qscan {reg f a {maxn ""}} {

}

proc ccusbcamac::cblock {reg f a num} {

}

proc ccusbcamac::isOnline {b c} {

}

proc ccusbcamac::getGl {b} {

}

proc ccusbcamac::C {b c} {

}

proc ccusbcamac::Z {b c} {

}

proc ccusbcamac::isInhibited {b c} {

}

proc ccusbcamac::Inhibit {b c onoff} {

}

proc ccusbcamac::ReadLams {b c} {

}
##
#
#
proc ccusbcamac::_doRead16 {reg f a } {
  
  set ctlr [lindex $reg 0]
  set n    [lindex $reg 1]

  set result [$ctlr simpleRead16 $n $a $f]

  set data [expr {$result & 0xffff}]
  set q [expr {($result >> 24) & 0x1}]
  set x [expr {($result >> 25) & 0x1}]

  return [list $data $q $x]
}

##
#
#
proc ccusbcamac::_doRead24 {reg f a } {
  
  set ctlr [lindex $reg 0]
  set n    [lindex $reg 1]

  set result [$ctlr simpleRead24 $n $a $f]

  set data [expr {$result & 0xffffff}]
  set q [expr {($result >> 24) & 0x1}]
  set x [expr {($result >> 25) & 0x1}]

  return [list $data $q $x]
  
}

##
#
#
proc ccusbcamac::_doWrite16 {reg f a d} {
  
  set ctlr [lindex $reg 0]
  set n    [lindex $reg 1]

  set result [$ctlr simpleWrite16 $n $a $f $d]

  set q [expr {($result & $::cccusb::CCCUSB_Q) == $::cccusb::CCCUSB_Q}]
  set x [expr {($result & $::cccusb::CCCUSB_X) == $::cccusb::CCCUSB_X}]

  return [list $d $q $x]
  
}

##
#
#
proc ccusbcamac::_doWrite24 {reg f a d} {
  
  set ctlr [lindex $reg 0]
  set n    [lindex $reg 1]

  set result [$ctlr simpleWrite24 $n $a $f $d]

  set q [expr {($result & $::cccusb::CCCUSB_Q) == $::cccusb::CCCUSB_Q}]
  set x [expr {($result & $::cccusb::CCCUSB_X) == $::cccusb::CCCUSB_X}]

  return [list $d $q $x]
  
}

##
#
#
proc ccusbcamac::_doControl {reg f a} {
  
  set ctlr [lindex $reg 0]
  set n    [lindex $reg 1]

  set result [$ctlr simpleControl $n $a $f]

  set q [expr {($result & $::cccusb::CCCUSB_Q) == $::cccusb::CCCUSB_Q}]
  set x [expr {($result & $::cccusb::CCCUSB_X) == $::cccusb::CCCUSB_X}]

  return [list 0 $q $x]
  
}


