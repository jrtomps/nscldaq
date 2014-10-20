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
  }

  ## @brief Destructor - noop
  destructor {}


  ## @brief Read module identifier
  #
  public method ReadModuleIdentifier {} 

  ## @brief Read parameters for all channels + offset 
  #
  # Reads all info from the device besides mux status. This then allows the user
  # to run the 
  public method ReadAllParameters {}

  ## @brief Read offset from device 
  #
  # @returns offset value 
  #
  # @throws error if an nonzero error code is returned from V288 or N568B
  public method ReadOffset {}

  ## @brief Read all parameters for a channel + offset 
  #
  # @returns list of the following form
  #
  # @throws error if an nonzero error code is returned from V288 or N568B
  # 
  public method ReadChannelParameters {channel}

  public method ReadMUXStatusAndLastChAccess {}

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
  # @returns the error code response from slave 
  public method SetParameter {channel parameter value}

  ## @brief Sets the DC offset of the signal (shifts baseline of output)
  #
  # @param value  offset value in range [0,255]
  public method SetOffset {value}

  ## @brief Disable the multiplexed outputs
  #
  # @returns response from the slave
  public method DisableMUX {}

  ## @brief Enable multiplexed outputs
  #
  # @returns response from the slave 
  public method EnableMUX {}


  #############################################################################
  # Parsed method... interprets the data read by one of the reads and then 
  # returns the desired value
  public method ReadFineGain {channel}
  public method ReadCoarseGain {channel}
  public method ReadPoleZero {channel}
  public method ReadShape {channel}
  public method ReadOutputConfiguration {channel}
  public method ReadPolarity {channel}

  ## @brief Test that we can read from the module
  #
  # This actually performs a read of the offset value
  #
  # @returns slave response for the offset
  public method TestCommunication {}


  public method ParseChannelParams {dataList}


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
itcl::body ACAENN568B::ReadModuleIdentifier {} {
  set status [$caennet SendCode $number 0]
  if {$status} {
    set msg "$this: error reading channel parameters. Module number: $number. "
    append msg "Error code: $status"
    return -code error -errorinfo ACAENN568B::ReadModuleIdentifier $msg
  } else {
    set data [$caennet Receive]
    set trimmedData [lreplace $data 0 0] 
    return [binary format c* $trimmedData]
  }
}

#
#
#
itcl::body ACAENN568B::ReadAllParameters {} {
  after 75

  set status [$caennet SendCode $number 1]
  if {$status} {
    set msg "$this: error reading channel parameters. Module number: $number. "
    append msg "Error code: $status"
    return -code error -errorinfo ACAENN568B::ReadAllParameters $msg
  } else {
    return [$caennet Receive]
  }
}

#
#
#
itcl::body ACAENN568B::ReadOffset {} {
  after 75
  set status [$caennet SendCode $number 0x2]
  if {$status} {
    set msg "$this: error reading global parameters. Module number: $number. "
    append msg "Error code: $status"
    return -code error -errorinfo ACAENN568B::ReadOffset $msg
  }
  return [$caennet Receive]
}

#
#
#
itcl::body ACAENN568B::ReadChannelParameters {channel} {
  after 75
  if {![Utils::isInRange 0 15 $channel]} {
    set msg "$this: Channel $channel is out of range. "
    append msg {Must be in range [0,15].} 
    return -code error -errorinfo ACAENN568B::ReadChannelParameters $msg
  }


  # the manual is wrong. To read parameters for a single channel, the code is:
  # 0xn03, where n is the slot number. This differs from the manual's value 0x0n3.
  set code [expr ($channel<<8)| 0x3]
  set status [$caennet SendCode $number $code]
  if {$status} {
    set msg "$this: error reading channel parameters. Module number: $number. "
    append msg "Error code: $status"
    return -code error -errorinfo ACAENN568B::ReadChannelParameters $msg
  } 
  set data [$caennet Receive]
  set status [lindex $data 0]
  if {$status!=0} {
    set msg "$this: Bad response from N568B. Module number: $number."
    append msg [format "Error code=0x%x." $status]
    return -code error -errorinfo ACAENN568B::ReadChannelParameters $msg
  }
  return [ParseChannelParams [lreplace $data 0 0]]
}

#
#
# offset code = 0x004 
#
#
itcl::body ACAENN568B::ReadMUXStatusAndLastChAccess {} {
  after 75
  set status [$caennet SendCode $number 0x004]
  if {$status} {
    set msg "$this: error reading MUX status and last channel access. Module "
    append msg "number: $number. Error code: $status"
    return -code error -errorinfo ACAENN568B::ReadMUXStatusAndLastChAccess $msg 
  } 
  return [$caennet Receive]
}

# 
#
# In this method the parameter to be set is coded in index
itcl::body ACAENN568B::SetParameter {channel parameter value} {
  after 75
  # allowed parameters and their value upper bounds
  set plist {fgain cgain pzero shape polar out}
  set llist {255 7 255 3 1 1}

  # find the parameter in the list
  set index [lsearch $plist $parameter]
  if {$index == -1} {
    set msg "$this: invalid parameter ($parameter)"
    return -code error -errorinfo ACAENN568B::SetParameter $msg
  }

  # if here, the parameter name was valid...is the value provided in range?
  if {![Utils::isInRange 0 [lindex $llist $index] $value]} {
    set msg "$this: $parameter out of range ($value)"
    return -code error -errorinfo ACAENN568B::SetParameter $msg
  }

  # is the channel valid? 
  if {![Utils::isInRange 0 $channels $channel]} {
    set msg "$this: channel number out of range ($channel)"
    return -code error -errorinfo ACAENN568B::SetParameter $msg
  }

  # all arguments checked out...send the command for the channel
  set code [expr ($channel<<8) | 0x10 | $index]
  set status [$caennet Send $number $code $value]

  # handle the response
  if {$status} {
    set msg "$this: error setting $parameter. Module number: $number. "
    append msg "Error code: $status"
    return -code error -errorinfo ACAENN568B::SetParameter $msg
  } else {
    return [$caennet Receive] 
  }
}

#
# set offset code = 0x16
#
itcl::body ACAENN568B::SetOffset {value} {
  after 75
  set status [$caennet Send $number 0x16 $value]
  if {$status} {
    set msg "$this: error enabling MUX. Module number: $number. Error code: $status"
    return -code error -errorinfo ACAENN568B::SetOffset $msg
  } 
  return [$caennet Receive]
}

#
# disable mux code = 0x20
#
itcl::body ACAENN568B::DisableMUX {} {
  after 75
  set status [$caennet SendCode $number 0x20]
  if {$status} {
    set msg "$this: error disabling MUX. Module number: $number. "
    append msg "Error code: $status"
    return -code error -errorinfo ACAENN568B::DisableMUX $msg
  } 
  return $status 
}

#
#
# enable mux code = 0x21
#
#
itcl::body ACAENN568B::EnableMUX {} {
  after 75
  set status [$caennet SendCode $number 0x21]
  if {$status} {
    set msg "$this: error enabling MUX. Module number: $number. "
    append msg "Error code: $status"
    return -code error -errorinfo ACAENN568B::EnableMUX $msg
  }
  return $status 
}

#
# Value extracted from return value of ReadChannelParameters
#
itcl::body ACAENN568B::ReadFineGain {channel} {
  set data [ReadChannelParameters $channel]
  return [lindex $data 0]
}

#
# Value extracted from return value of ReadChannelParameters
#
itcl::body ACAENN568B::ReadCoarseGain {channel} {
  set data [ReadChannelParameters $channel]
  return [lindex $data 5]
}

#
# Value extracted from return value of ReadChannelParameters
#
itcl::body ACAENN568B::ReadPoleZero {channel} {
  set data [ReadChannelParameters $channel]
  return [lindex $data 1]
}

#
# Value extracted from return value of ReadChannelParameters
#
itcl::body ACAENN568B::ReadShape {channel} {
  set data [ReadChannelParameters $channel]
  return [lindex $data 4]
}

#
# Value extracted from return value of ReadChannelParameters
#
itcl::body ACAENN568B::ReadOutputConfiguration {channel} {
  set data [ReadChannelParameters $channel]
  return [lindex $data 3] 
}


#
# Value extracted from return value of ReadChannelParameters
#
itcl::body ACAENN568B::ReadPolarity {channel} {
  set data [ReadChannelParameters $channel]
  return [lindex $data 2] 
}


#
#
#
itcl::body ACAENN568B::TestCommunication {} {
  set status [ReadOffset]
  if {$status != 0} {
    return $status
  } else {
    $caennet Receive
    return 0
  }
}

itcl::body ACAENN568B::ParseChannelParams {dataList} {
  if {[llength $dataList]!=3} {
    set msg "Argument list does not contain 3 elements."
    return -code error -errorinfo ACAENN568B::ParseChannelParams $msg
  }
  
  set result [list]
  lappend result [lindex $dataList 0]  ;# fine gain
  lappend result [lindex $dataList 1]  ;# pole zero

  set status [lindex $dataList 2]
  lappend result [expr ($status&0x40)>>6]; # output polarity
  lappend result [expr ($status&0x20)>>5]; # output configuration
  lappend result [expr ($status&0x18)>>3]; # shape
  lappend result [expr ($status&0x7)];     # coarse gain

  return $result
}

#
#
#
itcl::body ACAENN568B::Init {filename aname} {
  source $filename
  set status [TestCommunication]
  if {$status != 0} {
    set msg "$this: shaper not responding. Module number: $number. "
    append msg "Error code: $status"
    puts stderr $msg
    return $status
  }
  foreach name [array names $aname] {
    set value [lindex [array get $aname $name] 1]
    if {![string is alpha $name]} {
      scan $name {%[a-z]%d} parameter channel
      SetParameter $channel $parameter $value
    }
    if {[string match $name offset]} {
      SetOffset $value
    }
  }
  DisableMUX
}

