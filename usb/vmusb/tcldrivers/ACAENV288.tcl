#===================================================================
# class ACAENV288
#===================================================================
itcl::class ACAENV288 {
	protected variable device
	private variable base
	private variable status
	private variable transmit
	private variable reset

	constructor {de ba} {
		set device $de
		set base $ba
		set status [expr $base+0x2]
		set transmit [expr $base+0x4]
		set reset [expr $base+0x6]
	}
	
	destructor {}
	
	public method GetVariable {v} {set $v}
	public method Reset {} {$device Write24D16 $reset 1; after 5}
	public method GetStatus {} {return [expr [$device Read24D16 $status]&0x1]}
	public method TransmitData {}
	public method WriteTransmitBuffer {word}
	public method Receive {}
	public method ReceiveError {}
	public method Send {slave code value}
	public method SendCode {slave code}
	public method SendAll {slave code values nval}
}

itcl::body ACAENV288::TransmitData {} {
	$device Write24D16 $transmit 1
	if {[GetStatus]} {
		tk_messageBox -message "CAEN V288: error starting data packet transmission" -icon error
	}
}

itcl::body ACAENV288::WriteTransmitBuffer {word} {
	$device Write24D16 $base $word
	if {[GetStatus]} {
		tk_messageBox -message "CAEN V288: error writing data into transmit data buffer ($word)" -icon error
	}
}

itcl::body ACAENV288::Receive {} {
	set timeout 1000
	while {$timeout > 0 && [GetStatus] != 0} {
		set rdb [$device Read24D16 $base]
		incr timeout -1
	}
	if {$timeout == 0} {
		tk_messageBox -message "CAEN V288: timeout while receiving data" -icon error
		return 1
	}
	if {[info exists rdb]} {
		lappend buffer $rdb
		incr nwords
	}
	for {set i 0} {$i < 255} {incr i} {
		set rdb [$device Read24D16 $base]
		if {![GetStatus]} {
			lappend buffer $rdb
			incr nwords
		} else {break}
	}
	if {[info exists buffer]} {
		set buffer [linsert $buffer 0 $nwords]
		return $buffer
	} else {
		return 0
	}
}

itcl::body ACAENV288::ReceiveError {} {
	set timeout 1000
	while {$timeout > 0 && [GetStatus] != 0} {
		set rdb [$device Read24D16 $base]
		incr timeout -1
	}
	if {$timeout == 0} {
		tk_messageBox -message "CAEN V288: timeout while receiving data" -icon error
		return 1
	}
	if {[info exists rdb]} {return $rdb} else {return 0}
}

itcl::body ACAENV288::Send {slave code value} {
	if {$slave < 0 || $slave > 99} {
		tk_messageBox -message "CAEN V288: slave address code out of range" -icon error
		return 1
	}
	WriteTransmitBuffer 0x1
	WriteTransmitBuffer $slave
	WriteTransmitBuffer $code
	WriteTransmitBuffer $value
	TransmitData
	return [ReceiveError]
}

itcl::body ACAENV288::SendCode {slave code} {
	if {$slave < 0 || $slave > 99} {
		tk_messageBox -message "CAEN V288: slave address code out of range" -icon error
		return 1
	}
	WriteTransmitBuffer 0x1
	WriteTransmitBuffer $slave
	WriteTransmitBuffer $code
	TransmitData
	return [ReceiveError]
}

itcl::body ACAENV288::SendAll {slave code values nval} {
	if {$slave < 0 || $slave > 99} {
		tk_messageBox -message "CAEN V288: slave address code out of range" -icon error
		return 1
	}
	if {$nval < 0 || $nval > 253} {
		tk_messageBox -message "CAEN V288: number of set values out of range" -icon error
		return 1
	}
	WriteTransmitBuffer 0x1
	WriteTransmitBuffer $slave
	WriteTransmitBuffer $code
	for {set i 0} {$i < $nval} {incr i} {
		WriteTransmitBuffer [lindex $values $i]
	}
	TransmitData
	return [ReceiveError]
}
