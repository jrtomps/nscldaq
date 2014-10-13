#===================================================================
# class ACAENN568B
#===================================================================
itcl::class ACAENN568B {
	private variable caennet
	private variable number
	private variable channels
	private variable wait
	
	constructor {caen num} {
		set caennet $caen
		set number $num
		set channels 16
		set wait 20
	}
	
	destructor {}
	
	public method GetVariable {v} {set $v}
	public method SetParameter {channel parameter value}
	public method DisableMUX {}
	public method EnableMUX {}
	public method SetOffset {value}
	public method TestCommunication {}
	public method Init {filename aname}
}

# In this method the parameter to be set is coded in index
itcl::body ACAENN568B::SetParameter {channel parameter value} {
	set plist {fgain cgain pzero shape out polar}
	set llist {255 7 255 3 1 1}
	set index [lsearch $plist $parameter]
	if {$index == -1} {
		tk_messageBox -message "$this: invalid parameter ($parameter)" -icon error
		return
	}
	if {$channel < 0 || $channel > $channels} {
		tk_messageBox -message "$this: channel number out of range ($channel)" -icon error
		return
	}
	if {$value < 0 || $value > [lindex $llist $index]} {
		tk_messageBox -message "$this: $parameter out of range ($value)" -icon error
		return
	}
	set status [$caennet Send $number [expr $channel*0x100 + 0x10 + $index] $value]
	if {$status} {
		tk_messageBox -message "$this: error setting $parameter. Module number: $number. Error code: $status" -icon error
		return $status
	} else {
		after $wait
		$caennet Receive
		return 0
	}
}

itcl::body ACAENN568B::DisableMUX {} {
	set status [$caennet SendCode $number 0x20]
	if {$status} {
		tk_messageBox -message "$this: error disabling MUX. Module number: $number. Error code: $status" -icon error
		return $status
	} else {
		after $wait
		$caennet Receive
		return 0
	}
}

itcl::body ACAENN568B::EnableMUX {} {
	set status [$caennet SendCode $number 0x21]
	if {$status} {
		tk_messageBox -message "$this: error enabling MUX. Module number: $number. Error code: $status" -icon error
		return $status
	} else {
		after $wait
		$caennet Receive
		return 0
	}
}

itcl::body ACAENN568B::SetOffset {value} {
	set status [$caennet Send $number 0x16 $value]
	if {$status} {
		tk_messageBox -message "$this: error enabling MUX. Module number: $number. Error code: $status" -icon error
		return $status
	} else {
		after $wait
		$caennet Receive
		return 0
	}
}

itcl::body ACAENN568B::TestCommunication {} {
	set status [$caennet SendCode $number 0x2]
	if {$status != 0} {
		return $status
	} else {
		after $wait
		$caennet Receive
		return 0
	}
}

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

