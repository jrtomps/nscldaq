#!/bin/wish
# silstrip --
# top level program to control the silicon strip chip system
#
# RCS: @(#) $Id: silstrip,v 1.2 1998/09/14 18:23:30 elson Exp $

set defaultIncrFile "/home/hiratest/Current/HINP3/IncrData.incr"

#  Tk_Init
tk appname SilStrip99
wm title . "Silicon Strip Control"
# following line restores classic widget behavior
tk::classic::restore
set widgetDemo 1
set font -*-Helvetica-Medium-R-Normal--*-60-*-*-*-*-*-*
set smallfont -*-Helvetica-Medium-R-Normal--*-60-*-*-*-*-*-*

# Figure out where this script is and
# add the directory above us to the package load path;
# this latter is assuming we got installed in a subdirectory of the
# TclLibs directory tree.

set thisDirectory [file normalize [file dirname [info script]]]
lappend auto_path [file normalize [file join $thisDirectory ..]]

#  Load FPGA firmware GUI



source [file join $thisDirectory loader.tcl]
source [file join $thisDirectory notebook.tcl]
source [file join $thisDirectory tabnbook.tcl]
source [file join $thisDirectory  mclistbox.tcl]



tabnotebook_create .main
pack .main -side left -expand 1 -fill both

##################################
set discpage [tabnotebook_page .main "Discriminators"]

set silpage [tabnotebook_page .main "Chip DAC's"]

tabnotebook_refresh .main
tabnotebook_display .main "Discriminators"

InitXLMs

set CurrentBoard 1

set TotBoards [GetMaxMB]

source [file join $thisDirectory motherboard1.tcl]

#----------------------------------------------------------------
# The code below create the main window, consisting of a menu bar
# and a text widget that explains how to use the program, plus lists
# all of the demos as hypertext items.
#----------------------------------------------------------------
option add *Menubutton.selectColor: red
set font {Helvetica 14}
set midfont {Helvetica -14 bold}
menu .menuBar -tearoff 0
.menuBar add cascade -menu .menuBar.file -label "File" -underline 0
menu .menuBar.file -tearoff 0


    .menuBar.file add command -label "About..." -command "aboutBox" \
	-underline 0 -accelerator "<F1>"
.menuBar.file add command -label "Load" -command {FileBox load} \
	-underline 0 -accelerator "<F2>"
.menuBar.file add command -label "Save" -command {FileBox save} \
	-underline 0 -accelerator "<F3>"
.menuBar.file add sep
.menuBar.file add command -label "Initialize Selected Board" -command {ChipInitial $CurrentBoard} \
	-underline 0
.menuBar.file add command -label "Initialize All Boards" -command {InitAll} \
	-underline 0
.menuBar.file add command -label "XLM Configure" -command {XLMConfig} \
	-underline 0
.menuBar.file add command -label "Reset Selected XLM" \
    -command {ResetXLM $CurrentBoard} \
	-underline 0
.menuBar.file add command -label "Load Increment File" \
    -command {LoadIncrement $defaultIncrFile} 
    .menuBar.file add sep


.menuBar.file add command -label "Quit" -command "exit" -underline 0 \
    -accelerator "Meta-Q"
. configure -menu .menuBar
bind . <F1> aboutBox

frame .statusBar
label .statusBar.lab -text "   " -relief sunken -bd 1 \
    -font -*-Helvetica-Medium-R-Normal--*-60-*-*-*-*-*-* -anchor w
label .statusBar.foo -width 8 -relief sunken -bd 1 \
    -font -*-Helvetica-Medium-R-Normal--*-60-*-*-*-*-*-* -anchor w
pack .statusBar.lab -side left -padx 2 -expand yes -fill both
pack .statusBar.foo -side left -padx 2
pack .statusBar -side bottom -fill x -pady 2

frame $silpage.textFrame
scrollbar .s -orient vertical -command {.t yview} -highlightthickness 0 \
    -takefocus 1
pack .s -in $silpage.textFrame -side right -fill y
#text .t -yscrollcommand {.s set} -wrap word -width 60 -height 4 -font $font \
#    -setgrid 1 -highlightthickness 0 -padx 4 -pady 2 -takefocus 0
#pack .t -in $silpage.textFrame -expand y -fill both -padx 1
#pack  $silpage.textFrame -expand yes -fill both

# Create a bunch of tags to use in the text widget, such as those for
# section titles and demo descriptions.  Also define the bindings for
# tags.

#.t tag configure title -font {Helvetica 18 bold}

# We put some "space" characters to the left and right of each demo description
# so that the descriptions are highlighted only when the mouse cursor
# is right over them (but not when the cursor is to their left or right)
#
#.t tag configure demospace -lmargin1 1c -lmargin2 1c


# if {[winfo depth .] == 1} {
#     .t tag configure demo -lmargin1 1c -lmargin2 1c \
# 	-underline 1
#     .t tag configure visited -lmargin1 1c -lmargin2 1c \
# 	-underline 1
#     .t tag configure hot -background black -foreground white
# } else {
#     .t tag configure demo -lmargin1 1c -lmargin2 1c \
# 	-foreground blue -underline 1
#     .t tag configure visited -lmargin1 1c -lmargin2 1c \
# 	-foreground #303080 -underline 1
#     .t tag configure hot -foreground red -underline 1
# }
# .t tag bind demo <ButtonRelease-1> {
#     invoke [.t index {@%x,%y}]
# }
# set lastLine ""
# .t tag bind demo <Enter> {
#     set lastLine [.t index {@%x,%y linestart}]
#     .t tag add hot "$lastLine +1 chars" "$lastLine lineend -1 chars"
#     .t config -cursor hand2
#     showStatus [.t index {@%x,%y}]
# }
# .t tag bind demo <Leave> {
#     .t tag remove hot 1.0 end
#     .t config -cursor xterm
#     .statusBar.lab config -text ""
# }
# .t tag bind demo <Motion> {
#     set newLine [.t index {@%x,%y linestart}]
#     if {[string compare $newLine $lastLine] != 0} {
# 	.t tag remove hot 1.0 end
# 	set lastLine $newLine

# 	set tags [.t tag names {@%x,%y}]
# 	set i [lsearch -glob $tags demo-*]
# 	if {$i >= 0} {
# 	    .t tag add hot "$lastLine +1 chars" "$lastLine lineend -1 chars"
# 	}
#     }
#     showStatus [.t index {@%x,%y}]
# }

# Create the text for the text widget.

#.t insert end "Silicon Strip Chip Control Rev. H\n" title
#.t insert end "Special revision for timing tests.\n"
# .t insert end {
# This application controls one or more Silicon Strip Chips in a data
# acquisition system.
# }
# .t insert end "Controls" title
#.t insert end " \n " {demospace}

# global "defines"
set NumChannels 32

global silpage
global gain polarity tvcRange test1 extshap disc
global CSARef ResCV ARef EOff TOff CSAOff ShapOff
global ZC2 DACRef CFDRef
global CFDCap
global threshLeft threshRight
global Location
global isiton
global setAll
set setAll 0
set isiton 0
set CurrentBoard 1
set CurrentChip 1
set CurrentChannel 0
set CSAOff 0
set ZC2    0
set DACRef    0
set CFDRef  0
set ShapOff 0
set gain "high    "
set polarity positive
set tvcRange "short"
set test1 "No "
set extshap "No "
set disc "All     "
set threshLeft  "unknown"
set threshRight "unknown"
set Location "undefined"
set CFDCap 12

frame $silpage.top
frame $silpage.mid
frame $silpage.mid1
frame $silpage.mid2
pack $silpage.top $silpage.mid -side top -expand yes -pady 0 -anchor w
pack $silpage.mid1 $silpage.mid2 $silpage.mid -side top -expand yes -pady 0 -anchor s

frame $silpage.top.pol
frame $silpage.top.gain
frame $silpage.top.board
frame $silpage.top.chip
frame $silpage.top.chan
frame $silpage.top.cfdcap
frame $silpage.top.control

label $silpage.top.pol.l -font $font -wraplength 5i -justify left -text "Signal Polarity"
label $silpage.top.gain.l -font $font -wraplength 5i -justify left -text "Gain"
label $silpage.top.board.l -font $font -wraplength 5i -justify left -text "MotherBoard"
label $silpage.top.chip.l -font $font -wraplength 5i -justify left -text "Chip Board"
label $silpage.top.chan.l -font $font -wraplength 5i -justify left -text "Channel"
label $silpage.top.cfdcap.l -font $font -wraplength 5i -justify left -text "CFD Cap"

pack $silpage.top.pol $silpage.top.gain $silpage.top.board \
    $silpage.top.chip $silpage.top.chan $silpage.top.control \
    $silpage.top.cfdcap \
    -side left -expand yes  -pady .5 -padx .5

foreach polarity {positive negative} {
    set lower [string tolower $polarity]
    radiobutton $silpage.top.pol.$lower -text $polarity -variable polarity \
	    -relief flat -value $lower -selectcolor red -indicatoron false -width 8
    pack $silpage.top.pol.l $silpage.top.pol.$lower -side top -pady 2 -anchor w
    bind $silpage.top.pol.$lower <ButtonRelease-1> {
	SetChipPolarity $CurrentBoard $CurrentChip $polarity 
	UpdateText
    }
    bind $silpage.top.pol.$lower <ButtonRelease-1> {+
	if {$polarity == "positive"} {
	    set threshLeft   "Higher"
	    set threshRight  "Lower"} else {
		set threshLeft   "Lower"
		set threshRight  "Higher"}
	    }
}

foreach gain {"high    " "low     " "external"} {
    set lower [string tolower $gain]
    radiobutton $silpage.top.gain.$lower -text $gain -variable gain \
	    -relief flat -value $lower -selectcolor red -indicatoron false -width 8
    pack $silpage.top.gain.l $silpage.top.gain.$lower -side top -pady 2 -anchor w
bind $silpage.top.gain.$lower <ButtonRelease-1> \
	{SetChipGain $CurrentBoard $CurrentChip $gain }
}

scale $silpage.top.board.scale -orient horizontal -length 125 -from 1 -to $TotBoards \
    -tickinterval 1 -variable CurrentBoard
pack $silpage.top.board.l $silpage.top.board.scale \
    -side top -pady 2 -anchor w

scale $silpage.top.chip.scale -orient horizontal -length 150 -from 1 -to 16 \
	-tickinterval 6 -variable CurrentChip
	pack $silpage.top.chip.l $silpage.top.chip.scale -side top -pady 2 -anchor w

scale $silpage.top.chan.scale -orient horizontal -length 125 -from 0 -to 31 \
	-tickinterval 8 -variable CurrentChannel
	pack $silpage.top.chan.l $silpage.top.chan.scale -side top -pady 2 -anchor w

foreach CFDCap {5 12} {
    set lower [string tolower $CFDCap]
    radiobutton $silpage.top.cfdcap.$lower -text $CFDCap -variable CFDCap \
	    -relief flat -value $lower -selectcolor red -indicatoron false -width 5
    pack $silpage.top.cfdcap.l $silpage.top.cfdcap.$lower -side top -pady 2 -anchor w
bind $silpage.top.cfdcap.$lower <ButtonRelease-1> \
	{SetCFDCap $CurrentBoard $CurrentChip $CFDCap }
}

bind $silpage.top.board.scale <ButtonRelease-1> \
    {ChangeChip $CurrentBoard $CurrentChip $CurrentChannel}
bind $silpage.top.chip.scale <ButtonRelease-1> \
    {ChangeChip $CurrentBoard $CurrentChip $CurrentChannel}
bind $silpage.top.chan.scale <ButtonRelease-1> \
    {ChangeChannel $CurrentBoard $CurrentChip $CurrentChannel $disc}

frame $silpage.top.location
label $silpage.top.location.l -font $font -textvariable Location
    pack $silpage.top.location $silpage.top.location.l -side bottom -pady 1 -anchor n


frame $silpage.mid1.bleft
frame $silpage.mid1.bleft.tvc
#frame $silpage.mid1.bleft.test1
frame $silpage.mid1.bleft.extshap
frame $silpage.mid1.bleft.wZC2
frame $silpage.mid1.bleft.wDACRef
frame $silpage.mid1.bleft.wShapOff
frame $silpage.mid1.bleft.wCSAOff
frame $silpage.mid1.serdac

frame $silpage.mid2.serdac

label $silpage.mid1.bleft.wShapOff.l -font $font -wraplength 4i -justify right -text "Shaper Offset"
label $silpage.mid1.bleft.wCSAOff.l -font $font -wraplength 4i -justify right -text "CSA Offset"
label $silpage.mid1.bleft.wZC2.l -font $font -wraplength 4i -justify right -text "ZC2 Offset"
label $silpage.mid1.bleft.wDACRef.l -font $font -wraplength 4i -justify right -text "DAC Offset"

scale $silpage.mid1.bleft.wShapOff.scale -orient horizontal -length 150  -from 0 -to 1023 \
	-tickinterval 256 -variable ShapOff -borderwidth 2 -font $smallfont
pack $silpage.mid1.bleft.wShapOff.l $silpage.mid1.bleft.wShapOff.scale -side left -pady 0 -anchor e
bind $silpage.mid1.bleft.wShapOff.scale <ButtonRelease-1> \
    {SetChipShapOff $CurrentBoard $CurrentChip $ShapOff}

scale $silpage.mid1.bleft.wCSAOff.scale -orient horizontal -length 150  -from 0 -to 1023 \
	-tickinterval 256 -variable CSAOff -font $smallfont
pack $silpage.mid1.bleft.wCSAOff.l $silpage.mid1.bleft.wCSAOff.scale -side left -pady 0 -anchor e
bind $silpage.mid1.bleft.wCSAOff.scale <ButtonRelease-1> \
    {SetChipCSAOff $CurrentBoard $CurrentChip $CSAOff}

scale $silpage.mid1.bleft.wZC2.scale -orient horizontal -length 150  -from 0 -to 1023 \
	-tickinterval 256 -variable ZC2 -font $smallfont
pack $silpage.mid1.bleft.wZC2.l $silpage.mid1.bleft.wZC2.scale -side left -pady 0 -anchor e
bind $silpage.mid1.bleft.wZC2.scale <ButtonRelease-1> \
    {SetChipZC2 $CurrentBoard $CurrentChip $ZC2}

scale $silpage.mid1.bleft.wDACRef.scale -orient horizontal -length 150  -from 0 -to 1023 \
	-tickinterval 256 -variable DACRef -font $smallfont
pack $silpage.mid1.bleft.wDACRef.l $silpage.mid1.bleft.wDACRef.scale -side left -pady 0 -anchor e
bind $silpage.mid1.bleft.wDACRef.scale <ButtonRelease-1> \
    {SetChipDACRef $CurrentBoard $CurrentChip $DACRef}

pack $silpage.mid1.bleft.wZC2 $silpage.mid1.bleft.wDACRef \
    $silpage.mid1.bleft.wShapOff $silpage.mid1.bleft.wCSAOff  $silpage.mid1.bleft \
   -side bottom -pady 0 -anchor w
pack $silpage.mid1.bleft.tvc $silpage.mid1.bleft.extshap \
  $silpage.mid1.bleft -side left -expand no  -pady .5 -padx 0.0 -anchor e
pack $silpage.mid1.bleft \
 $silpage.mid1.serdac -side left -expand yes  -pady .5 -padx 0.0 -anchor e
label $silpage.mid1.bleft.tvc.l -font $font -wraplength 5i -justify left -text "TVC Range"
#label $silpage.mid1.bleft.test1.l -font $font -wraplength 5i -justify left -text "Show CSA/Shap Sigs"
label $silpage.mid1.bleft.extshap.l -font $font -wraplength 5i -justify left -text "Shaper"
label $silpage.mid1.serdac.l -font $font -wraplength 5i -justify left -text "DAC Settings"

foreach tvcRange {"short" "long "} {
    set lower [string tolower $tvcRange]
    radiobutton $silpage.mid1.bleft.tvc.$lower -text $tvcRange -variable tvcRange \
	    -relief flat -value $lower -selectcolor red -indicatoron false -width 7
    pack $silpage.mid1.bleft.tvc.l $silpage.mid1.bleft.tvc.$lower -side top -pady 1 -anchor w
    bind $silpage.mid1.bleft.tvc.$lower <ButtonRelease-1> \
	{SetChipTVCRange $CurrentBoard $CurrentChip $tvcRange }
}

# foreach test1 {"exclusive" "yes      " "no       "} {
#     set lower [string tolower $test1]
#     radiobutton $silpage.mid1.bleft.test1.$lower -text $test1 -variable test1 \
# 	    -relief flat -value $lower -selectcolor red
#     pack $silpage.mid1.bleft.test1.l $silpage.mid1.bleft.test1.$lower -side top -pady 1 -anchor w
#     bind $silpage.mid1.bleft.test1.$lower <ButtonRelease-1> \
# 	{SetChipTest1 $CurrentBoard $CurrentChip $test1 }
# }

foreach extshap {Internal External} {
    set lower [string tolower $extshap]
    radiobutton $silpage.mid1.bleft.extshap.$lower -text $extshap -variable extshap \
	    -relief flat -value $lower -selectcolor red -indicatoron false -width 8
    pack $silpage.mid1.bleft.extshap.l $silpage.mid1.bleft.extshap.$lower -side top -pady 1 -anchor w
    bind $silpage.mid1.bleft.extshap.$lower <ButtonRelease-1> \
	{SetChipExtShaper $CurrentBoard $CurrentChip $extshap }
}

frame $silpage.mid1.serdac.wCSARef
frame $silpage.mid1.serdac.wResCV
frame $silpage.mid1.serdac.wARef
frame $silpage.mid1.serdac.wCFDRef
frame $silpage.mid1.serdac.wEOff
frame $silpage.mid1.serdac.wTOff


#bind . <Control-l><Control-f> ControlFrame

#proc ControlFrame {} {
#    global isiton
#    global silpage

#    if {$isiton == 1} {
#	set isiton 0
#	pack forget $silpage.mid1.serdac.wCSARef $silpage.mid1.serdac.wResCV $silpage.mid1.serdac.wARef \
#    -side top -expand no -pady 0 -padx 0
#    } else {
#	set isiton 1
#	pack  $silpage.mid1.serdac.wCSARef $silpage.mid1.serdac.wResCV $silpage.mid1.serdac.wARef \
#    -side top -expand no -pady 0 -padx 0
#    }
#}
	pack  $silpage.mid1.serdac.wCSARef $silpage.mid1.serdac.wResCV $silpage.mid1.serdac.wARef \
    -side top -expand no -pady 0 -padx 0
#
label $silpage.mid1.serdac.wCSARef.l -font $font -wraplength 5i -justify left -text "CSA Ref"
label $silpage.mid1.serdac.wResCV.l -font $font -wraplength 5i -justify left -text "Reset CV"
label $silpage.mid1.serdac.wARef.l -font $font -wraplength 5i -justify left -text "Analog Ref"
label $silpage.mid1.serdac.wCFDRef.l -font $font -wraplength 5i -justify left -text "CFD Ref"
label $silpage.mid1.serdac.wEOff.l -font $font -wraplength 5i -justify left -text "Energy Offset"
label $silpage.mid1.serdac.wTOff.l -font $font -wraplength 5i -justify left -text "Time Offset"

scale $silpage.mid1.serdac.wCSARef.scale -orient horizontal -length 300  -from 0 -to 1023 \
	-tickinterval 256 -variable CSARef -borderwidth 2 -font $smallfont
pack $silpage.mid1.serdac.wCSARef.l $silpage.mid1.serdac.wCSARef.scale -side left -pady 0 -anchor w -expand no
bind $silpage.mid1.serdac.wCSARef.scale <ButtonRelease-1> \
    {SetChipCSARef $CurrentBoard $CurrentChip $CSARef}

scale $silpage.mid1.serdac.wResCV.scale -orient horizontal -length 300  -from 0 -to 1023 \
	-tickinterval 256 -variable ResCV -borderwidth 2 -font $smallfont
pack $silpage.mid1.serdac.wResCV.l $silpage.mid1.serdac.wResCV.scale -side left -pady 0 -anchor w -expand no
bind $silpage.mid1.serdac.wResCV.scale <ButtonRelease-1> \
    {SetChipResCV $CurrentBoard $CurrentChip $ResCV}

scale $silpage.mid1.serdac.wARef.scale -orient horizontal -length 300  -from 0 -to 1023 \
	-tickinterval 256 -variable ARef -borderwidth 2 -font $smallfont
pack $silpage.mid1.serdac.wARef.l $silpage.mid1.serdac.wARef.scale -side left -pady 0 -anchor w -expand no
bind $silpage.mid1.serdac.wARef.scale <ButtonRelease-1> \
    {SetChipARef $CurrentBoard $CurrentChip $ARef}

scale $silpage.mid1.serdac.wCFDRef.scale -orient horizontal -length 300  -from 0 -to 1023 \
	-tickinterval 256 -variable CFDRef -borderwidth 2 -font $smallfont
pack $silpage.mid1.serdac.wCFDRef.l $silpage.mid1.serdac.wCFDRef.scale -side left -pady 0 -anchor w -expand no
bind $silpage.mid1.serdac.wCFDRef.scale <ButtonRelease-1> \
    {SetChipCFDRef $CurrentBoard $CurrentChip $CFDRef}

scale $silpage.mid1.serdac.wEOff.scale -orient horizontal -length 300  -from 0 -to 1023 \
	-tickinterval 256 -variable EOff -borderwidth 2 -font $smallfont
pack $silpage.mid1.serdac.wEOff.l $silpage.mid1.serdac.wEOff.scale -side left -pady 0 -anchor w
bind $silpage.mid1.serdac.wEOff.scale <ButtonRelease-1> \
    {SetChipEOff $CurrentBoard $CurrentChip $EOff}

scale $silpage.mid1.serdac.wTOff.scale -orient horizontal -length 300  -from 0 -to 1023 \
	-tickinterval 256 -variable TOff -font $smallfont
pack $silpage.mid1.serdac.wTOff.l $silpage.mid1.serdac.wTOff.scale -side left -pady 0 -anchor w
bind $silpage.mid1.serdac.wTOff.scale <ButtonRelease-1> \
    {SetChipTOff $CurrentBoard $CurrentChip $TOff}

pack $silpage.mid1.serdac.wEOff \
    $silpage.mid1.serdac.wTOff $silpage.mid1.serdac.wCFDRef -side bottom -expand no -pady 0 -padx 0 -anchor w

#DDSC 8/31/09 moved disc stuff to new page

#InitChip
#
# ChangeChip --
#     select a chip in hardware
#     also brings in all global settings for that chip from data base
proc ChangeChip {CurrentBoard CurrentChip CurrentChannel} {
    global silpage
    global gain polarity tvcRange test1 extshap disc Threshold
    global CSARef ResCV ARef EOff TOff CFDCap
    global ZC2 DACRef CFDRef
    global hold1,hold2,hold3,hold4,hold5,hold6
    global threshLeft threshRight
    global Location

# set all variables to dummy initial value, just so they aren't unassigned
# When selected chip is changed, get values from database and set screen representation
    #puts stdout [format "ChangeChip %d" $CurrentChip]
    #puts stdout [format "  gain = %s" $gain]
    SelectBoard $CurrentBoard
    SelectChip $CurrentBoard $CurrentChip
    set gain [GetGain $CurrentBoard $CurrentChip]
    set polarity [GetPolarity $CurrentBoard $CurrentChip]
    set tvcRange [GetTVCRange $CurrentBoard $CurrentChip]
    set test1 [GetTest1 $CurrentBoard $CurrentChip]
    set extshap [GetExtShaper $CurrentBoard $CurrentChip]
    set disc [GetDiscMode $CurrentBoard $CurrentChip]
    set CSARef [GetCSARef $CurrentBoard $CurrentChip]
    set ResCV [GetResCV $CurrentBoard $CurrentChip]
    set ARef  [GetARef $CurrentBoard $CurrentChip]
    set ZC2   [GetZC2 $CurrentBoard $CurrentChip]
    set DACRef  [GetDACRef $CurrentBoard $CurrentChip]
    set CFDRef  [GetCFDRef $CurrentBoard $CurrentChip]
    set EOff    [GetEOff $CurrentBoard $CurrentChip]
    set TOff    [GetTOff $CurrentBoard $CurrentChip]
    set CFDCap  [GetCFDCap $CurrentBoard $CurrentChip]
    set newchip [GetNew $CurrentBoard $CurrentChip]
    puts stdout [format "new %s" $newchip]
    if {$newchip != "New"} {
	pack forget $silpage.mid1.serdac.wCFDRef $silpage.mid1.bleft.wZC2 $silpage.mid1.bleft.wDACRef \
    -side top -expand no -pady 0 -padx 0
    } else {
	pack  $silpage.mid1.serdac.wCFDRef
	pack  configure $silpage.mid1.bleft.wZC2 $silpage.mid1.bleft.wDACRef \
	    -after $silpage.mid1.bleft.wCSAOff \
	    -side bottom -expand no -pady 0 -padx 0 -anchor w
    }
    #puts stdout [format "SelectChip %d called" $CurrentChip]
    #puts stdout [format "  gain = %s" $gain]
    #puts stdout [format "  polarity = %s" $polarity]
    #puts stdout [format "  Disc mode = %s" $disc]
    #puts stdout [format "  TVC  = %s" $tvcRange]
    if {$polarity == "positive"} {
	set threshLeft   "<Higher"
	set threshRight  "Lower>"} else {
	set threshLeft   "<Lower"
	set threshRight  "Higher>"}

    # now update effectively selected channel changes
    ChangeDiscMode $CurrentBoard $CurrentChip $disc
    set Threshold \
	[GetThreshold $CurrentBoard $CurrentChip $CurrentChannel ] ;# refresh the displayed threshold
   set Location [GetChipLocation $CurrentBoard $CurrentChip ] ;#refresh displayed chip location
    UpdateText
}

proc ChangeDiscMode {CurrentBoard CurrentChip DiscMode } {
global NumChannels CurrentChannel
global mask0 mask1 mask2 mask3 mask4 mask5 mask6 mask7
global mask8 mask9 mask10 mask11 mask12 mask13 mask14 mask15
global mask16 mask17 mask18 mask19 mask20 mask21 mask22 mask23
global mask24 mask25 mask26 mask27 mask28 mask29 mask30 mask31
    puts stdout [format "ChangeDiscMode %s %s" $CurrentChip $DiscMode]
    SetChipDiscMode $CurrentBoard $CurrentChip $DiscMode
# update the check buttons of the disc mask
    switch -exact -- $DiscMode {
	"all     " {
	    for {set i 0} {$i < $NumChannels} {incr i} {
		set mask$i "1"
	    }
	}
	"none    " {
	    for {set i 0} {$i < $NumChannels} {incr i} {
		set mask$i "0"
	    }
	}
	"selected" {
	    for {set i 0} {$i < $NumChannels} {incr i} {
		set mask$i "0"
	    }
	    set mask$CurrentChannel "1"
	}
	"odds    " {
	    for {set i 0} {$i < $NumChannels} {incr i} {
		if {[expr {fmod($i,2) !=0} ]} {
		set mask$i "1"
		} else {
		    set mask$i "0"
		}
	    }
	}
	"evens   " {
	    for {set i 0} {$i < $NumChannels} {incr i} {
		if {[expr {fmod($i,2) != 0} ]} {
		    set mask$i "0"
		} else {
		    set mask$i "1"
		}
	    }
	}
	"mask    " {
	    set aa [GetDiscMask $CurrentBoard $CurrentChip]
	    #puts stdout [format "GetDiscMask %s" $aa]
	    for {set i 0} {$i < $NumChannels} {incr i} {
		if {[expr {($aa & 1 << $i) != 0} ]} {
		set mask$i "1"
		} else {
		    set mask$i "0"
		}
	    }
	}
    }
#    set i 1
#    UpdateMotherboard $i
    UpdateMotherboard $CurrentBoard
    UpdateThresholdDisp
}

proc ChangeChannel {CurrentBoard CurrentChip CurrentChannel DiscMode} {
    global Threshold
    SelectChannel $CurrentBoard $CurrentChannel  ;# update C database
    ChangeDiscMode $CurrentBoard $CurrentChip $DiscMode    ;# refresh displayed disc mask bits
    set Threshold [GetThreshold $CurrentBoard $CurrentChip $CurrentChannel ] ;# refresh the displayed threshold
    
    UpdateText
}

#.t configure -state disabled
focus .s
# positionWindow --
# This procedure is invoked by most of the demos to position a
# new demo window.
#
# Arguments:
# w -		The name of the window to position.

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
	"SilStrip --  a program to control the Silicon Strip Chip System\n\n
Contact: Jon Elson: jmelson@artsci.wustl.edu\n
Michael Famiano: famiano@nscl.msu.edu"
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
    
    label $w.msg -font $font -wraplength 4i -justify left -text \
    "Enter a file name in the entry box or click on the \"Browse\" buttons to select a file name using the file selection dialog."
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
    global fpgaslot
    global CurrentBoard
    global TotBoards
    global fpgacrate
    global xtype
    #   Type names		Extension(s)	Mac File Type(s)
    #
    #---------------------------------------------------------
    set types {
	{"Setup files"          {.setup .Setup} }
	{"Bit files"            {.bit .Bit .BIT} }
	{"Text files"		{.txt .doc}	}
	{"Text files"		{}		}
	{"All files"		*}
    }
    if {$operation == "load"} {
	InitAll
	set file [tk_getOpenFile -filetypes $types -parent $wind]
	puts "filename is $file"
	LoadFile $file
	reload $CurrentBoard
	set CurrentBoard 1
	set CurrentChip 1
	set CurrentChannel 0
	ChangeChip $CurrentBoard $CurrentChip $CurrentChannel
    }

    if {$operation == "save"} {
	set file [tk_getSaveFile -filetypes $types -parent $wind \
	    -initialfile Untitled -defaultextension .setup]
	puts "filename is $file"
	SaveFile $file
    }

    if {$operation == "config"} {
	set file [tk_getOpenFile -filetypes $types -parent $wind]
	LoadFPGA $file $fpgacrate $fpgaslot $xtype
	InitAll
    }

    if [string compare $file ""] {
	$ent delete 0 end
	$ent insert 0 $file
	$ent xview end
    }
}

proc LoadIncrement {file} {

    global CurrentBoard CurrentChip CurrentChannel Threshold

    puts "increment filename is $file"
    LoadIncr $file
    
    #update GUI
    #set Threshold [GetThreshold $CurrentBoard $CurrentChip $CurrentChannel]
    UpdateThresholdDisp
    #UpdateThresholdDisp
}

frame .conttitle
label .conttitle.title -font $font -wraplength 5i -justify left -text "XLM Configuration"

frame .controls
button .controls.exit -text Accept -command LeaveConfig
button .controls.fifo -text "Pick Load File" -command "FileBox config"


frame .seltitle
label .seltitle.title -font $font -wraplength 5i -justify left -text "Slot"
frame .selector
scale .selector.slotnum -orient vertical -length 125 -from 0 -to 22 \
	-tickinterval 4 -variable fpgaslot

scale .selector.cratenum -orient vertical -length 125 -from 0 -to 8 \
	-tickinterval 4 -variable fpgacrate
label .selector.crtit -font $font -wraplength 5i -justify left -text "Crate"
label .selector.seltitX -font $font -wraplength 5i -justify left -text "XXV"
label .selector.seltit8 -font $font -wraplength 5i -justify left -text "XLM80"

scale .selector.xlmtype -orient vertical -length 50 -from 0 -to 1 \
    -tickinterval 0 -variable xtype -showvalue 0

proc XLMConfig {} {
    SetupGUI
}

proc ChipInitial {BoardNum} {
    InitChip $BoardNum
}

proc InitAll {} {
    global TotBoards

    set BT [expr $TotBoards + 1]
    	for {set i 1} {$i < $BT} {incr i} {
	    InitChip $i
	}
}

# discpage GUI setup

set dispbg gray70
frame $discpage.top 
frame $discpage.bot
pack $discpage.top $discpage.bot -side top -expand yes -pady 0 -anchor w

frame $discpage.top.board
frame $discpage.top.chip
frame $discpage.top.chan
pack $discpage.top.board $discpage.top.chip $discpage.top.chan -side left -expand yes -pady .5 -padx .5

label $discpage.top.board.l -font $font -wraplength 5i -justify left -text "MotherBoard"
label $discpage.top.chip.l -font $font -wraplength 5i -justify left -text "Chip Board"
label $discpage.top.chan.l -font $font -wraplength 5i -justify left -text "Channel"

scale $discpage.top.board.scale -orient horizontal -length 125 -from 1 -to $TotBoards \
    -tickinterval 1 -variable CurrentBoard
pack $discpage.top.board.l $discpage.top.board.scale \
    -side top -pady 2 -anchor w

scale $discpage.top.chip.scale -orient horizontal -length 150 -from 1 -to 16 \
	-tickinterval 6 -variable CurrentChip
pack $discpage.top.chip.l $discpage.top.chip.scale -side top -pady 2 -anchor w

scale $discpage.top.chan.scale -orient horizontal -length 125 -from 0 -to 31 \
	-tickinterval 8 -variable CurrentChannel
pack $discpage.top.chan.l $discpage.top.chan.scale -side top -pady 2 -anchor w

label $discpage.top.location -font $font -textvariable Location
label $discpage.top.polarity -font $font -textvariable polarity

pack $discpage.top.location $discpage.top.polarity -side left -padx 2 -pady 8 -anchor w



bind $discpage.top.board.scale <ButtonRelease-1> \
    {ChangeChip $CurrentBoard $CurrentChip $CurrentChannel}
bind $discpage.top.chip.scale <ButtonRelease-1> \
    {ChangeChip $CurrentBoard $CurrentChip $CurrentChannel}
bind $discpage.top.chan.scale <ButtonRelease-1> \
    {ChangeChannel $CurrentBoard $CurrentChip $CurrentChannel $disc}

frame $discpage.bot.left
frame $discpage.bot.discdisp -bg $dispbg
frame $discpage.bot.mask
pack $discpage.bot.left $discpage.bot.discdisp $discpage.bot.mask -side left \
    -expand yes  -padx .5 -pady .5

frame $discpage.bot.left.thresh
frame $discpage.bot.left.test1
pack $discpage.bot.left.thresh -side top -expand yes -padx .5 -pady .5
pack $discpage.bot.left.test1 -side top -expand yes -padx 2 -pady 30 -fill x

frame $discpage.bot.left.thresh.banner
pack $discpage.bot.left.thresh.banner -side top -fill x -pady 0.5
label $discpage.bot.left.thresh.banner.left -font $font -textvariable  threshLeft
label $discpage.bot.left.thresh.banner.right -font $font -textvariable  threshRight
label $discpage.bot.left.thresh.banner.l -font $font -wraplength 5i -justify left -text "Discriminator Threshold"
pack $discpage.bot.left.thresh.banner.left $discpage.bot.left.thresh.banner.l $discpage.bot.left.thresh.banner.right  -side left -expand yes -pady 0.5 -anchor w
scale $discpage.bot.left.thresh.scale -font $smallfont -orient horizontal -length 320 -from -32 -to 31 \
	-tickinterval 4 -variable Threshold
	pack $discpage.bot.left.thresh.banner $discpage.bot.left.thresh.scale -side top -pady 1 -anchor w
bind $discpage.bot.left.thresh.scale <ButtonRelease-1> {
    SetChipThreshold $CurrentBoard $CurrentChip $CurrentChannel $polarity $Threshold $setAll
    UpdateThresholdDisp
}

set setRadio [frame $discpage.bot.left.thresh.radio]
set setAll 0
radiobutton $setRadio.chan -text "Channel $CurrentChannel only" \
    -variable setAll -relief flat -value 0 -width 25 -borderwidth 1 -selectcolor red -indicatoron false
radiobutton $setRadio.chip -text "All Channels on Chip $CurrentChip" \
    -variable setAll -relief flat -value 1 -width 25 -borderwidth 1 -selectcolor red -indicatoron false
radiobutton $setRadio.mb -text "All $polarity chips on MB $CurrentBoard" \
    -variable setAll -relief flat -value 2 -width 25 -borderwidth 1 -selectcolor red -indicatoron false
radiobutton $setRadio.all -text "All $polarity Boards" \
    -variable setAll -relief flat -value 3 -width 25 -borderwidth 1 -selectcolor red -indicatoron false
pack $setRadio.chan $setRadio.chip $setRadio.mb $setRadio.all -side top -anchor w
pack $setRadio -side top -pady 2 -anchor w

label $discpage.bot.left.test1.l -font $font -wraplength 5i -justify left \
    -text "Show CSA/Shap Sigs"

foreach test1 {"exclusive" "yes      " "no       "} {
    set lower [string tolower $test1]
    radiobutton $discpage.bot.left.test1.$lower -text $test1 -variable test1 \
	    -relief flat -value $lower -selectcolor red -indicatoron false -width 10
    pack $discpage.bot.left.test1.l $discpage.bot.left.test1.$lower -side top -pady 1 -anchor w
    bind $discpage.bot.left.test1.$lower <ButtonRelease-1> {
	SetChipTest1 $CurrentBoard $CurrentChip $test1 
	set disc [GetDiscMode $CurrentBoard $CurrentChip]
	set lower [string tolower $disc]
	ChangeDiscMode $CurrentBoard $CurrentChip $lower
    }
}

# disc channels display
label $discpage.bot.discdisp.lab -font $font -wraplength 120 -textvariable thrDispLab \
    -text "Thresholds and Masks for Chip $CurrentChip" -bg $dispbg
set dispFrame [frame $discpage.bot.discdisp.grid -bg $dispbg]
pack $discpage.bot.discdisp.lab $dispFrame -side top -fill x -pady 3

for {set chan 0} {$chan < 16} {incr chan} {

    set thetext $chan
    if {$chan < 10} {set thetext "$chan "}

    label $dispFrame.lab$chan -text $thetext -background $dispbg
    label $dispFrame.thres$chan -text "  00" -background $dispbg
    checkbutton $dispFrame.mask$chan -variable mask$chan -relief flat -borderwidth 1 \
	-width 2 -command "SetChipDiscMask \$CurrentBoard \$CurrentChip $chan \$mask$chan" \
	-selectcolor red -indicatoron false -highlightthickness 0 -disabledforeground LightSalmon3
    label $dispFrame.space$chan -text "    " -background $dispbg

    grid $dispFrame.lab$chan -row $chan -column 0 -sticky nw -padx 0 -pady 0
    grid $dispFrame.thres$chan -row $chan -column 1 -sticky nw -padx 0 -pady 0
    grid $dispFrame.mask$chan -row $chan -column 2 -sticky nw -padx 0 -pady 0
    grid $dispFrame.space$chan -row $chan -column 3 -sticky nw -padx 4 -pady 0

    bind $dispFrame.lab$chan <ButtonRelease-1> {
	set CurrentChannel [%W cget -text]
	set CurrentChannel [expr $CurrentChannel * 1]
	ChangeChannel $CurrentBoard $CurrentChip $CurrentChannel $disc
    }

}

for {set chan 16} {$chan < 32} {incr chan} {

    set thetext $chan
    if {$chan<10} {set thetext "$chan  "}

    label $dispFrame.lab$chan  -text $thetext -background $dispbg
    label $dispFrame.thres$chan  -text "  00" -background $dispbg
    checkbutton $dispFrame.mask$chan -variable mask$chan -relief flat -borderwidth 1 -width 2 \
	-command "SetChipDiscMask \$CurrentBoard \$CurrentChip $chan \$mask$chan"  \
	-selectcolor red -indicatoron false \
	-highlightthickness 0 -disabledforeground LightSalmon3
    grid $dispFrame.lab$chan -row [expr $chan-16] -column 4 -sticky nw -padx 0
    grid $dispFrame.thres$chan -row [expr $chan-16] -column 5 -sticky nw -padx 0
    grid $dispFrame.mask$chan -row [expr $chan-16] -column 6 -sticky nw -padx 0

    bind $dispFrame.lab$chan <ButtonRelease-1> {
	set CurrentChannel [%W cget -text]
	set CurrentChannel [expr $CurrentChannel * 1]
	ChangeChannel $CurrentBoard $CurrentChip $CurrentChannel $disc
    }
}

$dispFrame.lab$CurrentChannel config -background yellow
$dispFrame.thres$CurrentChannel config -background yellow
$dispFrame.mask$CurrentChannel config -background yellow

# disc mask modes
frame $discpage.bot.mask.general
frame $discpage.bot.mask.general.mode
frame $discpage.bot.mask.general.mode.radio
set modeframe $discpage.bot.mask.general.mode
label $discpage.bot.mask.l -font $font -wraplength 5i -justify left -text "Discriminator Mask"
foreach disc {"All     " "None    " "Selected" "Odds    " "Evens   " "Mask    "} {
    set lower [string tolower $disc]
    radiobutton $modeframe.radio.$lower -text $disc -variable disc -indicatoron false \
	-relief flat -value $lower -width 8 -borderwidth 1 -selectcolor red -justify center
    pack $modeframe.radio.$lower -side top -pady 0 -anchor w

    bind $modeframe.radio.$lower <ButtonRelease-1> \
	{ChangeDiscMode $CurrentBoard $CurrentChip $disc}
}

#set disc "all     "
set nonelower "none    "
set nonebut $modeframe.radio.$nonelower

button $modeframe.setbut -text "Use as mask" -font $midfont -command UseAsMask
pack $modeframe.radio $discpage.bot.mask.general.mode.setbut -side left -padx 15

#button frame
frame $discpage.bot.mask.general.button
frame $discpage.bot.mask.general.button.mb
frame $discpage.bot.mask.general.button.glob
pack $discpage.bot.mask.general.button.mb $discpage.bot.mask.general.button.glob -side left \
    -fill x -padx 5
button $discpage.bot.mask.general.button.glob.off -text "System Disc Off" \
    -command {ManyDiscChange "none    " 1 }
button $discpage.bot.mask.general.button.mb.off -text "This MB Disc Off" \
    -command {ManyDiscChange "none    " 0 }
button $discpage.bot.mask.general.button.glob.mask -text "System Disc Mask" \
    -command {ManyDiscChange "mask    " 1 }
button $discpage.bot.mask.general.button.mb.mask -text "This MB Disc Mask" \
    -command {ManyDiscChange "mask    " 0 }
button $discpage.bot.mask.general.button.glob.on -text "System Disc On" \
    -command {ManyDiscChange "all     " 1 }
button $discpage.bot.mask.general.button.mb.on -text "This MB Disc On" \
    -command {ManyDiscChange "all     " 0 }
pack $discpage.bot.mask.general.button.glob.on $discpage.bot.mask.general.button.glob.mask \
    $discpage.bot.mask.general.button.glob.off -side top -fill x -pady 5
pack $discpage.bot.mask.general.button.mb.on $discpage.bot.mask.general.button.mb.mask \
    $discpage.bot.mask.general.button.mb.off -side top -fill x -pady 5
pack $discpage.bot.mask.general.mode $discpage.bot.mask.general.button -side top -pady 0 \
    -padx 8 -anchor n
pack $discpage.bot.mask.l $discpage.bot.mask.general -side top -expand no -pady 0 -padx .5


proc UseAsMask {} {

    global CurrentBoard CurrentChip

    set newmask 0
    for {set chan 0} {$chan < 32} {incr chan} {
	global mask$chan
	set chansel [set mask$chan]
	set addthis [expr {$chansel << $chan}]
	set newmask [expr $newmask + $addthis]	
    }

    SetChipwideDiscMask $CurrentBoard $CurrentChip $newmask
    ChangeDiscMode $CurrentBoard $CurrentChip "mask    "
    UpdateThresholdDisp
}

proc ManyDiscChange {changemode changeall} {

    global CurrentBoard CurrentChip nonebut silpage modeframe

    SetManyDiscMode $CurrentBoard "$changemode\00" $changeall

    #update GUI
    $modeframe.radio.$changemode invoke
    ChangeDiscMode $CurrentBoard $CurrentChip $changemode
}


proc UpdateText { } {
    
    global setRadio CurrentChannel CurrentChip CurrentBoard polarity

    $setRadio.chan config -text "Channel $CurrentChannel only"
    $setRadio.chip config -text "All Channels on Chip $CurrentChip"
    $setRadio.mb config -text "All $polarity chips on MB $CurrentBoard" 
    $setRadio.all config -text "All $polarity Boards"
    
}

#disables/enables discrimator mask buttons based on disc mode
#disable/enable "use as mask" button based on disc mode
#highlights current channel in threshold display
#set displayed threshold as current threshold

proc UpdateThresholdDisp {} {

    global CurrentBoard
    global CurrentChip
    global CurrentChannel
    global thrDispLab
    global dispFrame silpage modeframe
    global dispbg
    global Threshold
    global disc
    global test1
    

    set thrDispLab "Thresholds and Masks for Chip $CurrentChip"

    #figure out whether mask buttons should be disabled
    set maskState disabled
    if { $disc == "Mask    " || $disc == "mask    "} { set maskState normal }

    #set all channels to their correct threshold and mask state, 
    #and normal background
    for {set chan 0} {$chan < 32} {incr chan} {
	global mask$chan
	set currThres [GetThreshold $CurrentBoard $CurrentChip $chan]
	$dispFrame.thres$chan config -text "  $currThres"
	$dispFrame.thres$chan config -background $dispbg
	$dispFrame.lab$chan config -background $dispbg
	$dispFrame.mask$chan config -background $dispbg
	$dispFrame.mask$chan configure -state $maskState
	if {$test1 == "exclusive"} { 
	    set mask$chan "0"
	}
	
    }
    #highlight current channel
    $dispFrame.lab$CurrentChannel config -background yellow
    $dispFrame.thres$CurrentChannel config -background yellow
    $dispFrame.mask$CurrentChannel config -background yellow

    set Threshold [GetThreshold $CurrentBoard $CurrentChip $CurrentChannel]

    #if showing CSA/Shaper signals exclusively, disable the ability to change 
    #discmode from "selected", otherwise enable it
    
    set maskState normal
    if {$test1 == "exclusive"} {
	global mask$CurrentChannel
	set mask$CurrentChannel "1"
 	set maskState disabled
    }
    foreach lower {"all     " "none    " "selected" "odds    " "evens   " "mask    "} {
 	$modeframe.radio.$lower configure -state $maskState
    }

    #disable "use as mask" button if mode is already "mask", else enable

    set maskState disabled
    if { $disc != "mask    " } {set maskState normal}
    $modeframe.setbut config -state $maskState

    UpdateText
    
}

UpdateThresholdDisp
