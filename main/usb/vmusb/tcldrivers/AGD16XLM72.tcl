#===================================================================
# class AGD16XLM72
#===================================================================
itcl::class AGD16XLM72 {
	inherit AXLM72

	constructor {de sl} {
		AXLM72::constructor $de $sl
	} {}

	public method WriteDelayWidth {ch de wi} {Write fpga [expr $ch*4] [expr $de+($wi<<8)]}
	public method WriteBypass {by} {Write fpga 68 $by}
	public method WriteInspect {in} {Write fpga 72 $in}
	public method Init {filename aname}
	public method SetupGUI {parent}
	public method GrabBusGD16 {} {AccessBus 0x10000}
	public method ReleaseBusGD16 {} {ReleaseBus}
	public method XgetGD16 {addr} {Read fpga $addr}
	public method XsetGD16 {addr data} {Write fpga $addr $data}
	public method UpdateGUIGD16 {}
	public method CheckModuleGD16 {}
	public method GetModuleGD16 {}
	public method PutModuleGD16 {}
	public method ReadFileGD16 {}
	public method WriteFileGD16 {}
	public method SetDelayWidth {i}
	public method SetInspect {}
	public method SetBypass {}
	public method DrawEntryGD16 {c x y id var label color}
	public method IncrementGD16 {var}
	public method DecrementGD16 {var}
	public method StopRepeatGD16 {}
	public method LockGD16 {}
	public method UnlockGD16 {}
}

itcl::body AGD16XLM72::Init {filename aname} {
	global gd16
	set gd16(file) $filename
	set gd16(moduleName) $aname
	set gd16(locked) 0
	source $filename
	foreach n [array names $aname] {set gd16($n) [lindex [array get $aname $n] 1]}
	set gd16(crate) $device
	AccessBus 0x10000
	for {set i 1} {$i <= 16} {incr i} {
		WriteDelayWidth $i [lindex [array get $aname delay$i] 1] [lindex [array get $aname width$i] 1]
	}
	WriteBypass [lindex [array get $aname bypass] 1]
	WriteInspect [lindex [array get $aname inspect] 1]
	ReleaseBus
}

# The following methods are adapted from the original gd16.tcl code

itcl::body AGD16XLM72::SetupGUI {parent} {
	global gd16
	
	wm title $parent "XLM72 16 channel Gate & Delay Generator GUI"
#	set gd16(w) [tabnotebook_page $parent "TDC Delays"]
	set gd16(w) $parent
	
	set cb lightblue
	frame $gd16(w).side -background $cb
	set w $gd16(w).side.command
	frame $w -borderwidth 2 -relief groove -background $cb
	label $w.cratelabel -text Crate -background $cb
	entry $w.crate -textvariable gd16(crate) -width 8 -background $cb
	label $w.slotlabel -text Slot -background $cb
	entry $w.slot -textvariable gd16(slot) -width 2 -background $cb
	button $w.get -text Get -command "$this GetModuleGD16" -background $cb
	button $w.put -text Put -command "$this PutModuleGD16" -background $cb
	label $w.namelabel -text "Module Name" -background $cb
	entry $w.name -textvariable gd16(moduleName) -width 16 -background $cb
	grid $w.namelabel $w.cratelabel $w.slotlabel $w.get -sticky news
	grid $w.name $w.crate $w.slot $w.put -sticky news
#	grid $w.cratelabel $w.crate
#	grid $w.slotlabel $w.slot
#	grid $w.get $w.put
	pack $w -side left -expand 1 -fill x
	
	set w $gd16(w).side.file
	set cb lightblue
	frame $w -borderwidth 2 -relief groove -background $cb
	label $w.filelabel -text "Configuration File:" -background $cb
	entry $w.file -textvariable gd16(file) -background $cb -width 40
	button $w.read -text Read -command "$this ReadFileGD16" -background $cb
	button $w.write -text Write -command "$this WriteFileGD16" -background $cb
	label $w.dtlabel -text "dt (ns):" -background $cb
	entry $w.dt -textvariable gd16(dt) -width 3 -background $cb
	grid $w.filelabel $w.dtlabel $w.read -sticky news
	grid $w.file $w.dt $w.write -sticky news
#	grid $w.read $w.write
#	grid $w.dtlabel $w.dt
	pack $w -side right -expand 1 -fill x
	pack $gd16(w).side -side top -expand 1 -fill both

	set w $gd16(w).control
	set cc pink
	frame $w -borderwidth 2 -relief sunken
	label $w.channel -text Channel -background $cc
	label $w.delay -text Delay -background $cc
	label $w.width -text Width -background $cc
	for {set i 1} {$i <= 4} {incr i} {
		label $w.inspect$i -text Inspect$i -background $cc
	}
	label $w.bypass -text Bypass -background $cc
	grid $w.channel $w.delay - $w.width - \
	$w.inspect1 $w.inspect2 $w.inspect3 $w.inspect4 $w.bypass \
	-sticky news
	for {set i 1} {$i <= 16} {incr i} {
		if {[expr ($i/2)*2] == $i} {set cc lightgreen} else {set cc lightyellow}
		entry $w.channel$i -textvariable gd16(channel$i) -width 10 -background $cc
		canvas $w.delay$i -height 22 -width 50 -background $cc
		DrawEntryGD16 $w.delay$i 0 11 delay$i delay$i "" $cc
		label $w.delaylabel$i -textvariable gd16(delaylabel$i) -background $cc
		canvas $w.width$i -height 22 -width 50 -background $cc
		DrawEntryGD16 $w.width$i 0 11 width$i width$i "" $cc
		label $w.widthlabel$i -textvariable gd16(widthlabel$i) -background $cc
		radiobutton $w.insp1ch$i -command "$this SetInspect" -variable gd16(inspect1) -value [expr $i-1] -background $cc
		radiobutton $w.insp2ch$i -command "$this SetInspect"  -variable gd16(inspect2) -value [expr $i-1] -background $cc
		radiobutton $w.insp3ch$i -command "$this SetInspect"  -variable gd16(inspect3) -value [expr $i-1] -background $cc
		radiobutton $w.insp4ch$i -command "$this SetInspect"  -variable gd16(inspect4) -value [expr $i-1] -background $cc
		checkbutton $w.bypass$i -command "$this SetBypass" -variable gd16(bypass$i) -background $cc
		grid $w.channel$i $w.delay$i $w.delaylabel$i $w.width$i $w.widthlabel$i \
		$w.insp1ch$i $w.insp2ch$i $w.insp3ch$i $w.insp4ch$i $w.bypass$i \
		-sticky news
	}
	grid columnconfigure $w "0 1 2 3 4 5 6 7 8 9 10" -weight 1
	pack $w -side left -expand 1 -fill both	
}

itcl::body AGD16XLM72::UpdateGUIGD16 {} {
	global gd16
	set w $gd16(w).control
	for {set i 1} {$i <= 16} {incr i} {
		if {[llength $gd16(dt)] > 0} {
			set gd16(delaylabel$i) [format "= %.0f ns" [expr $gd16(delay$i)*$gd16(dt)]]
			set gd16(widthlabel$i) [format "= %.0f ns" [expr $gd16(width$i)*$gd16(dt)]]
		} else {
			set gd16(delaylabel$i) "= ???? ns"
			set gd16(widthlabel$i) "= ???? ns"
		}
		set gd16(bypass$i) [expr ($gd16(bypass)&(1<<($i-1)))>>($i-1)]	
	}
	set gd16(inspect1) [expr $gd16(inspect)&0xf]
	set gd16(inspect2) [expr ($gd16(inspect)&0xf0)>>4]
	set gd16(inspect3) [expr ($gd16(inspect)&0xf00)>>8]
	set gd16(inspect4) [expr ($gd16(inspect)&0xf000)>>12]
}

itcl::body AGD16XLM72::CheckModuleGD16 {} {
	global gd16
	GrabBusGD16
	if {![string equal [format %x [XgetGD16 0x0]] $gd16(configuration)]} {
		tk_messageBox -icon error -message "XLM72 has wrong configuration!"
		ReleaseBusGD16
		return 0
	}
	return 1
}

itcl::body AGD16XLM72::GetModuleGD16 {} {
	global gd16
	if {![CheckModuleGD16]} {return}
	GrabBusGD16
	for {set i 1} {$i <= 16} {incr i} {
		set gd16(dw$i) [XgetGD16 [expr $i*4]]
		set gd16(delay$i) [expr $gd16(dw$i)&0xff]
		set gd16(width$i) [expr $gd16(dw$i)>>8]
	}
	set gd16(bypass) [XgetGD16 0x44]
	set gd16(inspect) [XgetGD16 0x48]
	UpdateGUIGD16
	ReleaseBusGD16
}

itcl::body AGD16XLM72::PutModuleGD16 {} {
	global gd16
	if {![CheckModuleGD16]} {return}
	GrabBusGD16
	for {set i 1} {$i <= 16} {incr i} {
		set gd16(dw$i) [expr $gd16(delay$i) + ($gd16(width$i)<<8)]
		XsetGD16 [expr $i*4] $gd16(dw$i)
	}
	SetInspect
	SetBypass
}

itcl::body AGD16XLM72::ReadFileGD16 {} {
	global gd16
	if {![file exist $gd16(file)]} {
		tk_messageBox -icon error -message "File $gd16(file) doesn't exist!"
		return
	}
	source $gd16(file)
	foreach n [array names $gd16(moduleName)] {set gd16($n) [lindex [array get $gd16(moduleName) $n] 1]}
	UpdateGUIGD16
}

itcl::body AGD16XLM72::WriteFileGD16 {} {
	global gd16
	if {[catch {set file [open $gd16(file) w]}]} {
		tk_messageBox -icon error -message "Unable to open file $gd16(file) for writing!"
		return
	}
	puts $file "# gd16 configuration file written on [clock format [clock second]]"
	puts $file [format "set %s(crate) %s" $gd16(moduleName) $gd16(crate)]
	puts $file [format "set %s(slot) %d" $gd16(moduleName) $gd16(slot)]
	puts $file [format "set %s(configFileName) %s" $gd16(moduleName) $gd16(configFileName)]
	puts $file [format "set %s(configuration) %s" $gd16(moduleName) $gd16(configuration)]
	puts $file [format "set %s(dt) %g" $gd16(moduleName) $gd16(dt)]
	for {set i 1} {$i <= 16} {incr i} {
		puts $file [format "set %s(channel$i) %s" $gd16(moduleName) $gd16(channel$i)]
		puts $file [format "set %s(delay$i) %d" $gd16(moduleName) $gd16(delay$i)]
		puts $file [format "set %s(width$i) %d" $gd16(moduleName) $gd16(width$i)]
	}
	puts $file [format "set %s(bypass) 0x%x" $gd16(moduleName) $gd16(bypass)]
	puts $file [format "set %s(inspect) 0x%x" $gd16(moduleName) $gd16(inspect)]
	close $file
}

itcl::body AGD16XLM72::SetDelayWidth {i} {
	global gd16
	if {![CheckModuleGD16]} {return}
	GrabBusGD16
	if {[llength $gd16(dt)] > 0} {
		set gd16(delaylabel$i) [format "= %.0f ns" [expr $gd16(delay$i)*$gd16(dt)]]
		set gd16(widthlabel$i) [format "= %.0f ns" [expr $gd16(width$i)*$gd16(dt)]]
	}
	set gd16(dw$i) [expr $gd16(delay$i) + ($gd16(width$i)<<8)]
	XsetGD16 [expr $i*4] $gd16(dw$i)
	ReleaseBusGD16
}

itcl::body AGD16XLM72::SetInspect {} {
	global gd16
	if {![CheckModuleGD16]} {return}
	GrabBusGD16
	set gd16(inspect) [expr $gd16(inspect1) + ($gd16(inspect2)<<4) + ($gd16(inspect3)<<8) + ($gd16(inspect4)<<12)]
	XsetGD16 0x48 $gd16(inspect)
	ReleaseBusGD16
}

itcl::body AGD16XLM72::SetBypass {} {
	global gd16
	if {![CheckModuleGD16]} {return}
	GrabBusGD16
	set gd16(bypass) 0
	for {set i 1} {$i <= 16} {incr i} {
		set gd16(bypass) [expr $gd16(bypass) + ($gd16(bypass$i)<<($i-1))]
	}
	XsetGD16 0x44 $gd16(bypass)
	ReleaseBusGD16
}

itcl::body AGD16XLM72::DrawEntryGD16 {c x y id var label color} {
	global gd16
	$c create text $x $y -text $label -anchor e
	label $c.$id -textvariable gd16($var) -width 4 -background $color
	$c create window $x $y -window $c.$id -anchor w
	set up [$c create polygon [expr $x+35] [expr $y-2] \
	[expr $x+45] [expr $y-2] [expr $x+40] [expr $y-10] -fill white -outline black]
	set down [$c create polygon [expr $x+35] [expr $y+2] \
	[expr $x+45] [expr $y+2] [expr $x+40] [expr $y+10] -fill white -outline black]
	$c bind $up <Enter> "$c itemconfigure $up -fill black"
	$c bind $up <Leave> "$c itemconfigure $up -fill white"
	$c bind $up <Button-1> "$this IncrementGD16 $var"
	$c bind $up <ButtonRelease> "$this StopRepeatGD16"
	$c bind $down <Enter> "$c itemconfigure $down -fill black"
	$c bind $down <Leave> "$c itemconfigure $down -fill white"
	$c bind $down <Button-1> "$this DecrementGD16 $var"
	$c bind $down <ButtonRelease> "$this StopRepeatGD16"
	return $c.$id
}

itcl::body AGD16XLM72::IncrementGD16 {var} {
	global gd16
	if {[info exist gd16(firstClick)] == 0} {
		set gd16(firstClick) 1
	}
	if {$gd16(firstClick) == 1} {
		set gd16(repeatID) [after 500 $this IncrementGD16 $var]
		set gd16(firstClick) 0
	} else {
		set gd16(repeatID) [after 50 $this IncrementGD16 $var]
	}
	if {$gd16(locked) == 1} {
		return
	}
	set gd16($var) [expr $gd16($var) + 1]
	if {$gd16($var) > 255} {
		set gd16($var) 255
	}
	if {[string match delay* $var]} {set i [string trimleft $var delay]}
	if {[string match width* $var]} {set i [string trimleft $var width]}
	SetDelayWidth $i
}

itcl::body AGD16XLM72::DecrementGD16 {var} {
	global gd16
	if {[info exist gd16(firstClick)] == 0} {
		set gd16(firstClick) 1
	}
	if {$gd16(firstClick) == 1} {
		set gd16(repeatID) [after 500 $this DecrementGD16 $var]
		set gd16(firstClick) 0
	} else {
		set gd16(repeatID) [after 50 $this DecrementGD16 $var]
	}
	if {$gd16(locked) == 1} {
		return
	}
	set gd16($var) [expr $gd16($var) - 1]
	if {$gd16($var) < 0} {
		set gd16($var) 0
	}
	if {[string match delay* $var]} {set i [string trimleft $var delay]}
	if {[string match width* $var]} {set i [string trimleft $var width]}
	SetDelayWidth $i
}

itcl::body AGD16XLM72::StopRepeatGD16 {} {
	global gd16
	after cancel $gd16(repeatID)
	set gd16(firstClick) 1
}

itcl::body AGD16XLM72::LockGD16 {} {
	global gd16
	set gd16(locked) 1
#	set keep "$gd16(w).side.command.get"
	set keep ""
	foreach c [winfo children $gd16(w).side.command] {
		if {[lsearch $keep $c] == -1} {
			$c configure -state disabled
		}
	}
	set keep "$gd16(w).side.file.read"
	foreach c [winfo children $gd16(w).side.file] {
		if {[lsearch $keep $c] == -1} {
			$c configure -state disabled
		}
	}
	foreach c [winfo children $gd16(w).control] {
		$c configure -state disabled
	}
}

itcl::body AGD16XLM72::UnlockGD16 {} {
	global gd16
	set gd16(locked) 0
	foreach c [winfo children $gd16(w).side.command] {
		$c configure -state normal
	}
	foreach c [winfo children $gd16(w).side.file] {
		$c configure -state normal
	}
	foreach c [winfo children $gd16(w).control] {
		$c configure -state normal
	}
}
