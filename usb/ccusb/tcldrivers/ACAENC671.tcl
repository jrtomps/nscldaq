#===================================================================
# class ACAENC671
#===================================================================


package provide caenc671 1.0

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
  return [$device simpleWrite24 $node $bank 18 $pat]
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
  return [$device simpleWrite24 $node $bank 20 $wid]
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
  return [$device simpleWrite24 $node $ch 17 $del]
}


##
# @brief Set the threshold for a given channel
#
# The range is settable "in a range from -1 and -256mV (5mV minimum 
# required)" in steps of 1mV. 
#
# \param ch the channel to target
# \param th threshold value as an integer between 1 and 255.
#
# \return bit pattern of QX
itcl::body ACAENC671::Threshold {ch th} {
  return [$device simpleWrite24 $node $ch 16 $th]
}
