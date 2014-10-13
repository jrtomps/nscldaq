#===================================================================
# class ACAENN568B
#===================================================================
#
package provide caenn568b 11.0

package require Itcl
package require Utils


## @brief Device driver for the CAEN N568B Spec Amp
#
# This device driver provides the mechanisms to support the configuration of
# the CAEN N568B through the ACAENV288 driver. A single instance of this driver
# should control a single N568B living as a node on the caennet controlled by
# the V288. The user has the ability to set the coarse gain, fine gain, shaping
# time, pole zero value, output polarity, and the configuration of the output
# (normal or inverted).
#
#
itcl::class ACAENN568B {
	private variable caennet  ;#< a CAENV288 object
	private variable number   ;#< node index
	private variable channels ;#< number of channels
	private variable wait     ;#< number of ms to wait after sending a command
	
  ## @brief Constructor
  #
  # Store necessary variables.
  #
  # @param caen   an ACAENV288 object
  # @param num    the node number of the CAENN568B to target
  #
  # @returns name of the newly constructed object
	constructor {caen num} {
		set caennet $caen
		set number $num
		set channels 16
		set wait 20
	}
	
  ## @brief Destructor - noop
	destructor {}
	
  ## @brief Set a parameter for a specific channel
  #
  # Possible parameter values: fgain, cgain, pzero, shape, out, polar
  # There understood parameters and their ranges are specified below:
  #  Description  Name       Range
  #  Fine gain    fgain     [0,255]
  #  Course gain  cgain     [0,7] 
  #  Pole zero    pzero     [0,255]
  #  Shaping      shape     [0,3]
  #  Output conf. out       0 or 1 
  #  Polarity     polar     0 or 1 
  #
  # @param channel    channel to target in device
  # @param parameter  parameter name (see table above)
  # @param value      parameter value to write
  #
  # @returns the response from the slave
	public method SetParameter {channel parameter value}


  ## @brief Disable the multiplexed outputs
  #
  # @returns response from the slave
	public method DisableMUX {}

  ## @brief Enable multiplexed outputs
  #
  # @returns response from the slave 
	public method EnableMUX {}

  ## @brief Sets the DC offset of the signal (shifts baseline of output)
  #
  # @param value  offset value in range [0,255]
	public method SetOffset {value}

	public method ReadOffset {}

	public method ReadChannelParameters {channel}

  ## @brief Test that we can read from the module
  #
  # This actually performs a read of the offset value
  #
  # @returns slave response for the offset
	public method TestCommunication {}

  ## @brief Standard initialization routine
  #
  # This sources a file that should define the proper configuration
  # information. The user should have in this file an old-style tcl array
  # containing element names of the form %s%02d where %s is a parameter name and
  # the %02d is the channel:
  # 
  #   cgain00, cgain01, cgain02, ... , cgain15
  #   fgain00, fgain01, fgain02, ... , fgain15
  #   polar00, polar01, polar02, ... , polar15
  #   pzero00, pzero01, pzero02, ... , pzero15
  #   shape00, shape01, shape02, ... , shape15
  #   offset
  #
  # It is not necessary that the user provide every possible parameter name,
  # however, the value of any unspecified parameter will be undefined. 
  #
  # @param filename a filename containing config info
  # @param array    name containing config info
  #
  # @returns ""
  #
  # @throws error if unable to communicate with device (@see TestCommunication)
	public method Init {filename aname}


  ## @brief Return the value of a nested variable name 
  #
	public method GetVariable {v} {set $v}


} ;# end of class definition


#-----------------------------------------------------------------------------#
# Implementation
#

#
# 
#
# In this method the parameter to be set is coded in index
itcl::body ACAENN568B::SetParameter {channel parameter value} {

  # allowed parameters and their value upper bounds
	set plist {fgain cgain pzero shape out polar}
	set llist {255 7 255 3 1 1}

  # find the parameter in the list
	set index [lsearch $plist $parameter]
	if {$index == -1} {
		tk_messageBox -message "$this: invalid parameter ($parameter)" -icon error
		return
	}

  # if here, the parameter name was valid...is the value provided in range?
	if {![Utils::isInRange 0 [lindex $llist $index] $value]} {
		tk_messageBox -message "$this: $parameter out of range ($value)" -icon error
		return
	}

  # is the channel valid? 
	if {![Utils::isInRange 0 $channels $channel]} {
    set msg "$this: channel number out of range ($channel)"
		tk_messageBox -message $msg -icon error
		return
	}

  # all arguments checked out...send the command for the channel
  set code [expr $channel*0x100 + 0x10 + $index]
	set status [$caennet Send $number $code $value]
  puts "Send returned status : $status"

  # handle the response
	if {$status} {
    set msg "$this: error setting $parameter. Module number: $number. "
    append msg "Error code: $status"
		tk_messageBox -message $msg -icon error
		return $status
	} 
}


itcl::body ACAENN568B::ReadChannelParameters {channel} {

  if {![Utils::isInRange 0 15 $channel]} {
    set msg "ACAENN568B::ReadParameter Channel $channel is out of range. "
    append msg {Must be in range [0,15].} 
    return -code error $msg
  }
  
  set code [expr $channel*0x10 + 3]
  set status [$caennet SendCode $number $code]
  if {$status} {
    set msg "$this: error reading channel parameters. Module number: $number. "
    append msg "Error code: $status"
		return -code error $msg
  } else {
		after $wait
		return [$caennet Receive]
  }
}


itcl::body ACAENN568B::ReadOffset {} {
  set status [$caennet SendCode $number 0x2]
  if {$status} {
    set msg "$this: error reading global parameters. Module number: $number. "
    append msg "Error code: $status"
		tk_messageBox -message $msg -icon error
		return $status
  }
}

#
#
# disable mux code = 0x20
#
#
#
itcl::body ACAENN568B::DisableMUX {} {
	set status [$caennet SendCode $number 0x20]
	if {$status} {
    set msg "$this: error disabling MUX. Module number: $number. "
    append msg "Error code: $status"
		tk_messageBox -message $msg -icon error
		return $status
	} 
}

#
#
# enable mux code = 0x21
#
#
itcl::body ACAENN568B::EnableMUX {} {
	set status [$caennet SendCode $number 0x21]
	if {$status} {
    set msg "$this: error enabling MUX. Module number: $number. "
    append msg "Error code: $status"
		tk_messageBox -message $msg  -icon error
		return $status
	}
}

#
#
# offset code = 0x16
#
#
itcl::body ACAENN568B::SetOffset {value} {
	set status [$caennet Send $number 0x16 $value]
	if {$status} {
    set msg "$this: error enabling MUX. Module number: $number. Error code: $status"
		tk_messageBox -message $msg -icon error 
		return $status
	} 
}

#
# 
#
#
#
itcl::body ACAENN568B::TestCommunication {} {
	set status [ReadOffset]
	if {$status != 0} {
		return $status
	} else {
		after $wait
		$caennet Receive
		return 0
	}
}

#
#
#
#
#
itcl::body ACAENN568B::Init {filename aname} {
	source $filename
	set status [TestCommunication]
	if {$status != 0} {
		tk_messageBox -message "$this: shaper not responding. Module number: $number. Error code: $status" -icon error
		return
	}
	foreach name [array names $aname] {
		set value [lindex [array get $aname $name] 1]
		if {![string is alpha $name]} {
			scan $name {%[a-z]%d} parameter channel
			SetParameter $channel $parameter $value
#			puts "Call to SetParameter on $this at channel $channel, parameter $parameter, value $value"
		}
		if {[string match $name offset]} {
			SetOffset $value
		}
	}
	DisableMUX
}

