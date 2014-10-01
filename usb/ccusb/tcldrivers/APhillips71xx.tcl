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
	private variable channel  
	private variable hits
	
  ## @brief Constructor
  #
  # @param de   a cccusb::CCCUSB object
  # @param no   the slot in which the module resides
  #
	constructor {de no} {
		set device $de
		set node $no
		for {set i 0} {$i < 16} {incr i} {set channel($i) 0}
		set hits 0
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
	public method SetControlRegister {reg} { Execute [list sSetControlRegister $reg]}

  ## @brief Reset the control register bits to a value
  #
  # @see SetControlRegister for the meaning of bits in the control register
  #
  # @param reg  value to write to register (should only use bits 0-2).
	public method ResetControlRegister {reg} { Execute [list sResetControlRegister $reg]}

  ## @brief Read the control register value
  #
  # This not only contains the information for the ped, lower thresh, and upper
  # thresh, but also will contain info on the conversion dealy in bits 8-15.
  #
  #  @returns register value with QX encoded in bits 24 and 24.
  #
	public method ReadControlRegister {} {return [Execute [list sReadControlRegister]]}

  ## @brief Read the hit register data
  #
  # Set bits correspond to channels whose pedestal corrected values fall within
  # their upper and lower thresholds
  #
  # @returns the value of the hit register
  #
	public method ReadHitRegister {} {return [Execute [list sReadHitRegister]]}

	public method ReadChannel {channel} { return [Execute [list sReadChannel]]}

	public method ReadChannelLAM {channel} { 
    return [Execute [list sReadChannelLAM]]
  }

  #----------------------------------------------------------------------------#
  # Complex commands composed of other simple commands 
  #
	public method WritePedestals {peds}
	public method WriteLowerThresholds {llt}
	public method WriteUpperThresholds {ult}


  #----------------------------------------------------------------------------#
  # Stack building methods
  #

	public method sClear {stack}
  public method sEnableLAM {stack}
  public method sDisableLAM {stack}
  public method sSetControlRegister {stack reg}
  public method sResetControlRegister {stack reg}
  public method sReadControlRegister {stack}
  public method sReadHitRegister {stack}
	public method sReadChannel {stack channel}
	public method sReadChannelLAM {stack channel}
	public method ReadSparse {}
	public method sReadSparse {stack}
	public method sReadSparseLAM {stack}


  private method Execute {script}
}


itcl::body APhillips71xx::ReadChannel {channel} {
	set data [Execute [list sReadChannel $channel]]
	return [expr {$data & 0xfff}]
}

itcl::body APhillips71xx::ReadChannelLAM {channel} {
  set data [Execute [list sReadChannelLAM $channel]]
	return $data
}


itcl::body APhillips71xx::WritePedestals {peds} {
	if {[llength $peds] < 16} {
    set msg "Insufficient number of pedestals in \n WritePedestals of $this"
 		tk_messageBox -icon error -message $msg
		return
	}
	$device simpleWrite24 $node 0 17 0
	for {set i 0} {$i < 16} {incr i} {
		$device simpleWrite24 $node $i 20 [lindex $peds $i]
		if {[lindex $peds $i] > 0x8000} {
			set check [$device simpleRead24 $node $i 1]
		} else {
			set check [expr [$device simpleRead24 $node $i 1]&0xfff]
		}
		if {![expr $check == [lindex $peds $i]]} {
      set msg "Error while writing pedestal $i to $this\n($check should be "
      append msg "[lindex $peds $i])"
			tk_messageBox -icon error -message $msg
			return
		}
	}
}

itcl::body APhillips71xx::WriteLowerThresholds {llt} {
	if {[llength $llt] < 16} {
    set msg "Unsufficient number of thresholds in \n WriteLowerThresholds of $this"
		tk_messageBox -icon error -message $msg
		return
	}
	$device simpleWrite24 $node 1 17 0
	for {set i 0} {$i < 16} {incr i} {
		$device simpleWrite24 $node $i 20 [lindex $llt $i]
		set check [expr [$device simpleRead24 $node $i 1]&0xfff]
		if {![expr $check == [lindex $llt $i]]} {
      set msg "Error while writing lower threshold $i to $this"
			tk_messageBox -icon error -message $msg
			return
		}
	}
}

itcl::body APhillips71xx::WriteUpperThresholds {ult} {
	if {[llength $ult] < 16} {
    set msg "Unsufficient number of thresholds in \n WriteUpperThresholds "
    append msg "of $this"
		tk_messageBox -icon error -message $msg
		return
	}
	$device simpleWrite24 $node 2 17 0
	for {set i 0} {$i < 16} {incr i} {
		$device simpleWrite24 $node $i 20 [lindex $ult $i]
		set check [expr [$device simpleRead24 $node $i 1]&0xfff]
		if {![expr $check == [lindex $ult $i]]} {
      set msg "Error while writing upper threshold $i to $this"
			tk_messageBox -icon error -message $msg
			return
		}
	}
}



itcl::body APhillips71xx::sClear {stack} {
	set A 3
	set F 11
  $stack addControl $node $A $F
}


itcl::body APhillips71xx::sEnableLAM {stack} {
	set A 0
	set F 26 
  $stack addControl $node $A $F
}


itcl::body APhillips71xx::sDisableLAM {stack} {
	set A 0
	set F 24 
  $stack addControl $node $A $F
}


itcl::body APhillips71xx::sSetControlRegister {stack reg} {
	set A 0
	set F 19 
  $stack addWrite24 $node $A $F $reg
}


itcl::body APhillips71xx::sResetControlRegister {stack reg} {
	set A 0
	set F 23 
  $stack addWrite24 $node $A $F $reg
}


itcl::body APhillips71xx::sReadControlRegister {stack} {
	set A 0
	set F 6 
  $stack addRead24 $node $A $F
}


itcl::body APhillips71xx::sReadHitRegister {stack} {
	set A 1
	set F 6 
  $stack addRead24 $node $A $F
}


itcl::body APhillips71xx::sReadChannel {stack channel} {
	set A $channel
	set F 0
	$stack addRead24 $node $A $F 
}

itcl::body APhillips71xx::sReadChannelLAM {stack channel} {
	set A $channel
	set F 0
	$stack addRead24 $node $A $F 1
}

itcl::body APhillips71xx::ReadSparse {} {
# First read hit register
	set A 1
	set F 6
# Use the Address Pattern feature of the CC-USB
	lappend command [expr 0x8000+($node<<9)+($A<<5)+$F]
	lappend command [expr (1<<9)]; # Address Pattern bit
	set A 0
	set F 0
	lappend command [expr ($node<<9)+($A<<5)+$F]
	return [XXUSBExecuteStack $device $command]
}

itcl::body APhillips71xx::sReadSparse {stack} {
# First read hit register
	set A 1
	set F 6
# Use the Address Pattern feature of the CC-USB
	lappend command [expr 0x8000+($node<<9)+($A<<5)+$F]
	lappend command [expr (1<<9)]; # Address Pattern bit
	set A 0
	set F 0
	lappend command [expr ($node<<9)+($A<<5)+$F]
	$device AddToStack $stack $command
}

itcl::body APhillips71xx::sReadSparseLAM {stack} {
# First read hit register
	set A 1
	set F 6
# Use the Address Pattern feature of the CC-USB
	lappend command [expr 0x8000+($node<<9)+($A<<5)+$F]
	lappend command [expr (1<<9)+(1<<7)]; # Address Pattern bit + LAM
	set A 0
	set F 0
	lappend command [expr ($node<<9)+($A<<5)+$F]
	$device AddToStack $stack $command
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
