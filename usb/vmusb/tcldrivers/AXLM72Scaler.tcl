#===================================================================
# class AXLM72Scaler
# D. Bazin, Dec. 2009
# These functions are based on the FPGA configuration named ech32x24
# which contains 32 scalers of 24 bit depth each, as well as a trigger
# output selectable from the bits set in the trigger variable
#===================================================================
itcl::class AXLM72Scaler {
	inherit AXLM72
	
	private variable around
	private variable cancel
	public variable wrap
	public variable scaler
	public variable rate
	public variable enable
	public variable trigger
	public variable name
	public variable live
	public variable frequency
	
	constructor {de sl} {
		AXLM72::constructor $de $sl
	} {
		# This is for a SpartanXL version of the module (24 bits scalers)
		set around [expr 1<<24]
		for {set i 0} {$i < 32} {incr i} {
			set scaler($i) 0
			set rate($i) 0
			set enable 0
			set trigger($i) 0
		}
		# look for a wrap file
		if {[file exists "XLM72Scaler_[string trimleft $this :].tcl"]} {
			source "XLM72Scaler_[string trimleft $this :].tcl"
		} else {
			for {set i 0} {$i < 32} {incr i} {
				set wrap($i) 0
				set name($i) ""
			}
			set live 0
			set frequency 1
		}
		# look for module and update from it
		if {[expr [GetFirmware] == 0xdaba0002]} {UpdatePanel 1}
}

# Interactive functions
	public method GetFirmware {}
	public method Reset {}
	public method Latch {}
	public method SetTrigger {bit}
	public method SetTriggerBit {bit value}
	public method ReadTrigger {}
	public method SetEnable {}
	public method ReadEnable {}
	public method SetLive {}
	public method ReadAll {}
	public method ControlPanel {}
	public method UpdatePanel {once}
	public method SaveSettings {}
	public method Exit {}
# Stack functions
	public method sEnable {stack}
	public method sDisable {stack}
	public method sLatch {stack}
	public method sReadAll {stack}
}

# Interactive functions implementation
itcl::body AXLM72Scaler::GetFirmware {} {
	AccessBus 0x10000
	set value [Read fpga 0]
	ReleaseBus
	return [format 0x%8x $value]
}

itcl::body AXLM72Scaler::Reset {} {
	AccessBus 0x10000
	Write fpga 0 1
	Write fpga 0 0
	ReleaseBus
	for {set i 0} {$i < 32} {incr i} {
		set wrap($i) 0
	}
}

itcl::body AXLM72Scaler::Latch {} {
	AccessBus 0x10000
	Write fpga 4 1; # set latch bit
	Write fpga 4 0; # reset it
	ReleaseBus; # once bus is released, FPGA takes over and writes scalers to SRAMA
}

itcl::body AXLM72Scaler::SetTriggerBit {bit value} {
	set trigger($bit) $value
	SetTrigger $bit
}

itcl::body AXLM72Scaler::SetTrigger {bit} {
	AccessBus 0x10000
	set value [Read fpga 0xc]
	if {$trigger($bit)} {
		set value [expr $value|(1<<$bit)]
	} else {
		set value [expr $value^(1<<$bit)]
	}
	Write fpga 0xc $value
	ReleaseBus
}

itcl::body AXLM72Scaler::ReadTrigger {} {
	AccessBus 0x10000
	set value [Read fpga 0xc]
	ReleaseBus
	for {set i 0} {$i < 32} {incr i} {
		set trigger($i) [expr ($value&(1<<$i))>>$i]
	}
#	return [format 0x%8x $value]
}

itcl::body AXLM72Scaler::SetEnable {} {
	AccessBus 0x10000
	Write fpga 8 $enable
	ReleaseBus
}

itcl::body AXLM72Scaler::ReadEnable {} {
	AccessBus 0x10000
	set enable [expr [Read fpga 8]&1]
	ReleaseBus
}

itcl::body AXLM72Scaler::ReadAll {} {
	Latch
	AccessBus 1
	set data [ReadSBLT srama 0 32]
	ReleaseBus
	return $data
}

itcl::body AXLM72Scaler::ControlPanel {} {
	set top ".aXLM72Scaler_[string trimleft $this :]"
	if {[winfo exist $top]} {return}
	toplevel $top
	wm title $top "AXLM72Scaler$this"
	set w $top.s1
	frame $w -bg lightblue
	for {set i 0} {$i < 16} {incr i} {
		checkbutton $w.trigger$i -text "Ch$i" -variable [itcl::scope trigger($i)] -command "$this SetTrigger $i" -bg lightblue
		entry $w.name$i -textvariable [itcl::scope name($i)] -width 6 -validatecommand SaveSettings
		label $w.scaler$i -textvariable [itcl::scope scaler($i)] -width 8 -bg lightblue -anchor e
		label $w.rate$i -textvariable [itcl::scope rate($i)] -width 8 -bg lightblue -anchor e
		grid $w.name$i $w.scaler$i $w.rate$i $w.trigger$i -sticky news
	}
	set w $top.s2
	frame $w -bg lightblue
	for {set i 16} {$i < 32} {incr i} {
		checkbutton $w.trigger$i -text "Ch$i" -variable [itcl::scope trigger($i)] -command "$this SetTrigger $i" -bg lightblue
		entry $w.name$i -textvariable [itcl::scope name($i)] -width 8 -validatecommand SaveSettings
		label $w.scaler$i -textvariable [itcl::scope scaler($i)] -width 8 -bg lightblue -anchor e
		label $w.rate$i -textvariable [itcl::scope rate($i)] -width 8 -bg lightblue -anchor e
		grid $w.name$i $w.scaler$i $w.rate$i $w.trigger$i -sticky news
	}
	grid $top.s1 $top.s2 -sticky news
	set w $top.c
	frame $w -bg lightblue
	checkbutton $w.enable -text Enable -variable [itcl::scope enable] -command "$this SetEnable" -bg lightblue
	checkbutton $w.live -text Live -variable [itcl::scope live] -command "$this SetLive" -bg lightblue
	spinbox $w.freq -textvariable [itcl::scope frequency] -width 3 -increment 1 -from 1 -to 10
	button $w.reset -text Reset -command "$this Reset"
	button $w.exit -text Exit -command "$this Exit"
	grid $w.enable $w.live $w.freq $w.reset $w.exit -sticky news
	grid $top.c - -sticky news
	grid columnconfigure $top 0 -weight 1
	grid columnconfigure $top 1 -weight 1
	grid rowconfigure $top 0 -weight 1
	grid rowconfigure $top 1 -weight 1
}

itcl::body AXLM72Scaler::UpdatePanel {once} {
	global runinfo
	set top ".aXLM72Scaler_[string trimleft $this :]"
	if {[string equal $runinfo(state) active]} {
		foreach c [winfo children $top.s1] {$c configure -state disable}
		foreach c [winfo children $top.s2] {$c configure -state disable}
		foreach c [winfo children $top.c] {$c configure -state disable}
	} else {
		foreach c [winfo children $top.s1] {$c configure -state normal}
		foreach c [winfo children $top.s2] {$c configure -state normal}
		foreach c [winfo children $top.c] {$c configure -state normal}
		ReadEnable
		ReadTrigger
		set values [ReadAll]
		for {set i 0} {$i < 32} {incr i} {
			set updated [expr $wrap($i)*$around + [lindex $values [expr $i*2]] +	 [lindex $values [expr $i*2+1]]*(1<<16)]
			set rate($i) [format %.1f [expr ($updated-$scaler($i))/$frequency]]
			set scaler($i) $updated
		}
	}
	if {!$once} {
		set cancel [after [expr $frequency*1000] "$this UpdatePanel 0"]
	}
}

itcl::body AXLM72Scaler::SetLive {} {
	if {$live} {
		UpdatePanel 0
	} else {
		after cancel $cancel
	}
}

itcl::body AXLM72Scaler::Exit {} {
	after cancel $cancel
	SaveSettings
	set top ".aXLM72Scaler_[string trimleft $this :]"
	destroy $top
}

itcl::body AXLM72Scaler::SaveSettings {} {
	set file [open "XLM72Scaler_[string trimleft $this :].tcl" w]
	for {set i 0} {$i < 32} {incr i} {
		puts $file "set trigger($i) $trigger($i)"
		puts $file "set name($i) \"$name($i)\""
		puts $file "set wrap($i) $wrap($i)"
	}
	puts $file "set enable $enable"
	puts $file "set live $live"
	puts $file "set frequency $frequency"
	close $file
	return 1
}


# Stack functions implementation
itcl::body AXLM72Scaler::sEnable {stack} {
	sAccessBus $stack 0x10000
	sWrite $stack fpga 8 1
	sReleaseBus $stack
}

itcl::body AXLM72Scaler::sDisable {stack} {
	sAccessBus $stack 0x10000
	sWrite $stack fpga 8 0
	sReleaseBus $stack
}

itcl::body AXLM72Scaler::sLatch {stack} {
	sAccessBus $stack 0x10000
	sWrite $stack fpga 4 1
	sWrite $stack fpga 4 0
	sReleaseBus $stack
}

itcl::body AXLM72Scaler::sReadAll {stack} {
	sLatch $stack
	$device sWait $stack 20
	sAccessBus $stack 1
	sReadSBLT $stack srama 0 32
	sReleaseBus $stack
}
