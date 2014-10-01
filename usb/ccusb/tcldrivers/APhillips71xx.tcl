#===================================================================
# class APhillips71xx
#===================================================================
#

package provide phillips71xx 11.0

package require Itcl
package require cccusbreadoutlist

itcl::class APhillips71xx {
	private variable device   ;#< reference to USB controller
	private variable node     ;#< slot number (i.e. N)
	
  ## @brief Constructor
  #
  # @param de   a cccusb::CCCUSB object
  # @param no   the slot in which the module resides
  #
	constructor {de no} {
		set device $de
		set node $no
	}
	
	destructor {}
	
  # ---------------------------------------------------------------------------
  # Single-shot commands

  ## @brief Clear hit register, LAM, and data memory
  #
	public method Clear {} { Execute sClear }

  ## @brief Enables LAM
  #
  # LAM is enabled on the S1 strobe.
  #
	public method EnableLAM {} { Execute sEnableLAM }

  ## @brief Disables LAM
  #
  # LAM is disabled on the S2 strobe.
  #
	public method DisableLAM {} { Execute sDisableLAM }

  ## @brief Write value to the control register
  #
  # Control register bits are as follows:
  #   bit 0    :   PED enable
  #   bit 1    :   Lower threshold enable
  #   bit 2    :   Upper threshold enable
  #   bits 7-3 :   Not meaningful and should be left as 0
  #   bits 15-8:   Conversion delay (read-only)
  #
  #   @param reg  value to write to register (should only use bits 0-2).
  #
	public method SetControlRegister {reg} { 
    Execute [list sSetControlRegister $reg]
  }

  ## @brief Reset the control register bits to a value
  #
  # @see SetControlRegister for the meaning of bits in the control register
  #
  # @param reg  value to write to register (should only use bits 0-2).
	public method ResetControlRegister {reg} { 
    Execute [list sResetControlRegister $reg]
  }

  ## @brief Read the control register value
  #
  # This not only contains the information for the ped, lower thresh, and upper
  # thresh, but also will contain info on the conversion dealy in bits 8-15.
  #
  #  @returns register value with QX encoded in bits 24 and 24.
  #
	public method ReadControlRegister {} {
    return [Execute [list sReadControlRegister]]
  }

  ## @brief Read the hit register data
  #
  # Set bits correspond to channels whose pedestal corrected values fall within
  # their upper and lower thresholds
  #
  # @returns the value of the hit register
  #
	public method ReadHitRegister {} {return [Execute [list sReadHitRegister]]}

  ## @brief Read the data associated with a channel
  #
  # Reads the available data for a channel. 
  #
  # @param channel  the channel whose data is being requested
  #
  # @returns the value read back and the QX bits encoded in bits 24 and 25 
  #
	public method ReadChannel {channel} { return [Execute [list sReadChannel]]}

  ## @brief Waits for LAM and then reads the data associated with a channel 
  #
  # Reads the available data for a channel once a LAM is made available from
  # the module.
  #
  # @param channel  the channel whose data is being requested
  #
  # @returns the value read back and the QX bits encoded in bits 24 and 25 
  #
	public method ReadChannelLAM {channel} { 
    return [Execute [list sReadChannelLAM]]
  }

  ## @brief Immediately reads only the channels with "interesting" data 
  #
  # The channels with pedestal-corrected values above the lower and below the
  # upper threshold are read out and returned. These are the channels that
  # satisfy the characteristics defined by the user as worth being read out. 
  #
  # The data words returned have the channel encoded into bits 12-15 and the
  # 12-bit value in bits 0-11.
  #
  # @returns a list of the channel data
  #
	public method ReadSparse {}



  #----------------------------------------------------------------------------#
  # Complex commands composed of other simple commands 
  #
  #
  #
  
  ## @brief Write the list of pedestals to the module
  #
  # This writes a list of pedestals to the module and ensures that they are set
  # properly by reading back the values written. Pedestal values are interpreted
  # as signed values.
  #
  # @param peds   a list of 16 pedestal values (must be 16 elements long)
  #
  # @throws error under following conditions
  #   - User a list with fewer than 16 pedestal values
  #   - Read-back of value failed to agree with the value written
  #
  public method WritePedestals {peds}

  ## @brief Write the list of lower threshold values to the module
  #
  # This writes a list of lower thresholds to the module and ensures that they
  # are set properly by reading back the values written. Lower threshold values
  # are interpreted as signed values. The first 16 threshold values in the list
  # are used and any remaining are ignored.
  #
  # @param llt  a list of at least 16 threshold values 
  #
  # @throws error under following conditions - User a list with fewer than 16
  # threshold values - Read-back of value failed to agree with the value written
  #
	public method WriteLowerThresholds {llt}

  ## @brief Write the list of upper threshold values to the module
  #
  # This writes a list of upper thresholds to the module and ensures that they
  # are set properly by reading back the values written. Upper threshold values
  # are interpreted as signed values. The first 16 threshold values in the list
  # are used and any remaining are ignored.
  #
  # @param ult  a list of at least 16 threshold values 
  #
  # @throws error under following conditions - User a list with fewer than 16
  # threshold values - Read-back of value failed to agree with the value written
  #
	public method WriteUpperThresholds {ult}


  #----------------------------------------------------------------------------#
  # Stack building methods
  #

  ## @brief Add command to stack to clear hit mask, LAM, and data memory
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object
	public method sClear {stack}

  ## @brief Add command to stack to enable the LAM
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object
  public method sEnableLAM {stack}

  ## @brief Add command to stack to disable the LAM
  # @param 
  public method sDisableLAM {stack}

  ## @brief Add command to stack to write value to the control register
  # 
  #  For the meaning of bits in the control register @see SetControlRegister
  #
  #  @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
  public method sSetControlRegister {stack reg}

  ## @brief Add command to stack to rewrite in the control register
  # 
  #  For the meaning of bits in the control register @see SetControlRegister
  #
  #  @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
  public method sResetControlRegister {stack reg}

  ## @brief Add command to stack to read the control register
  # 
  #  @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
  public method sReadControlRegister {stack}

  ## @brief Add command to stack to read the hit register
  #
  #  @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
  public method sReadHitRegister {stack}

  ## @brief Add command to stack to read channel data immediately 
  #
  #  @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
	public method sReadChannel {stack channel}

  ## @brief Add command to stack to read channel data after LAM is present 
  #
  #  @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
	public method sReadChannelLAM {stack channel}



  ## @brief Add commands to stack to read all data with meaningful values
  #
  # This uses the value returned from the hit register to determine which
  # channels have data between the lower and upper thresholds following
  # pedestal substraction. These channels are then read out from the module.
  #
  #  @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
	public method sReadSparse {stack}

  ## @brief Add commands to stack to read all data with significant values
  #
  # This is the very same thing as the sReadSparse method except that it waits
  # for the LAM to be present.
  #
  #  @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
	public method sReadSparseLAM {stack}




  # --------------------------------------------------------------------------#
  #
  # Utility methods



  ## @brief Process stack consisting of commands created by a method 
  #
  # Given a list whose first element is a proc name and subsequent elements are
  # arguments (i.e. procname arg0 arg1 arg2 ...) , this creates a stack
  #
  #  procname stack arg0 arg1 arg2 ...
  #  
  #  This is useful for converting the various stack building scripts into
  #  single-shot operations. For example the user can read the control register
  #  by calling
  #
  #  $obj Execute [list sReadControlRegister]
  #
  #  note that this is exactly how the single-shot operations are implemented.
  #
  #  @param script  list of form {procname arg0 arg1 arg2 ...}
  #
  #  @returns resulting data from the stack execution
  #
  private method Execute {script}

} ;# End of the APhillips71xx




itcl::body APhillips71xx::ReadChannel {channel} {
	set data [Execute [list sReadChannel $channel]]
	return [expr {$data & 0xfff}]
}

itcl::body APhillips71xx::ReadChannelLAM {channel} {
  set data [Execute [list sReadChannelLAM $channel]]
	return $data
}



#-----------------------------------------------------------------------------#
#
# Composite methods


#
# Write the pedestals and ensure they get set
# 
itcl::body APhillips71xx::WritePedestals {peds} {
	if {[llength $peds] < 16} {
    set msg "Insufficient number of pedestals in \n WritePedestals of $this"
 		tk_messageBox -icon error -message $msg
		return
	}

  # set the device into pedestal setting mode
	$device simpleWrite24 $node 0 17 0
	for {set i 0} {$i < 16} {incr i} {

    # ensure that this is valid to begin with
    if {([lindex $ped $i]>4095) || ([lindex $ped $i]< -4095)} {
      set msg "APhillips71xx::WritePedestals pedestal at index $i out of range."
      append msg { Must be in range [-4095,4095].}
      return -code error $msg
    }

    # write pedestal for the ith channel
		$device simpleWrite24 $node $i 20 [lindex $peds $i]

    # read back the value and verify that it is correct
    set check [$device simpleRead24 $node $i 1]

		if {![expr $check == [lindex $peds $i]]} {
      set msg "Error while writing pedestal $i to $this\n($check should be "
      append msg "[lindex $peds $i])"
			tk_messageBox -icon error -message $msg
			return
		}
	}
}

#
#
# Write the lower thresholds and ensure they get set
# 
itcl::body APhillips71xx::WriteLowerThresholds {llt} {
  if {[llength $llt] < 16} {
    set msg "Unsufficient number of thresholds in \n WriteLowerThresholds of "
    append msg "$this"
    tk_messageBox -icon error -message $msg
    return
  }

  #ensure device is in lower threshold setting mode
  $device simpleWrite24 $node 1 17 0

  for {set i 0} {$i < 16} {incr i} {

    # ensure that the threshold is valid to begin with
    if {([lindex $llt $i]>4095) || ([lindex $llt $i]<0)} {
      set msg "APhillips71xx::WriteLowerThresholds threshold at index $i "
      append msg { is out of range. Must be in range [0,4095].}
      return -code error $msg
    }

    $device simpleWrite24 $node $i 20 [lindex $llt $i]
    set check [expr [$device simpleRead24 $node $i 1]&0xfff]
    if {![expr $check == [lindex $llt $i]]} {
      set msg "Error while writing lower threshold $i to $this"
      tk_messageBox -icon error -message $msg
      return
    }
  }
}

#
# Write the upper thresholds and ensure they get set
# 
itcl::body APhillips71xx::WriteUpperThresholds {ult} {
  if {[llength $ult] < 16} {
    set msg "Unsufficient number of thresholds in \n WriteUpperThresholds "
    append msg "of $this"
    tk_messageBox -icon error -message $msg
    return
  }

  # put device into upper threshold setting mode
  $device simpleWrite24 $node 2 17 0

  for {set i 0} {$i < 16} {incr i} {

    # ensure that the threshold is valid to begin with
    if {([lindex $ult $i]>4095) || ([lindex $ult $i]<0)} {
      set msg "APhillips71xx::WriteUpperThresholds threshold at index $i "
      append msg { is out of range. Must be in range [0,4095].}
      return -code error $msg
    }

    $device simpleWrite24 $node $i 20 [lindex $ult $i]
    set check [expr [$device simpleRead24 $node $i 1]&0xfff]
    if {![expr $check == [lindex $ult $i]]} {
      set msg "Error while writing upper threshold $i to $this"
      tk_messageBox -icon error -message $msg
      return
    }
  }
}


#-----------------------------------------------------------------------------#
#
# Stack building methods

#
#
#
itcl::body APhillips71xx::sClear {stack} {
  $stack addControl $node 3 11 
}


itcl::body APhillips71xx::sEnableLAM {stack} {
  $stack addControl $node 0 26 
}


itcl::body APhillips71xx::sDisableLAM {stack} {
  $stack addControl $node 0 24
}


itcl::body APhillips71xx::sSetControlRegister {stack reg} {
  $stack addWrite24 $node 0 19 $reg
}


itcl::body APhillips71xx::sResetControlRegister {stack reg} {
  $stack addWrite24 $node 0 23 $reg
}


itcl::body APhillips71xx::sReadControlRegister {stack} {
  $stack addRead24 $node 0 6 
}


itcl::body APhillips71xx::sReadHitRegister {stack} {
  $stack addRead24 $node 1 6 
}


itcl::body APhillips71xx::sReadChannel {stack channel} {
	$stack addRead24 $node $channel 0
}

itcl::body APhillips71xx::sReadChannelLAM {stack channel} {
	$stack addRead24 $node $channel 0 1
}

itcl::body APhillips71xx::ReadSparse {} {
  return [Execute [list sReadSparse]]
}

itcl::body APhillips71xx::sReadSparse {stack} {
  $stack addAddressPatternRead16 $node 1 6 
  sReadChannel $stack 0 
}

itcl::body APhillips71xx::sReadSparseLAM {stack} {
  set lamWait 1
  $stack addAddressPatternRead16 $node 1 6 $lamWait
  sReadChannel $stack 0
}


# A utility to facility single-shot operation evaluation
#
# Given a list whose first element is a proc name and subsequent elements are
# arguments (i.e. procname arg0 arg1 arg2 ...) , this creates a stack
#
#  procname stack arg0 arg1 arg2 ...
#  
#
itcl::body APhillips71xx::Execute {script} {

#ensure there is a device to execute the readout list
  if {$device ne ""} {

    # create a new readout list
    set rdoList [cccusbreadoutlist::CCCUSBReadoutList %AUTO%]

    # extract the proc we want to use to manipulate the stack
    set cmd [lindex $script 0]

    # if there are arguments provided, use them. otherwise, dont.
    if {[llength $script]>1} { 
      $cmd $rdoList {*}[lreplace $script 0 0] 
    } else {
      $cmd $rdoList 
    }

    # At this point the list will contain some commands added by the cmd
    # command

    # execute the list
    set data [$device executeList $rdoList [expr 4<<20]] 
  } else { 
    set msg "APhillips71xx::Execute user must set the controller first with "
    append msg "SetController"
    return -code error $msg
  } 
}
