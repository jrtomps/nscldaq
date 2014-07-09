#===================================================================
# class CCUSB
#===================================================================
itcl::class CCUSB {
	inherit XXUSB
	
	private variable firmware
	
	constructor {} {
		XXUSB::constructor
	} {}

	public method GetFirmware {}
	public method ReadRegister {reg}
	public method WriteRegister {reg data}
	public method SetRegister {reg data}
	public method SetBufferLength {length}
	public method SetMixedBuffer {value}
	public method SetForceDump {value}
	public method SetLatchEvent {value}
	public method SetHeader {value}
#	public method SetRepeatDelay {value}
	public method SetTriggerDelay {value}
	public method SetLAMTimeout {value}
	public method SetScalerPeriod {value}
	public method SetScalerFrequency {value}
	public method SetLED {LED code invert latch}
	public method SetNIM {NIM code invert latch}
	public method SetGateDelay {register intput output gate delay invert latch}
	public method ReadScaler {scaler}
	public method WriteScaler {scaler data}
	public method SetLAMMask {mask}
	public method SetBulkBuffers {buffers}
	public method SetBulkTimeOut {timeout}
	public method sReadScaler {stack scaler}
	public method sAddMarker {stack marker}
}

itcl::body CCUSB::GetFirmware {} {
	set error [catch {set f [ReadRegister 0]} message]
	if {$error == 1} {set f [ReadRegister 0]}
	set firmware [format 0x%8x $f]
#	set firmware "Firmware: Year= [expr ($f&0x1f000000)>>24] Month= [expr ($f&0xe0000000)>>29] \
	Version= [expr ($f&0xf00)>>8].[expr $f&0xf]"
	return $firmware
}

itcl::body CCUSB::ReadRegister {reg} {
	set r [::CCUSBReadRegister $self $reg]
	return $r
}

itcl::body CCUSB::WriteRegister {reg data} {
	::CCUSBWriteRegister $self $reg $data
	set check [ReadRegister $reg]
	if {![expr $check == $data]} {
		tk_messageBox -icon error -message "Error while writing register $reg to $self\n\
		Data: $data, ReadBack: $check"
		return
	}
}

itcl::body CCUSB::SetRegister {reg data} {
	::CCUSBWriteRegister $self $reg $data
}

itcl::body CCUSB::SetBufferLength {length} {
	set bits [expr int(log($length)/log(2)+.5)]
	if {$bits > 12} {set bits 12}
	if {$bits < 6} {set bits 5}
	set code [expr 12-$bits]
	set mode [ReadRegister 1]
	set mode [expr ($mode&0xfff8)+$code]
	WriteRegister 1 $mode
}

itcl::body CCUSB::SetMixedBuffer {value} {
	set mode [ReadRegister 1]
	set mode [expr ($mode&0xffdf)+($value<<5)]
	WriteRegister 1 $mode
}

itcl::body CCUSB::SetForceDump {value} {
	set mode [ReadRegister 1]
	set mode [expr ($mode&0xffbf)+($value<<6)]
	WriteRegister 1 $mode
}

itcl::body CCUSB::SetLatchEvent {value} {
	set mode [ReadRegister 1]
	set mode [expr ($mode&0xffef)+($value<<4)]
	WriteRegister 1 $mode
}

itcl::body CCUSB::SetHeader {value} {
	set mode [ReadRegister 1]
	set mode [expr ($mode&0xfeff)+($value<<8)]
	WriteRegister 1 $mode
}

#itcl::body CCUSB::SetRepeatDelay {value} {
# value is in units of 100 ns (max delay 700 ns)
#	set mode [ReadRegister 1]
#	set mode [expr ($mode&0x1fff)+($value<<13)]
#	WriteRegister 1 $mode
#}

itcl::body CCUSB::SetTriggerDelay {value} {
	set delays [ReadRegister 2]
	set delays [expr ($delays&0xff00)+$value]
	WriteRegister 2 $delays
}

itcl::body CCUSB::SetLAMTimeout {value} {
	set delays [ReadRegister 2]
	set delays [expr ($delays&0xff)+($value<<8)]
	WriteRegister 2 $delays
}

itcl::body CCUSB::SetScalerPeriod {value} {
	set scaler [ReadRegister 3]
	set scaler [expr ($scaler&0xff0000)+$value]
	WriteRegister 3 $scaler
}

itcl::body CCUSB::SetScalerFrequency {value} {
	set scaler [ReadRegister 3]
	set scaler [expr ($scaler&0xffff)+($value<<16)]
	WriteRegister 3 $scaler
}

itcl::body CCUSB::SetLED {LED code invert latch} {
	::CCUSBSetLED $self $LED $code $invert $latch
}

itcl::body CCUSB::SetNIM {NIM code invert latch} {
	::CCUSBSetNIM $self $NIM $code $invert $latch
}

itcl::body CCUSB::SetGateDelay {register input output gate delay invert latch} {
	if {[string equal $register A]} {set r 0}
	if {[string equal $register B]} {set r 1}
	if {![info exist r]} {
		tk_messageBox -icon error -message "Error while setting gate & delay of $self\n\
		unknown register: $register ; must be either A or B"
		return
	}
	::CCUSBSetDGG $self $r $input $output $gate $delay $invert $latch
}

itcl::body CCUSB::SetLAMMask {mask} {
	WriteRegister 9 $mask
}

itcl::body CCUSB::ReadScaler {scaler} {
	if {[string equal $scaler A]} {set r 11}
	if {[string equal $scaler B]} {set r 12}
	if {![info exist r]} {
		tk_messageBox -icon error -message "Error while reading scaler of $self\n\
		unknown scaler: $scaler ; must be either A or B"
		return
	}
	set data [ReadRegister $r]
	return $data
}

itcl::body CCUSB::WriteScaler {scaler data} {
	if {[string equal $scaler A]} {set r 11}
	if {[string equal $scaler B]} {set r 12}
	if {![info exist r]} {
		tk_messageBox -icon error -message "Error while reading scaler of $self\n\
		unknown scaler: $scaler ; must be either A or B"
		return
	}
	WriteRegister $r $data
}

itcl::body CCUSB::SetBulkBuffers {buffers} {
	set bulk [ReadRegister 14]
	set bulk [expr ($bulk&0xf00)+$buffers]
	WriteRegister 14 $bulk
}

itcl::body CCUSB::SetBulkTimeOut {timeout} {
	set bulk [ReadRegister 14]
	set bulk [expr ($bulk&0xff)+($timeout<<8)]
	WriteRegister 14 $bulk
}

itcl::body CCUSB::sReadScaler {stack scaler} {
	if {[string equal $scaler A]} {set r 11}
	if {[string equal $scaler B]} {set r 12}
	if {![info exist r]} {
		tk_messageBox -icon error -message "Error while reading scaler of $self\n\
		unknown scaler: $scaler ; must be either A or B"
		return
	}
	set N 25
	set A $r
	set F 0
	set command [expr ($N<<9)+($A<<5)+$F+0x4000]
	AddToStack $stack $command
}

itcl::body CCUSB::sAddMarker {stack marker} {
	set N 0
	set A 0
	set F 16
	set command [expr ($N<<9)+($A<<5)+$F]
	AddToStack $stack $command
	AddToStack $stack $marker
}

