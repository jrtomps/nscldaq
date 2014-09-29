#===================================================================
# class ACAENC671
#===================================================================


package provide caenc671 11.0

##
# Device driver for the CAENC671 CAMAC CFD
#
# This provides minimal functionality for controlling the C671 and 
# does not provide the support for the actual readout control 
#
itcl::class ACAENC671 {
  protected variable device ;#< a USB device from cccusb package
  private variable node     ;#< the slot occupied by the card

  ##
  # Constructor
  #
  # Store the name of the controller and the slot.
  # 
  # @param de   a CAMAC controller
  # @param no   the slot in which the device resides
  #
  constructor {de no} {
    set device $de
    set node $no
  }

  destructor {}

  # Access and Set the controller
  public method SetController {ctlr}
  public method GetController {}

  ##
  # Manipulation functions
  #
  public method GetVariable {v} 
  public method Enable {bank pat} 
  public method Width {code wid} 
  public method Delay {ch del} 
  public method Threshold {ch th} 

  public method DeadTime {bank dt} 
  public method PromptWidth {code wid} 

  private method isInRange {low high val}
}


## @brief Pass a controller object into the class for use
#
# This is particularly useful for the case when the driver is used in an
# initialization script where the controller is technically only known at the
# point of the script. In this case, it is useful for the user to be able to
# configure the device to use the current controller object.
# 
# @param ctlr   a cccusb::CCCUSB object  
itcl::body ACAENC671::SetController {ctlr} {
  set device $ctlr
}

## @brief Retrieve the name of the current controller object
#
# @returns the name of the current controller object
#
itcl::body ACAENC671::GetController {} {
  return $device
}



##
#  @brief Enable channels within bank
#
#  Channels are broken into two banks: 0-7 in bank 0
#  and 8-15 in bank 1. User can provide the proper
#  pattern to write for enabling channels. For example, if bit 0=1
#  then the corresponding channel is enabled.
#
#  \param bank integer specifying bank 0 or 1
#  \param pat  bitset specifying which channels to enable
#
#  \returns bit pattern of QX
itcl::body ACAENC671::Enable {bank pat} {
  if {![isInRange 0 1 $bank]} {
    return -code error \
      "ACAENC671::Enable bank $bank is invalid. Must be either 0 or 1."
  }

  if {[string is boolean $pat]} {
    set offon [string is true $pat]
    return [$device simpleWrite24 $node $bank 18 $offon]
  } else {
    return -code error "ACAENC671::Enable user provided non-boolean value."
  }
}

##
# @brief Set the threshold for a given channel
#
# The range is settable "in a range from -1 and -256mV (5mV minimum 
# required)" in steps of 1mV. 
#
# \param ch the channel to target
# \param th threshold value as an integer between 0 and 255.
#
# \return bit pattern of QX
itcl::body ACAENC671::Threshold {ch th} {
  if {![isInRange 0 15 $ch]} {
    return -code error \
      "ACAENC671::Threshold ch $ch is invalid. Must be in range \[0,15\]."
  }

  if {![isInRange 0 255 $th]} {
    set msg "ACAENC671::Threshold $th is an invalid threshold setting. "
    append msg {Must be in range [0,255].}
    return -code error $msg
  } 

  return [$device simpleWrite24 $node $ch 16 $th]
}


##
# @brief Set the delay value of a given channel
#
# The delay for a channel can be a value between 35ns and
# 535ns, with a granularity of 1.96ns. 
#
# \param ch the channel to target
# \param del the amount of delay as an integer between 0 and 255
#
# \return bit pattern of QX
itcl::body ACAENC671::Delay {ch del} {
  if {![isInRange 0 15 $ch]} {
    return -code error \
      "ACAENC671::Delay ch $ch is invalid. Must be in range \[0,15\]."
  }

  if {![isInRange 0 255 $del]} {
    set msg "ACAENC671::Delay $del is an invalid delay setting. "
    append msg {Must be in range [0,255].}
    return -code error $msg
  } 
  return [$device simpleWrite24 $node $ch 17 $del]
}

##
#  @brief Set delayed output width for a bank
#
#  Set the width of the delayed output signal for a bank of channels.
#  Minimum pulse width = 10ns and Maximum pulse width = 250ns.
#
#  \param bank integer specifying bank 0 or 1
#  \param pat  integer specifying width of signal (min=0, max=255)
#
#  \returns bit pattern of QX
itcl::body ACAENC671::Width {bank wid} {
  if {![isInRange 0 1 $bank]} {
    return -code error \
      "ACAENC671::Width bank $bank is invalid. Must be either 0 or 1."
  }

  if {![isInRange 0 255 $wid]} {
    set msg "ACAENC671::Width $wid is an invalid width setting. "
    append msg {Must be in range [0,255].}
    return -code error $msg
  } 

  set A [expr {$bank + 0}]
  return [$device simpleWrite24 $node $A 20 $wid]
}

## brief Set the dead time for this module
#
# Set the amount of time following an input signal to disable input signals.
# This can be set from 160 ns to 2 us using an 8-bit range. 
# (0 --> 160 ns and 255 --> 2 us)
# 
# @param bank integer specifying bank 0 or 1
# @param dt  value to set delay to [0,255]
# 
itcl::body ACAENC671::DeadTime {bank dt} {
  if {![isInRange 0 1 $bank]} {
    return -code error \
      "ACAENC671::DeadTime bank $bank is invalid. Must be either 0 or 1."
  }

  if {![isInRange 0 255 $dt]} {
    set msg "ACAENC671::DeadTime $dt is an invalid dead time setting. "
    append msg {Must be in range [0,255].}
    return -code error $msg
  } 

  # if we made it here then we can execute the command!
  set A [expr {$bank+4}]

  return [$device simpleWrite24 $node $A 20 $dt]
}


##
#  @brief Set prompt output width for a bank
#
#  Set the width of the prompt output signal for a single channels.
#  Minimum pulse width = 10ns and Maximum pulse width = 250ns.
#
#  \param bank integer specifying bank 0 or 1
#  \param pat  integer specifying width of signal (min=0, max=255)
#
#  \returns bit pattern of QX
itcl::body ACAENC671::PromptWidth {bank wid} {
  if {![isInRange 0 1 $bank]} {
    return -code error \
      "ACAENC671::PromptWidth bank $bank is invalid. Must be either 0 or 1."
  }

  if {![isInRange 0 255 $wid]} {
    set msg "ACAENC671::PromptWidth $wid is an invalid output width setting. "
    append msg {Must be in range [0,255].}
    return -code error $msg

  } 

  set A [expr {$bank+6}]
  return [$device simpleWrite24 $node $A 20 $wid]
}


# -----------------------------------------------------------------------------
# Utility methods

itcl::body ACAENC671::isInRange {low high val} {
  return [expr {($val>=$low) && ($val<=$high)}]
}
