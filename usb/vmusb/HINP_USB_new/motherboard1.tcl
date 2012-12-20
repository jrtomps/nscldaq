#!/bin/wish
# motherboard1 --
# page to control motherboard resources
#
# RCS: @(#) $Id: motherboard1,v 1.2 2004/03/03  elson Exp $

#  Tk_Init
tk appname Motherboard1
wm title . "Motherboard Control"
set widgetDemo 1
set smallfont -*-Helvetica-Medium-R-Normal--*-80-*-*-*-*-*-*


##################################
set mbpage [tabnotebook_page .main "Motherboard Control"]

#----------------------------------------------------------------
# The code below create the main window, consisting of a menu bar
# and a text widget that explains how to use the program, plus lists
# all of the demos as hypertext items.
#----------------------------------------------------------------
option add *Menubutton.selectColor: red
   set data {
     #define data_width 16
     #define data_height 16
     static unsigned char data_bits[] = {
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
   }
   set mask {
     #define mask_width 16
     #define mask_height 16
     static unsigned char mask_bits[] = {
       0xe0, 0x07, 0xf8, 0x1f, 0xfc, 0x3f, 0x3e, 0x7c, 0x0e, 0x70, 0xcf,
       0xf3, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xcf, 0xf3,
       0x0e, 0x70, 0x3e, 0x7c, 0xfc, 0x3f, 0xf8, 0x1f, 0xe0, 0x07 };
   }
   set fore {
     #define data_width 16
     #define data_height 16
     static unsigned char data_bits[] = {
       0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
       0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
       0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
   }

set backbutton [image create bitmap -background gray -foreground red \
		    -data $data -maskdata $mask]
set forebutton [image create bitmap -background red -foreground gray \
		    -data $data -maskdata $fore]

set font {Helvetica 8}
set midfont {Helvetica 7}

# global "defines"

global steering1 steering2 steering3 steering4 steering5 steering6 steering7 steering8
global steering9 steering10 steering11 steering12 steering13 steering14 steering15 steering16
global status1 status2 status3 status4 status5 status6 status7 status8
global status9 status10 status11 status12 status13 status14 status15 status16
global SumOffA SumOffB SumOffC ShapOff CSAOff
global SISDelay AcqDelay PauseDelay CycleTimeout GlobalTimeout TriggerDelay
global FTDelay AADelay GDDelay
global CoincWindow
global CurrentBoard
global TotBoards
global CurrentChip
global CurrentChannel
global ForceRead

set CurrentChip 1

for {set i 1} {$i <= 16} {incr i} {
    set steering$i "o"
    set status$i "empty"
}
frame $mbpage.top
frame $mbpage.mid
frame $mbpage.bot
pack $mbpage.top $mbpage.mid $mbpage.bot -side top -expand yes -pady 0 -anchor w

frame $mbpage.mid.serdac
frame $mbpage.mid.times
frame $mbpage.mid.cb
pack  $mbpage.mid.serdac $mbpage.mid.times $mbpage.mid.cb -side left -expand yes  -pady .5 -padx .5

label $mbpage.mid.serdac.l -font $font -wraplength 5i -justify left -text "DAC Settings"

scale $mbpage.mid.cb.scale -font $font -orient horizontal -length 125 -from 1 -to $TotBoards \
    -tickinterval 1 -variable CurrentBoard -label "Current Board"
pack $mbpage.mid.cb.scale \
    -side top -pady 2 -anchor w
bind $mbpage.mid.cb.scale <ButtonRelease-1> \
    {ChangeChip $CurrentBoard $CurrentChip $CurrentChannel}

checkbutton $mbpage.mid.cb.setForce -text "Forced Readout" -variable ForceRead \
    -relief flat -selectcolor red -indicatoron false
pack $mbpage.mid.cb.setForce -side bottom -pady 1 -anchor w
bind $mbpage.mid.cb.setForce <ButtonRelease-1> \
    {SetForceRead $CurrentBoard $ForceRead}

frame $mbpage.mid.serdac.wSumOffA
frame $mbpage.mid.serdac.wSumOffB
frame $mbpage.mid.serdac.wSumOffC
frame $mbpage.mid.serdac.wShapOff
frame $mbpage.mid.serdac.wCSAOff

pack  $mbpage.mid.serdac.wSumOffA $mbpage.mid.serdac.wSumOffB $mbpage.mid.serdac.wSumOffC \
	$mbpage.mid.serdac.wShapOff $mbpage.mid.serdac.wCSAOff  -side top -expand no -pady 0 -padx 0


label $mbpage.mid.serdac.wSumOffA.l -font $font -wraplength 5i -justify left -text "Sum Offset A"
label $mbpage.mid.serdac.wSumOffB.l -font $font -wraplength 5i -justify left -text "Sum Offset B"
label $mbpage.mid.serdac.wSumOffC.l -font $font -wraplength 5i -justify left -text "Sum Offset C"
label $mbpage.mid.serdac.wShapOff.l -font $font -wraplength 5i -justify left -text "Shaper Offset"
label $mbpage.mid.serdac.wCSAOff.l -font $font -wraplength 5i -justify left -text "CSA Offset"

scale $mbpage.mid.serdac.wSumOffA.scale -orient horizontal -length 150  -from 0 -to 1023 \
	-tickinterval 256 -variable SumOffA -borderwidth 2 -font $smallfont
pack $mbpage.mid.serdac.wSumOffA.l $mbpage.mid.serdac.wSumOffA.scale -side left -pady 0 -anchor w -expand no
bind $mbpage.mid.serdac.wSumOffA.scale <ButtonRelease-1> \
    {SetSumOffA  $CurrentBoard $SumOffA}

scale $mbpage.mid.serdac.wSumOffB.scale -orient horizontal -length 150  -from 0 -to 1023 \
	-tickinterval 256 -variable SumOffB -borderwidth 2 -font $smallfont
pack $mbpage.mid.serdac.wSumOffB.l $mbpage.mid.serdac.wSumOffB.scale -side left -pady 0 -anchor w -expand no
bind $mbpage.mid.serdac.wSumOffB.scale <ButtonRelease-1> \
    {SetSumOffB $CurrentBoard $SumOffB}

scale $mbpage.mid.serdac.wSumOffC.scale -orient horizontal -length 150  -from 0 -to 1023 \
	-tickinterval 256 -variable SumOffC -borderwidth 2 -font $smallfont
pack $mbpage.mid.serdac.wSumOffC.l $mbpage.mid.serdac.wSumOffC.scale -side left -pady 0 -anchor w -expand no
bind $mbpage.mid.serdac.wSumOffC.scale <ButtonRelease-1> \
    {SetSumOffC $CurrentBoard $SumOffC}

scale $mbpage.mid.serdac.wShapOff.scale -orient horizontal -length 150  -from 0 -to 1023 \
	-tickinterval 256 -variable ShapOff -borderwidth 2 -font $smallfont
pack $mbpage.mid.serdac.wShapOff.l $mbpage.mid.serdac.wShapOff.scale -side left -pady 0 -anchor w
bind $mbpage.mid.serdac.wShapOff.scale <ButtonRelease-1> \
    {SetChipShapOff $CurrentBoard $CurrentChip $ShapOff}

scale $mbpage.mid.serdac.wCSAOff.scale -orient horizontal -length 150  -from 0 -to 1023 \
	-tickinterval 256 -variable CSAOff -font $smallfont
pack $mbpage.mid.serdac.wCSAOff.l $mbpage.mid.serdac.wCSAOff.scale -side left -pady 0 -anchor w
bind $mbpage.mid.serdac.wCSAOff.scale <ButtonRelease-1> \
    {SetChipCSAOff $CurrentBoard $CurrentChip $CSAOff}

frame $mbpage.mid.times.sisDelay
frame $mbpage.mid.times.acqDelay
frame $mbpage.mid.times.pauseDelay
frame $mbpage.mid.times.cycleTimeout
frame $mbpage.mid.times.globalTimeout
frame $mbpage.mid.times.trigDelay
frame $mbpage.mid.times.coincWind
pack  $mbpage.mid.times.sisDelay $mbpage.mid.times.acqDelay \
    $mbpage.mid.times.pauseDelay $mbpage.mid.times.cycleTimeout \
    $mbpage.mid.times.globalTimeout $mbpage.mid.times.trigDelay \
    $mbpage.mid.times.coincWind \
    -side top -expand no -pady 0 -padx 0
label $mbpage.mid.times.sisDelay.l -font $font -wraplength 5i -justify left -text "SIS Clock Delay (ns)"
label $mbpage.mid.times.acqDelay.l -font $font -wraplength 5i -justify left -text "Acq Delay (ns)"
label $mbpage.mid.times.pauseDelay.l -font $font -wraplength 5i -justify left -text "Pause Delay (ns)"
label $mbpage.mid.times.cycleTimeout.l -font $font -wraplength 5i -justify left -text "Cycle Timeout (ns)"
label $mbpage.mid.times.globalTimeout.l -font $font -wraplength 5i -justify left -text "Global Timeout (ns)"
label $mbpage.mid.times.trigDelay.l -font $font -wraplength 5i -justify left -text "Trigger Delay (ns)"

label $mbpage.mid.times.coincWind.l -font $font -wraplength 5i -justify left -text "Coincidence Window (ns)"

scale $mbpage.mid.times.sisDelay.scale -orient horizontal -length 300  -from 0 -to 25500 \
	-tickinterval 6000 -variable SISDelay -borderwidth 0 -resolution 40 -font $smallfont
pack $mbpage.mid.times.sisDelay.l $mbpage.mid.times.sisDelay.scale -side left \
	-pady 0 -anchor w -expand no
bind $mbpage.mid.times.sisDelay.scale <ButtonRelease-1> \
    {SetSISDelay  $CurrentBoard $SISDelay}

scale $mbpage.mid.times.acqDelay.scale -orient horizontal -length 300  -from 0 -to 25500 \
	-tickinterval 6000 -variable AcqDelay -borderwidth 0 -resolution 40 -font $smallfont
pack $mbpage.mid.times.acqDelay.l $mbpage.mid.times.acqDelay.scale -side left \
	-pady 0 -anchor w -expand no
bind $mbpage.mid.times.acqDelay.scale <ButtonRelease-1> \
    {SetAcqDelay  $CurrentBoard $AcqDelay}

scale $mbpage.mid.times.pauseDelay.scale -orient horizontal -length 300  -from 0 -to 25500 \
	-tickinterval 6000 -variable PauseDelay -borderwidth 0 -resolution 40 -font $smallfont
pack $mbpage.mid.times.pauseDelay.l $mbpage.mid.times.pauseDelay.scale -side left \
	-pady 0 -anchor w -expand no
bind $mbpage.mid.times.pauseDelay.scale <ButtonRelease-1> \
    {SetPauseDelay $CurrentBoard $PauseDelay}

scale $mbpage.mid.times.cycleTimeout.scale -orient horizontal -length 300  -from 0 -to 25500 \
	-tickinterval 6000 -variable CycleTimeout -borderwidth 0 -resolution 40 -font $smallfont
pack $mbpage.mid.times.cycleTimeout.l $mbpage.mid.times.cycleTimeout.scale -side left \
	-pady 0 -anchor w -expand no
bind $mbpage.mid.times.cycleTimeout.scale <ButtonRelease-1> \
    {SetCycleTimeout $CurrentBoard $CycleTimeout}

scale $mbpage.mid.times.globalTimeout.scale -orient horizontal -length 300  -from 0 -to 25500 \
	-tickinterval 6000 -variable GlobalTimeout -borderwidth 0 -resolution 40 -font $smallfont
pack $mbpage.mid.times.globalTimeout.l $mbpage.mid.times.globalTimeout.scale -side left \
	-pady 0 -anchor w -expand no
bind $mbpage.mid.times.globalTimeout.scale <ButtonRelease-1> \
    {SetGlobalTimeout $CurrentBoard $GlobalTimeout}

scale $mbpage.mid.times.trigDelay.scale -orient horizontal -length 300  -from 0 -to 3100 \
	-tickinterval 700 -variable TriggerDelay -borderwidth 0 -resolution 12 -font $smallfont
pack $mbpage.mid.times.trigDelay.l $mbpage.mid.times.trigDelay.scale -side left \
	-pady 0 -anchor w -expand no
bind $mbpage.mid.times.trigDelay.scale <ButtonRelease-1> \
    {SetTriggerDelay $CurrentBoard $TriggerDelay}

scale $mbpage.mid.times.coincWind.scale -orient horizontal -length 300  -from 0 -to 3100 \
	-tickinterval 700 -variable CoincWindow -borderwidth 0 -resolution 12 -font $smallfont
pack $mbpage.mid.times.coincWind.l $mbpage.mid.times.coincWind.scale -side left \
	-pady 0 -anchor w -expand no
bind $mbpage.mid.times.coincWind.scale <ButtonRelease-1> \
    {SetCoincWindow $CurrentBoard $CoincWindow}

frame $mbpage.bot.slot
frame $mbpage.bot.times
pack $mbpage.bot.slot $mbpage.bot.times -side right -expand yes  -pady .5 -padx .5

frame $mbpage.bot.times.ftDelay
frame $mbpage.bot.times.aaDelay
frame $mbpage.bot.times.gdDelay

pack $mbpage.bot.times.ftDelay $mbpage.bot.times.aaDelay $mbpage.bot.times.gdDelay \
    -side left -expand yes -pady 0.5 -padx 0.5

label $mbpage.bot.times.ftDelay.l -font $smallfont -wraplength 5i -justify left -text "FORCE_TRACK Delay (ns)"
label $mbpage.bot.times.aaDelay.l -font $smallfont -wraplength 5i -justify left -text "ACQ_ALL Delay (ns)"
label $mbpage.bot.times.gdDelay.l -font $smallfont -wraplength 5i -justify left -text "GLOBAL_DISABLE  Delay (ns)"

scale $mbpage.bot.times.ftDelay.scale -orient vertical -length 200  -from 0 -to 3100 \
	-tickinterval 700 -variable FTDelay -borderwidth 2 -resolution 12 -font $smallfont
pack $mbpage.bot.times.ftDelay.l $mbpage.bot.times.ftDelay.scale -side top \
	-pady 0 -anchor w -expand no
scale $mbpage.bot.times.aaDelay.scale -orient vertical -length 200  -from 0 -to 3100 \
	-tickinterval 700 -variable AADelay -borderwidth 2 -resolution 12 -font $smallfont
pack $mbpage.bot.times.aaDelay.l $mbpage.bot.times.aaDelay.scale -side top \
	-pady 0 -anchor w -expand no
scale $mbpage.bot.times.gdDelay.scale -orient vertical -length 200  -from 0 -to 3100 \
	-tickinterval 700 -variable GDDelay -borderwidth 2 -resolution 12 -font $smallfont
pack $mbpage.bot.times.gdDelay.l $mbpage.bot.times.gdDelay.scale -side top \
	-pady 0 -anchor w -expand no

bind $mbpage.bot.times.ftDelay.scale <ButtonRelease-1> \
    {SetFTDelay $CurrentBoard $FTDelay}
bind $mbpage.bot.times.aaDelay.scale <ButtonRelease-1> \
    {SetAADelay $CurrentBoard $AADelay}
bind $mbpage.bot.times.gdDelay.scale <ButtonRelease-1> \
    {SetGDDelay $CurrentBoard $GDDelay}

frame $mbpage.bot.slot.number

# chip board map and routing label
label $mbpage.bot.slot.l1 -font $midfont -wraplength -1 -justify right -text "       Slot Status OR/Mult. Steering    Slot   Status   OR/Mult. Steering"
pack $mbpage.bot.slot.l1  \
    -side top -expand no -pady 0 -padx .5

frame $mbpage.bot.slot.number.l
frame $mbpage.bot.slot.number.r
for {set i 1} {$i <= 8} {incr i}  {
    frame $mbpage.bot.slot.number.l.$i
    foreach steer {"C" "B" "A" "O"} {
	set lower [string tolower $steer]
	radiobutton $mbpage.bot.slot.number.l.$i.$lower -text $steer \
	    -variable steering$i -relief flat -borderwidth 1 -width 2 \
	    -value $lower -command "UpdateRouting $i $steer" \
	    -selectcolor red -indicatoron false -anchor w
	pack $mbpage.bot.slot.number.l.$i.$lower \
	    -side right -fill x
#	bind $mbpage.bot.slot.number.$i.$lower <ButtonRelease-1> \
#	    {SetSlotRouting $CurrentBoard $i $steer}
    }
    label $mbpage.bot.slot.number.l.$i.l \
	-font $font -wraplength 1i -justify right \
	-padx 10 -text "$i"
# places label "Empty", "2 chips", etc.
    label $mbpage.bot.slot.number.l.$i.status \
	-font $font -wraplength 1i -justify left \
	-padx 10 -textvariable status$i -width 9
    pack $mbpage.bot.slot.number.l.$i.l $mbpage.bot.slot.number.l.$i.status \
	-side left -expand no -fill none -anchor e
}
for {set i 9} {$i <= 16} {incr i}  {
    frame $mbpage.bot.slot.number.r.$i
    foreach steer {"C" "B" "A" "O"} {
	set lower [string tolower $steer]
	radiobutton $mbpage.bot.slot.number.r.$i.$lower -text $steer \
	    -variable steering$i -relief flat -borderwidth 1 -width 2 \
	    -value $lower -command "UpdateRouting $i $steer" \
	    -selectcolor red -indicatoron false -anchor w
	pack $mbpage.bot.slot.number.r.$i.$lower \
	    -side right -fill x
#	bind $mbpage.bot.slot.number.$i.$lower <ButtonRelease-1> \
#	    {SetSlotRouting $CurrentBoard $i $steer}
    }
    label $mbpage.bot.slot.number.r.$i.l \
	-font $font -wraplength 1i -justify right \
	-padx 10 -text "$i"
# places label "Empty", "2 chips", etc.
    label $mbpage.bot.slot.number.r.$i.status \
	-font $font -wraplength 1i -justify left \
	-padx 10 -textvariable status$i -width 9
    pack $mbpage.bot.slot.number.r.$i.l $mbpage.bot.slot.number.r.$i.status \
	-side left -expand no -fill none -anchor e
}

for {set i 1} {$i <= 8} {incr i} {
    pack $mbpage.bot.slot.number.l.$i $mbpage.bot.slot.number.l -side top -expand no -fill none -anchor e
}
for {set i 9} {$i <= 16} {incr i} {
    pack $mbpage.bot.slot.number.r.$i $mbpage.bot.slot.number.r -side top -expand no -fill none -anchor w
}
pack $mbpage.bot.slot.number.l $mbpage.bot.slot.number.r $mbpage.bot.slot.number \
    -side left -expand no -pady 0 -pady .5
pack $mbpage.bot.slot.number -side top -expand no -pady 0 -padx .5
#

proc UpdateRouting {slot steerage} {
    global CurrentBoard

    SetSlotRouting $CurrentBoard $slot $steerage
}

proc UpdateMotherboard { Motherboard } {
    global SumOffA SumOffB SumOffC ShapOff CSAOff
    global steering1 steering2 steering3 steering4 steering5 steering6 steering7 steering8
    global steering9 steering10 steering11 steering12 steering13 steering14 steering15 steering16
    global status1 status2 status3 status4 status5 status6 status7 status8
    global status9 status10 status11 status12 status13 status14 status15 status16
    global SISDelay AcqDelay PauseDelay CycleTimeout GlobalTimeout TriggerDelay CoincWindow
    global FTDelay AADelay GDDelay
    global CurrentBoard CurrentChip
    global ForceRead

    # When selected MB is changed, get values from database and set screen representation
    puts stdout [format "UpdateMotherboard %d" $Motherboard]
    set SumOffA [GetSumOffA $CurrentBoard]
#    puts stdout [format "SumOffA ia %d" $SumOffA]
    set SumOffB [GetSumOffB $CurrentBoard]
#    puts stdout [format "SumOffB ia %d" $SumOffB]
    set SumOffC [GetSumOffC $CurrentBoard]
#    puts stdout [format "SumOffC ia %d" $SumOffC]
    set ShapOff [GetShapOff $CurrentBoard $CurrentChip]
    set CSAOff  [GetCSAOff $CurrentBoard $CurrentChip]
    for {set i 1} {$i <= 16} {incr i} {
	set routing [GetSlotStatus $CurrentBoard $i]
#	puts stdout [format "GetSlotStatus for %d is %s" $i $routing]
	set status$i [string tolower $routing]
#	puts stdout [format "status%d is *%s*" $i [set status$i]]
    }
    for {set i 1} {$i <= 16} {incr i} {
	set routing [GetSlotRouting $CurrentBoard $i]
	set steering$i [string tolower $routing]
#	puts stdout [format "steering%d is *%s*" $i [set steering$i]]
    }
    set SISDelay [GetSISDelay $CurrentBoard]
    set AcqDelay [GetAcqDelay $CurrentBoard]
    set PauseDelay [GetPauseDelay $CurrentBoard]
    set CycleTimeout [GetCycleTimeout $CurrentBoard]
    set GlobalTimeout [GetGlobalTimeout $CurrentBoard]
    set TriggerDelay [GetTriggerDelay $CurrentBoard]
    set FTDelay [GetFTDelay $CurrentBoard]
    set AADelay [GetAADelay $CurrentBoard]
    set GDDelay [GetGDDelay $CurrentBoard]
    set ForceRead [GetForceRead $CurrentBoard]
    set CoincWindow [GetCoincWindow $CurrentBoard]
}


proc positionWindow w {
    wm geometry $w +100+100
}

# showVars --
# Displays the values of one or more variables in a window, and
# updates the display whenever any of the variables changes.
#
# Arguments:
# w -		Name of new window to create for display.
# args -	Any number of names of variables.

proc showVars {w args} {
    catch {destroy $w}
    toplevel $w
    wm title $w "Variable values"
    label $w.title -text "Variable values:" -width 20 -anchor center \
	    -font {Helvetica 18}
    pack $w.title -side top -fill x
    set len 1
    foreach i $args {
	if {[string length $i] > $len} {
	    set len [string length $i]
	}
    }
    foreach i $args {
	frame $w.$i
	label $w.$i.name -text "$i: " -width [expr $len + 2] -anchor w
	label $w.$i.value -textvar $i -anchor w
	pack $w.$i.name -side left
	pack $w.$i.value -side left -expand 1 -fill x
	pack $w.$i -side top -anchor w -fill x
    }
    button $w.ok -text OK -command "destroy $w" -default active
    bind $w <Return> "tkButtonInvoke $w.ok"
    pack $w.ok -side bottom -pady 2
}

# invoke --
# This procedure is called when the user clicks on a demo description.
# It is responsible for invoking the demonstration.
#
# Arguments:
# index -	The index of the character that the user clicked on.

proc invoke index {
    global tk_library
    set tags [.t tag names $index]
    set i [lsearch -glob $tags demo-*]
    if {$i < 0} {
	return
    }
    set cursor [.t cget -cursor]
    .t configure -cursor watch
    update
    set demo [string range [lindex $tags $i] 5 end]
    uplevel [list source [file join $tk_library demos $demo.tcl]]
    update
    .t configure -cursor $cursor

    .t tag add visited "$index linestart +1 chars" "$index lineend -1 chars"
}

# showStatus --
#
#	Show the name of the demo program in the status bar. This procedure
#	is called when the user moves the cursor over a demo description.
#
proc showStatus index {
    global tk_library
    set tags [.t tag names $index]
    set i [lsearch -glob $tags demo-*]
    set cursor [.t cget -cursor]
    if {$i < 0} {
	.statusBar.lab config -text " "
	set newcursor xterm
    } else {
	set demo [string range [lindex $tags $i] 5 end]
	.statusBar.lab config -text "Run the \"$demo\" sample program"
	set newcursor hand2
    }
    if [string compare $cursor $newcursor] {
	.t config -cursor $newcursor
    }
}


# showCode --
# This procedure creates a toplevel window that displays the code for
# a demonstration and allows it to be edited and reinvoked.
#
# Arguments:
# w -		The name of the demonstration's window, which can be
#		used to derive the name of the file containing its code.

proc showCode w {
    global tk_library
    set file [string range $w 1 end].tcl
    if ![winfo exists .code] {
	toplevel .code
	frame .code.buttons
	pack .code.buttons -side bottom -fill x
	button .code.buttons.dismiss -text Dismiss \
            -default active -command "destroy .code"
	button .code.buttons.rerun -text "Rerun Demo" -command {
	    eval [.code.text get 1.0 end]
	}
	pack .code.buttons.dismiss .code.buttons.rerun -side left \
	    -expand 1 -pady 2
	frame .code.frame
	pack  .code.frame -expand yes -fill both -padx 1 -pady 1
	text .code.text -height 40 -wrap word\
	    -xscrollcommand ".code.xscroll set" \
	    -yscrollcommand ".code.yscroll set" \
	    -setgrid 1 -highlightthickness 0 -pady 2 -padx 3
	scrollbar .code.xscroll -command ".code.text xview" \
	    -highlightthickness 0 -orient horizontal
	scrollbar .code.yscroll -command ".code.text yview" \
	    -highlightthickness 0 -orient vertical

	grid .code.text -in .code.frame -padx 1 -pady 1 \
	    -row 0 -column 0 -rowspan 1 -columnspan 1 -sticky news
	grid .code.yscroll -in .code.frame -padx 1 -pady 1 \
	    -row 0 -column 1 -rowspan 1 -columnspan 1 -sticky news
#	grid .code.xscroll -in .code.frame -padx 1 -pady 1 \
#	    -row 1 -column 0 -rowspan 1 -columnspan 1 -sticky news
	grid rowconfig    .code.frame 0 -weight 1 -minsize 0
	grid columnconfig .code.frame 0 -weight 1 -minsize 0
    } else {
	wm deiconify .code
	raise .code
    }
    wm title .code "Demo code: [file join $tk_library demos $file]"
    wm iconname .code $file
    set id [open [file join $tk_library demos $file]]
    .code.text delete 1.0 end
    .code.text insert 1.0 [read $id]
    .code.text mark set insert 1.0
    close $id
}

# aboutBox --
#
#	Pops up a message box with an "about" message
#
proc aboutBox {} {
    tk_messageBox -icon info -type ok -title "About SilStrip" -message \
"SilStrip --  a program to control the Silicon Strip Chip System\n\n"
#showVars  mask0 mask1 mask2
}

proc FileBox {Direction} {
    set font -*-Helvetica-Medium-R-Normal--*-120-*-*-*-*-*-*
    set w .filebox
    catch {destroy $w}
    toplevel $w
    wm title $w "File Selection"
    wm iconname $w "filebox"
    positionWindow $w

    label $w.msg -font $font -wraplength 4i -justify left -text "Enter a file name in the entry box or click on the \"Browse\" buttons to select a file name using the file selection dialog."
    pack $w.msg -side top

    frame $w.buttons
    pack $w.buttons -side bottom -fill x -pady 2m
    button $w.buttons.cancel -text Cancel -command "destroy $w"
    button $w.buttons.ok -text OK -command "destroy $w"
    pack $w.buttons.ok $w.buttons.cancel -side left -expand 1
    
	set g [frame $w.open]
	label $g.lab -text "Select a file to $Direction: " -anchor e
	entry $g.ent -width 20 -textvariable FileName
	button $g.but -text "Browse ..." -command "fileDialog $w $g.ent $Direction"
	pack $g.lab -side left
	pack $g.ent -side left -expand yes -fill x
	pack $g.but -side left
	pack $g -fill x -padx 1c -pady 3
    
    }

proc fileDialog {wind ent operation} {
    #   Type names		Extension(s)	Mac File Type(s)
    #
    #---------------------------------------------------------
    set types {
	{"Setup files"          {.setup .Setup} }
	{"Text files"		{.txt .doc}	}
	{"Text files"		{}		}
	{"All files"		*}
    }
    if {$operation == "load"} {
	set file [tk_getOpenFile -filetypes $types -parent $wind]
	puts "filename is $file"
	LoadFile $file
	reload
	set CurrentChip 1
	set CurrentChannel 0
	ChangeChip $CurrentChip $CurrentChannel
    } else {
	set file [tk_getSaveFile -filetypes $types -parent $wind \
	    -initialfile Untitled -defaultextension .setup]
	puts "filename is $file"
	SaveFile $file
    }
    if [string compare $file ""] {
	$ent delete 0 end
	$ent insert 0 $file
	$ent xview end
    }
}

proc XLMConfig {} {

    source loadlv1.tcl
}