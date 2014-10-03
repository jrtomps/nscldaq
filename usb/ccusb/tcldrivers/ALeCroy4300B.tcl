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
	public method SetControlRegister {control} {CCUSBWrite $device $node 0 16 $control}
# CAMAC LAM enabled; CAMAC sequential read; Pedestal subtraction enabled
	public method Init {} {SetControlRegister 0x7800}
	public method Clear {} {CCUSBRead $device $node 0 9}

# Stack methods
  
	public method sClear {stack}
	public method sReadHeader {stack}
	public method sReadChannel {stack channel}
	public method sReadSparse {stack}
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
	if {[llength $peds] < 16} {
		tk_messageBox -icon error -message "Unsufficient number of pedestals in \n WritePedestals of $self"
		return
	}
	for {set i 0} {$i < 16} {incr i} {
		CCUSBWrite $device $node $i 17 [lindex $peds $i]
	}
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

itcl::body ALeCroy4300B::sClear {stack} {
	set A 0
	set F 9
	lappend command [expr ($node<<9)+($A<<5)+$F]
	$device AddToStack $stack $command
}

itcl::body ALeCroy4300B::sReadHeader {stack} {
	set A 0
	set F 2
	lappend command [expr ($node<<9)+($A<<5)+$F]
	$device AddToStack $stack $command
}

itcl::body ALeCroy4300B::sReadChannel {stack channel} {
	set A $channel
	set F 2
	lappend command [expr ($node<<9)+($A<<5)+$F]
	$device AddToStack $stack $command
}

itcl::body ALeCroy4300B::sReadSparse {stack} {
	set A 0
	set F 2
	lappend command [expr 0x8000+($node<<9)+($A<<5)+$F]
	lappend command [expr 0x8000+(1<<7)+(1<<4)]; # Wait for LAM; Q-stop mode;
	lappend command 17; # No more than 17 times (header + 16 data words)
	$device AddToStack $stack $command
}
