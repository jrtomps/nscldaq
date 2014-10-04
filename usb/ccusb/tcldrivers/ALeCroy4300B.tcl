#===================================================================
# class ALeCroy4300B
#===================================================================
#
#
package provide lecroy4300b 11.0

package require Itcl



itcl::class ALeCroy4300B {
	private variable device ;#< a reference to a cccusb::CCCUSB object
	private variable node   ;#< slot in which the module resides
	private variable self   ;#< name of this instance
	
  ## @brief Constructor 
  #
  # Store references of controller, slot, and instance name.
  #
  # @param de   name of a cccusb::CCCUSB object
  # @param no   slot number
  #
  # @returns name of the new instance
	constructor {de no} {
		set device $de
		set node $no
		set self [string trimleft $this :]
	}
	
  ## @brief Destructor - a no-op
	destructor {}
	
  public method SetController {ctlr}
  public method GetController {}

	public method SetPedestals {peds}
	public method WritePedestals {}
	public method SetControlRegister {control}
	public method Clear {}

  #---------------------------------------------------------------------------#
  # Compound methods
  
  ## @brief Initialize the module 
  #
  # The following modes are set:
  # CAMAC LAM enabled; 
  # CAMAC sequential read; 
  # Pedestal subtraction enabled
  # 
	public method Init {} {
    SetControlRegister 0x7800
  }

# Stack methods
  
	public method sClear {stack}
	public method sReadHeader {stack}
	public method sReadChannel {stack channel}
	public method sReadSparse {stack}

  public method sSetPedestals {stack peds}
  public method sSetControlRegister {stack regval}

  ## @brief Process stack consisting of commands created by a method 
  #
  # Given a list whose first element is a proc name and subsequent elements are
  # arguments (i.e. procname arg0 arg1 arg2 ...) , this creates a stack
  #
  #  procname stack arg0 arg1 arg2 ...
  #  
  #  This is useful for converting the various stack building scripts into
  #  single-shot operations. For example the user can set the QDC gate width to
  #  200 by calling 
  #
  #  Execute [list sSetQDCGateWidth 200]
  #
  #  note that this is exactly how the single-shot operations are implemented.
  #
  #  Ultimately, this calls cccusb::CCCUSB::executeList, which returns a swig-
  #  wrapped vector<uint16_t>. This is converted into a standard tcl list but 
  #  the user needs to provide some info to help the conversion. A write command
  #  will cause there to only be 16-bits of data returned such that a list of
  #  32-bit integers cannot be formed by combining an even number of uint16_ts.
  #  However, any read24 command will return 32-bit numbers so that every other
  #  uint16_t should be combined into a single uint32_t integer. For this, the
  #  user must provide the grouping parameter. This specifies how many uint16_t
  #  should be combined to form an element in the resulting Tcl list. This
  #  should typically be 1 (writes) or 2 (reads).
  #
  #  @param grouping  how to group uint16_ts to form resulting tcl list 
  #  @param script    list of form {procname arg0 arg1 arg2 ...}
  #
  #  @returns resulting data from the stack execution as a tcl list of 32-bit words
  #
  public method Execute {grouping script} 
}

#--------------------------------------------------------------------
#
#

itcl::body ALeCroy4300B::SetController {ctlr} {
  set device $ctlr
}

itcl::body ALeCroy4300B::GetController {} {
  return $device
}

itcl::body ALeCroy4300B::SetPedestals {peds} {
  set res [catch {Execute 1 [list sSetPedestals $peds]} msg]
  if {$res} {
    return -code error [lreplace $msg 0 0 ALeCroy4300B::SetPedestals]
  }
  return $msg
}

itcl::body ALeCroy4300B::Clear {} {
  return [Execute 1 [list sClear]]
}

itcl::body ALeCroy4300B::WritePedestals {} {
	global SCINT_ENERGY
	if {![info exists SCINT_ENERGY]} {
		tk_messageBox -icon error -message "Cannot find array SCINT_ENERGY for $self"
		return
	}
	for {set i 0} {$i < 16} {incr i} {lappend peds $SCINT_ENERGY([format "ped%.2d" $i])}
	SetPedestals $peds
}

itcl::body ALeCroy4300B::SetControlRegister {regval} {
  return [Execute 1 [list sSetControlRegister $regval]]
}

itcl::body ALeCroy4300B::sClear {stack} {
  $stack addControl $node 0 9	
}

itcl::body ALeCroy4300B::sReadHeader {stack} {
  $stack addRead24 $node 0 2
}

itcl::body ALeCroy4300B::sReadChannel {stack channel} {
  $stack addRead24 $node $channel 2
}

itcl::body ALeCroy4300B::sReadSparse {stack} {
#	set A 0
#	set F 2
#	lappend command [expr 0x8000+($node<<9)+($A<<5)+$F]
#	lappend command [expr 0x8000+(1<<7)+(1<<4)]; # Wait for LAM; Q-stop mode;
#	lappend command 17; # No more than 17 times (header + 16 data words)
  $stack addQScan $node 0 2 17 1
#	$device AddToStack $stack $command
}

itcl::body ALeCroy4300B::sSetPedestals {stack peds} {
	if {[llength $peds] < 16} {
    set msg "ALeCroy4300B::sSetPedestals Fewer than 16 pedestal values provided."
		return -code error $msg
	}

	for {set i 0} {$i < 16} {incr i} {
		$stack addWrite24 $node $i 17 [lindex $peds $i]
	}
}


itcl::body ALeCroy4300B::sSetControlRegister {stack regval} {
  $stack addWrite24 $node 0 16 $regval
}

# A utility to facility single-shot operation evaluation
#
# Given a list whose first element is a proc name and subsequent elements are
# arguments (i.e. procname arg0 arg1 arg2 ...) , this creates a stack
#
#  procname stack arg0 arg1 arg2 ...
#  
#
itcl::body ALeCroy4300B::Execute {grouping script} {
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

    # the returned data is actually a std::vector<uin16_t> wrapped by swig. 
    # Convert this into a list of 32-bit integers and return it as a tcl list
    return [::CCUSBDriverSupport::shortsListToTclList data $grouping]
  } else { 
    set msg "ATrigget2367::Execute user must set the controller first with "
    append msg "SetController"
    return -code error $msg
  } 
}
