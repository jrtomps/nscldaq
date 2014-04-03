#===================================================================
# class ACrdcXLM72
#===================================================================
itcl::class ACrdcXLM72 {
	inherit AXLM72

	constructor {de sl} {
		AXLM72::constructor $de $sl
	} {}

	public method WriteSamples {sa} {Write fpga 4 $sa}
	public method WritePeriod {pe} {Write fpga 12 $pe}
	public method WriteDelay {de} {Write fpga 16 $de}
	public method WriteWidth {wi} {Write fpga 20 $wi}
	public method WriteShift {sh} {Write fpga 24 $sh}
	public method WriteThresholds {th}
	public method Clear {} {Write srama 0 0}
	public method Init {filename array}
	public method sReadAll {stack}
}

itcl::body ACrdcXLM72::WriteThresholds {th} {
# if the th list contains less than 256 values, pad it with 1023 (10 bit max)
	if {[llength $th] < 256} {
		for {set i 0} {$i < 256-[llength $th]} {incr i} {lappend th 1023}
	}
# now write thresholds to RAM block of FPGA
	for {set i 0} {$i < 64} {incr i} {
		Write fpga 40 $i; # set RAM address
		Write fpga 44 [lindex $th $i]; # set connector 0 threshold register
		Write fpga 48 [lindex $th [expr $i+64]]; # set connector 1 threshold register
		Write fpga 52 [lindex $th [expr $i+128]]; # set connector 2 threshold register
		Write fpga 56 [lindex $th [expr $i+192]]; # set connector 3 threshold register
		Write fpga 60 1; # toggle WE of RAM (write RAM)
		Write fpga 60 0; # toggle back
		Write fpga 64 1; # enable RAM address for read
		Write fpga 72 0; # read RAM into registers
		Write fpga 64 0; # disable RAM address for read
		for {set c 0} {$c < 4} {incr c} {
			set check [Read fpga [expr 44+$c*4]]
			if {$check != [lindex $th [expr $i+$c*64]]} {
				tk_messageBox -icon error -message "Failed to set threshold in XLM72V of [$this GetVariable self]: $check vs [lindex $th [expr $i+$c*64]]"
			}
		}
	}
}

# This method assumes filename points to an "old" type Tcl file defining parameters
# in an array called "aname"
itcl::body ACrdcXLM72::Init {filename aname} {
	source $filename
	AccessBus 0x10001
	WriteSamples [lindex [array get $aname samples] 1]
	WritePeriod [lindex [array get $aname period] 1]
	WriteDelay [lindex [array get $aname delay] 1]
	WriteWidth [lindex [array get $aname width] 1]
	WriteShift [lindex [array get $aname shift] 1]
	for {set i 0} {$i < 256} {incr i} {
		lappend th [lindex [array get $aname [format thresholds%.3d $i]] 1]
	}
	WriteThresholds $th
	Clear
	ReleaseBus
#	Write vme 0x20000 0x824; # Clear mail register
}

itcl::body ACrdcXLM72::sReadAll {stack} {
	sAccessBus $stack 0x1
# Special NBLT read mode where address 0 of SRAMA contains the length of the subsequent block transfer
#	sReadNBLT $stack srama 0 0xffc srama 4; # mask 0x1ffc is for 8191 bytes max converted to 32 bit words (last 2 bits are 0)
# Clear first memory slot of SRAMA which contains the number of bytes to read
#	sWrite $stack srama 0 0
	sReadNBLT $stack vme 0x20824 0xffc srama 4; # mask 0x1ffc is for 8191 bytes max converted to 32 bit words (last 2 bits are 0)
#	sWrite $stack vme 0x20000 0x824; # Clear mail register
	sReleaseBus $stack
}

