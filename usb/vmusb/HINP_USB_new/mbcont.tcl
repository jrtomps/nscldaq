#!/bin/wish
tk appname Motherboard1;wm title . "Motherboard Control";set widgetDemo 1;set\
font -*-Helvetica-Medium-R-Normal--*-120-*-*-*-*-*-*;set smallfont\
-*-Helvetica-Medium-R-Normal--*-80-*-*-*-*-*-*;set mbpage [tabnotebook_page\
.main "Motherboard Control"];option add *Menubutton.selectColor: red;set font\
{Helvetica 14};global steering1 steering2 steering3 steering4 steering5\
steering6 steering7 steering8;global steering9 steering10 steering11\
steering12 steering13 steering14 steering15 steering16;global SumOffA SumOffB\
SumOffC ShapOff CSAOff;global SISDelay AcqDelay PauseDelay CycleTimeout\
GlobalTimeout TriggerDelay;global CoinWindow;global CurrentBoard;global\
TotBoards;global CurrentChip;global CurrentChannel;global ForceRead;for {set i\
0} {$i < 16} {incr i} {set steering$i "o"};frame $mbpage.top;frame\
$mbpage.mid;frame $mbpage.bot;pack $mbpage.top $mbpage.mid $mbpage.bot -side\
top -expand yes -pady 0 -anchor w;frame $mbpage.mid.serdac;frame\
$mbpage.mid.times;frame $mbpage.mid.cb;pack $mbpage.mid.serdac\
$mbpage.mid.times $mbpage.mid.cb -side left -expand yes -pady .5 -padx\
.5;label $mbpage.mid.serdac.l -font $font -wraplength 5i -justify left -text\
"DAC Settings";scale $mbpage.mid.cb.scale -orient horizontal -length 125 -from\
1 -to $TotBoards -tickinterval 1 -variable CurrentBoard -label "Current\
Board";pack $mbpage.mid.cb.scale -side top -pady 2 -anchor w;bind\
$mbpage.mid.cb.scale <ButtonRelease-1> {ChangeChip $CurrentBoard $CurrentChip\
$CurrentChannel};checkbutton $mbpage.mid.cb.setForce -text "Forced Readout" \
-variable ForceRead -relief flat -selectcolor red;pack $mbpage.mid.cb.setForce\
-side bottom -pady 1 -anchor w;bind $mbpage.mid.cb.setForce <ButtonRelease-1>\
{SetForceRead $CurrentBoard $ForceRead};frame\
$mbpage.mid.serdac.wSumOffA;frame $mbpage.mid.serdac.wSumOffB;frame\
$mbpage.mid.serdac.wSumOffC;frame $mbpage.mid.serdac.wShapOff;frame\
$mbpage.mid.serdac.wCSAOff;pack $mbpage.mid.serdac.wSumOffA\
$mbpage.mid.serdac.wSumOffB $mbpage.mid.serdac.wSumOffC\
$mbpage.mid.serdac.wShapOff $mbpage.mid.serdac.wCSAOff -side top -expand no\
-pady 0 -padx 0;label $mbpage.mid.serdac.wSumOffA.l -font $font -wraplength 5i\
-justify left -text "Sum Offset A";label $mbpage.mid.serdac.wSumOffB.l -font\
$font -wraplength 5i -justify left -text "Sum Offset B";label\
$mbpage.mid.serdac.wSumOffC.l -font $font -wraplength 5i -justify left -text\
"Sum Offset C";label $mbpage.mid.serdac.wShapOff.l -font $font -wraplength 5i\
-justify left -text "Shaper Offset";label $mbpage.mid.serdac.wCSAOff.l -font\
$font -wraplength 5i -justify left -text "CSA Offset";scale\
$mbpage.mid.serdac.wSumOffA.scale -orient horizontal -length 150 -from 0 -to\
1023 -tickinterval 256 -variable SumOffA -borderwidth 2 -font smallfont;pack\
$mbpage.mid.serdac.wSumOffA.l $mbpage.mid.serdac.wSumOffA.scale -side left\
-pady 0 -anchor w -expand no;bind $mbpage.mid.serdac.wSumOffA.scale\
<ButtonRelease-1> {SetSumOffA $CurrentBoard $SumOffA};scale\
$mbpage.mid.serdac.wSumOffB.scale -orient horizontal -length 150 -from 0 -to\
1023 -tickinterval 256 -variable SumOffB -borderwidth 2 -font smallfont;pack\
$mbpage.mid.serdac.wSumOffB.l $mbpage.mid.serdac.wSumOffB.scale -side left\
-pady 0 -anchor w -expand no;bind $mbpage.mid.serdac.wSumOffB.scale\
<ButtonRelease-1> {SetSumOffB $CurrentBoard $SumOffB};scale\
$mbpage.mid.serdac.wSumOffC.scale -orient horizontal -length 150 -from 0 -to\
1023 -tickinterval 256 -variable SumOffC -borderwidth 2 -font smallfont;pack\
$mbpage.mid.serdac.wSumOffC.l $mbpage.mid.serdac.wSumOffC.scale -side left\
-pady 0 -anchor w -expand no;bind $mbpage.mid.serdac.wSumOffC.scale\
<ButtonRelease-1> {SetSumOffC $CurrentBoard $SumOffC};scale\
$mbpage.mid.serdac.wShapOff.scale -orient horizontal -length 150 -from 0 -to\
1023 -tickinterval 256 -variable ShapOff -borderwidth 2 -font smallfont;pack\
$mbpage.mid.serdac.wShapOff.l $mbpage.mid.serdac.wShapOff.scale -side left\
-pady 0 -anchor w;bind $mbpage.mid.serdac.wShapOff.scale <ButtonRelease-1>\
{SetShapOff $CurrentBoard $ShapOff};scale $mbpage.mid.serdac.wCSAOff.scale\
-orient horizontal -length 150 -from 0 -to 1023 -tickinterval 256 -variable\
CSAOff -font smallfont;pack $mbpage.mid.serdac.wCSAOff.l\
$mbpage.mid.serdac.wCSAOff.scale -side left -pady 0 -anchor w;bind\
$mbpage.mid.serdac.wCSAOff.scale <ButtonRelease-1> {SetCSAOff $CurrentBoard\
$CSAOff};frame $mbpage.mid.times.sisDelay;frame\
$mbpage.mid.times.acqDelay;frame $mbpage.mid.times.pauseDelay;frame\
$mbpage.mid.times.cycleTimeout;frame $mbpage.mid.times.globalTimeout;frame\
$mbpage.mid.times.trigDelay;frame $mbpage.mid.times.coinWind;pack\
$mbpage.mid.times.sisDelay $mbpage.mid.times.acqDelay\
$mbpage.mid.times.pauseDelay $mbpage.mid.times.cycleTimeout\
$mbpage.mid.times.globalTimeout $mbpage.mid.times.trigDelay\
$mbpage.mid.times.coinWind -side top -expand no -pady 0 -padx 0;label\
$mbpage.mid.times.sisDelay.l -font $font -wraplength 5i -justify left -text\
"SIS Clock Delay (ns)";label $mbpage.mid.times.acqDelay.l -font $font\
-wraplength 5i -justify left -text "Acq Delay (ns)";label\
$mbpage.mid.times.pauseDelay.l -font $font -wraplength 5i -justify left -text\
"Pause Delay (ns)";label $mbpage.mid.times.cycleTimeout.l -font $font\
-wraplength 5i -justify left -text "Cycle Timeout (ns)";label\
$mbpage.mid.times.globalTimeout.l -font $font -wraplength 5i -justify left\
-text "Global Timeout (ns)";label $mbpage.mid.times.trigDelay.l -font $font\
-wraplength 5i -justify left -text "Trigger Delay (ns)";label\
$mbpage.mid.times.coinWind.l -font $font -wraplength 5i -justify left -text\
"Coincidence Window (ns)";scale $mbpage.mid.times.sisDelay.scale -orient\
horizontal -length 300 -from 0 -to 25500 -tickinterval 6000 -variable SISDelay\
-borderwidth 2 -resolution 40 -font smallfont;pack\
$mbpage.mid.times.sisDelay.l $mbpage.mid.times.sisDelay.scale -side left -pady\
0 -anchor w -expand no;bind $mbpage.mid.times.sisDelay.scale <ButtonRelease-1>\
{SetSISDelay $CurrentBoard $SISDelay};scale $mbpage.mid.times.acqDelay.scale\
-orient horizontal -length 300 -from 0 -to 25500 -tickinterval 6000 -variable\
AcqDelay -borderwidth 2 -resolution 40 -font smallfont;pack\
$mbpage.mid.times.acqDelay.l $mbpage.mid.times.acqDelay.scale -side left -pady\
0 -anchor w -expand no;bind $mbpage.mid.times.acqDelay.scale <ButtonRelease-1>\
{SetAcqDelay $CurrentBoard $AcqDelay};scale $mbpage.mid.times.pauseDelay.scale\
-orient horizontal -length 300 -from 0 -to 25500 -tickinterval 6000 -variable\
PauseDelay -borderwidth 2 -resolution 40 -font smallfont;pack\
$mbpage.mid.times.pauseDelay.l $mbpage.mid.times.pauseDelay.scale -side left\
-pady 0 -anchor w -expand no;bind $mbpage.mid.times.pauseDelay.scale\
<ButtonRelease-1> {SetPauseDelay $CurrentBoard $PauseDelay};scale\
$mbpage.mid.times.cycleTimeout.scale -orient horizontal -length 300 -from 0\
-to 25500 -tickinterval 6000 -variable CycleTimeout -borderwidth 2 -resolution\
40 -font smallfont;pack $mbpage.mid.times.cycleTimeout.l\
$mbpage.mid.times.cycleTimeout.scale -side left -pady 0 -anchor w -expand\
no;bind $mbpage.mid.times.cycleTimeout.scale <ButtonRelease-1>\
{SetCycleTimeout $CurrentBoard $CycleTimeout};scale\
$mbpage.mid.times.globalTimeout.scale -orient horizontal -length 300 -from 0\
-to 25500 -tickinterval 6000 -variable GlobalTimeout -borderwidth 2\
-resolution 40 -font smallfont;pack $mbpage.mid.times.globalTimeout.l\
$mbpage.mid.times.globalTimeout.scale -side left -pady 0 -anchor w -expand\
no;bind $mbpage.mid.times.globalTimeout.scale <ButtonRelease-1>\
{SetGlobalTimeout $CurrentBoard $GlobalTimeout};scale\
$mbpage.mid.times.trigDelay.scale -orient horizontal -length 300 -from 0 -to\
3100 -tickinterval 700 -variable TriggerDelay -borderwidth 2 -resolution 12\
-font smallfont;pack $mbpage.mid.times.trigDelay.l\
$mbpage.mid.times.trigDelay.scale -side left -pady 0 -anchor w -expand no;bind\
$mbpage.mid.times.trigDelay.scale <ButtonRelease-1> {SetTriggerDelay\
$CurrentBoard $TriggerDelay};scale $mbpage.mid.times.coinWind.scale -orient\
horizontal -length 300 -from 0 -to 3100 -tickinterval 700 -variable CoinWindow\
-borderwidth 2 -resolution 12 -font smallfont;pack\
$mbpage.mid.times.coinWind.l $mbpage.mid.times.trigDelay.scale -side left\
-pady 0 -anchor w -expand no;bind $mbpage.mid.times.coinWind.scale\
<ButtonRelease-1> {SetTriggerDelay $CurrentBoard $CoinWindow};frame\
$mbpage.bot.slot;pack $mbpage.bot.slot -side right -expand yes -pady .5 -padx\
.5;frame $mbpage.bot.slot.number;label $mbpage.bot.slot.l1 -font $font\
-wraplength 5i -justify left -text "Board Slot            Status     OR/Mult.\
Steering          ";label $mbpage.bot.slot.l2 -font $font -wraplength 5i\
-justify left -text "                                         Off  A    B    C\
    ";pack $mbpage.bot.slot.l1 $mbpage.bot.slot.l2 -side top -expand no -pady\
0 -padx .5;for {set i 1} {$i <= 16} {incr i} {frame\
$mbpage.bot.slot.number.$i;foreach steer {"C" "B" "A" "O"} {set lower [string\
tolower $steer];radiobutton $mbpage.bot.slot.number.$i.$lower -variable\
steering$i -relief flat -borderwidth 1 -width 0 -value $lower -command\
"UpdateRouting $i $steer" -selectcolor red;pack\
$mbpage.bot.slot.number.$i.$lower -side right -expand no -fill none};label\
$mbpage.bot.slot.number.$i.l -font $font -wraplength 1i -justify left -padx 50\
-text "$i";label $mbpage.bot.slot.number.$i.status -font $font -wraplength 1i\
-justify left -padx 50 -text [GetSlotStatus $CurrentBoard $i];puts stdout\
[format "GetSlotStatus for slot %d %s" $i [GetSlotStatus $CurrentBoard\
$i]];pack $mbpage.bot.slot.number.$i.l $mbpage.bot.slot.number.$i.status -side\
left -expand no -fill none};for {set i 1} {$i <= 16} {incr i} {pack\
$mbpage.bot.slot.number.$i -side top -expand no -fill none};pack\
$mbpage.bot.slot.number -side top -expand no -pady 0 -padx .5;proc\
UpdateRouting {slot steerage} {global CurrentBoard;SetSlotRouting\
$CurrentBoard $slot $steerage};proc UpdateMotherboard {Motherboard} {global\
SumOffA SumOffB SumOffC ShapOff CSAOff;global steering1 steering2 steering3\
steering4 steering5 steering6 steering7 steering8;global steering9 steering10\
steering11 steering12 steering13 steering14 steering15 steering16;global\
SISDelay AcqDelay PauseDelay CycleTimeout GlobalTimeout TriggerDelay\
CoinWindow;global CurrentBoard;global ForceRead;set SumOffA [GetSumOffA\
$CurrentBoard];set SumOffB [GetSumOffB $CurrentBoard];set SumOffC [GetSumOffC\
$CurrentBoard];set ShapOff [GetShapOff $CurrentBoard];set CSAOff [GetCSAOff\
$CurrentBoard];for {set i 1} {$i <= 16} {incr i} {set routing [GetSlotRouting\
$CurrentBoard $i];set steering$i [string tolower $routing]};set SISDelay\
[GetSISDelay $CurrentBoard];set AcqDelay [GetAcqDelay $CurrentBoard];set\
PauseDelay [GetPauseDelay $CurrentBoard];set CycleTimeout [GetCycleTimeout\
$CurrentBoard];set GlobalTimeout [GetGlobalTimeout $CurrentBoard];set\
TriggerDelay [GetTriggerDelay $CurrentBoard];set ForceRead [GetForceRead\
$CurrentBoard]};proc positionWindow w {wm geometry $w +100+100};proc showVars\
{w args} {catch {destroy $w};toplevel $w;wm title $w "Variable values";label\
$w.title -text "Variable values:" -width 20 -anchor center -font {Helvetica\
18};pack $w.title -side top -fill x;set len 1;foreach i $args {if {[string\
length $i] > $len} {set len [string length $i]}};foreach i $args {frame\
$w.$i;label $w.$i.name -text "$i: " -width [expr $len + 2] -anchor w;label\
$w.$i.value -textvar $i -anchor w;pack $w.$i.name -side left;pack $w.$i.value\
-side left -expand 1 -fill x;pack $w.$i -side top -anchor w -fill x};button\
$w.ok -text OK -command "destroy $w" -default active;bind $w <Return>\
"tkButtonInvoke $w.ok";pack $w.ok -side bottom -pady 2};proc invoke index\
{global tk_library;set tags [.t tag names $index];set i [lsearch -glob $tags\
demo-*];if {$i < 0} {return};set cursor [.t cget -cursor];.t configure -cursor\
watch;update;set demo [string range [lindex $tags $i] 5 end];uplevel [list\
source [file join $tk_library demos $demo.tcl]];update;.t configure -cursor\
$cursor;.t tag add visited "$index linestart +1 chars" "$index lineend -1\
chars"};proc showStatus index {global tk_library;set tags [.t tag names\
$index];set i [lsearch -glob $tags demo-*];set cursor [.t cget -cursor];if {$i\
< 0} {.statusBar.lab config -text " ";set newcursor xterm} {set demo [string\
range [lindex $tags $i] 5 end];.statusBar.lab config -text "Run the \"$demo\"\
sample program";set newcursor hand2};if [string compare $cursor $newcursor]\
{.t config -cursor $newcursor}};proc showCode w {global tk_library;set file\
[string range $w 1 end].tcl;if ![winfo exists .code] {toplevel .code;frame\
.code.buttons;pack .code.buttons -side bottom -fill x;button\
.code.buttons.dismiss -text Dismiss -default active -command "destroy\
.code";button .code.buttons.rerun -text "Rerun Demo" -command {eval\
[.code.text get 1.0 end]};pack .code.buttons.dismiss .code.buttons.rerun -side\
left -expand 1 -pady 2;frame .code.frame;pack .code.frame -expand yes -fill\
both -padx 1 -pady 1;text .code.text -height 40 -wrap word -xscrollcommand\
".code.xscroll set" -yscrollcommand ".code.yscroll set" -setgrid 1\
-highlightthickness 0 -pady 2 -padx 3;scrollbar .code.xscroll -command\
".code.text xview" -highlightthickness 0 -orient horizontal;scrollbar\
.code.yscroll -command ".code.text yview" -highlightthickness 0 -orient\
vertical;grid .code.text -in .code.frame -padx 1 -pady 1 -row 0 -column 0\
-rowspan 1 -columnspan 1 -sticky news;grid .code.yscroll -in .code.frame -padx\
1 -pady 1 -row 0 -column 1 -rowspan 1 -columnspan 1 -sticky news;grid\
rowconfig .code.frame 0 -weight 1 -minsize 0;grid columnconfig .code.frame 0\
-weight 1 -minsize 0} {wm deiconify .code;raise .code};wm title .code "Demo\
code: [file join $tk_library demos $file]";wm iconname .code $file;set id\
[open [file join $tk_library demos $file]];.code.text delete 1.0\
end;.code.text insert 1.0 [read $id];.code.text mark set insert 1.0;close\
$id};proc aboutBox {} {tk_messageBox -icon info -type ok -title "About\
SilStrip" -message "SilStrip --  a program to control the Silicon Strip Chip\
System\n\n"};proc FileBox {Direction} {set font\
-*-Helvetica-Medium-R-Normal--*-120-*-*-*-*-*-*;set w .filebox;catch {destroy\
$w};toplevel $w;wm title $w "File Selection";wm iconname $w\
"filebox";positionWindow $w;label $w.msg -font $font -wraplength 4i -justify\
left -text "Enter a file name in the entry box or click on the \"Browse\"\
buttons to select a file name using the file selection dialog.";pack $w.msg\
-side top;frame $w.buttons;pack $w.buttons -side bottom -fill x -pady\
2m;button $w.buttons.cancel -text Cancel -command "destroy $w";button\
$w.buttons.ok -text OK -command "destroy $w";pack $w.buttons.ok\
$w.buttons.cancel -side left -expand 1;set g [frame $w.open];label $g.lab\
-text "Select a file to $Direction: " -anchor e;entry $g.ent -width 20\
-textvariable FileName;button $g.but -text "Browse ..." -command "fileDialog\
$w $g.ent $Direction";pack $g.lab -side left;pack $g.ent -side left -expand\
yes -fill x;pack $g.but -side left;pack $g -fill x -padx 1c -pady 3};proc fileDialog {wind ent operation} {set types {
	{"Setup files"          {.setup .Setup} }
	{"Text files"		{.txt .doc}	}
	{"Text files"		{}		}
	{"All files"		*}
    };if {$operation == "load"} {set file [tk_getOpenFile -filetypes $types\
-parent $wind];puts "filename is $file";LoadFile $file;reload;set CurrentChip\
1;set CurrentChannel 0;ChangeChip $CurrentChip $CurrentChannel} {set file\
[tk_getSaveFile -filetypes $types -parent $wind -initialfile Untitled\
-defaultextension .setup];puts "filename is $file";SaveFile $file};if [string\
compare $file ""] {$ent delete 0 end;$ent insert 0 $file;$ent xview end}};proc\
XLMConfig {} {source loadlv1.tcl};
