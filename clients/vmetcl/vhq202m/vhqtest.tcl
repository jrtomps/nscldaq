#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


# (C) Copyright Michigan State University 1938, All rights reserved 
# source vhq202m.tcl

set here    [file dirname [info script]]
set libDir  [file join $here ..]
set wd [pwd]
cd $libDir
set libDir [pwd]
cd $wd

if {[file search $auto_path $libDir] == -1} {
    set auto_path [concat $libDir $auto_path]
}


set base 0xda00
package require vhq

label .v1 -text 0
label .i1 -text 0

label .v2 -text 0
label .i2 -text 0

label .serial -text ""
pack .serial .v1 .i1 .v2 .i2

label .raval -text 0
label .rbval -text 0
pack .raval .rbval

label .vlima -text 0
label .vlimb -text 0

label .ilimah -text 0
label .ilimas -text 0

label .ilimbh -text 0
label .ilimbs -text 0

pack .vlima .vlimb .ilimah .ilimas .ilimbh .ilimbs

set stat1bits {vz manual plus off kill rampup stable error} 

set stat2bits {ilimit opcomplete fpchanged voverset inhibited overvori badq}
set stat2texts {"Current Limit" "Operation Done" "Switch thrown" \
	"Set voltage higher than limit" "External inhibit" \
	"Over voltage or current" "Voltage quality not given"}

foreach bit $stat1bits {
    checkbutton .a$bit -text a$bit -state disabled
    checkbutton .b$bit -text b$bit -state disabled
    pack .a$bit .b$bit
}

proc PostAlarm {top bits} {
    set dlg $top.alarm
    if {[winfo exists $dlg]} {
	destroy $dlg
    }
    toplevel $dlg

    label $dlg.caption -text "VHQ202M alarm"
    pack  $dlg.caption

    global stat2bits stat2texts
    checkbutton $dlg.tot -text "Timeout error"
    pack $dlg.tot
    set timeout [lindex $bits 0]
    set astat   [lindex $bits 1]
    set bstat   [lindex $bits 2]
    if {[lindex $timeout 1]} {$dlg.tot select } else {$dlg.tot deselect}
    for {set i 0} {$i < 7} {incr i} {
	set wid [lindex $stat2bits $i]
	append wid a
	checkbutton $dlg.$wid -text "A [lindex $stat2texts $i]"
	pack $dlg.$wid
	if {[lindex [lindex $astat $i] 1]} {
	    $dlg.$wid select
	} else {$dlg.$wid deselect}
	set wid [lindex $stat2bits $i]
	append wid b
	checkbutton $dlg.$wid -text "B [lindex $stat2texts $i]"
	pack $dlg.$wid
	if {[lindex [lindex $bstat $i] 1]} {
	    $dlg.$wid select
	} else { $dlg.$wid deselect }
    }

    button $dlg.dismiss -text "Dismiss" -command "destroy $dlg"
    pack $dlg.dismiss
}

proc update {ctl ms top} {

#   Update voltage display:

    set chana [vhq::actual $ctl a]
    set chanb [vhq::actual $ctl b]

    set v1 [lindex $chana 0]
    set v2 [lindex $chanb 0]

    set i1 [lindex $chana 1]
    set i2 [lindex $chanb 1]

    set serial [vhq::id $ctl]

#   serial number.

    $top.serial config -text $serial
    $top.v1 config -text $v1
    $top.i1 config -text $i1
    $top.v2 config -text $v2
    $top.i2 config -text $i2

    $top.vlima config -text [::vhq::limit $ctl  v a]
    $top.vlimb config -text [::vhq::limit $ctl v b]

    set ilima [::vhq::limit $ctl i a]
    set ilimb [::vhq::limit $ctl i b]
    $top.ilimah config -text [lindex $ilima 1]
    $top.ilimas config -text [lindex $ilima 0]
    $top.ilimbh config -text [lindex $ilimb 1]
    $top.ilimbs config -text [lindex $ilimb 0]


# ramp speeds:
    $top.raval config -text [::vhq::rampspeed $ctl a]
    $top.rbval config -text [::vhq::rampspeed $ctl b]

#  Status register 1:

    global stat1bits
    set stat1 [vhq::stat1 $ctl]
    set astat1 [lindex $stat1 0]
    set bstat1 [lindex $stat1 1]
    set widget $top.a
    foreach item $astat1 {
	set widget $top.a
	append widget [lindex $item 0]
	if {[lindex $item 1]} {
	    $widget  select
	} else {
	    $widget  deselect
	}
    }
    foreach item $bstat1 {
	set widget $top.b
	append widget [lindex $item 0]
	if {[lindex $item 1]} {
	    $widget select
	} else {
	    $widget deselect
	}

    }

# Ramp Speeds:

    

# Status register 2 alarms:

    set stat2 [vhq::stat2 $ctl]
    set tot [lindex $stat2 0]
    set astat [lindex $stat2 1]
    set bstat [lindex $stat2 2]
    if {[lindex $tot 1]} {PostAlarm $top $stat2}
    foreach item $astat {
	if {[lindex $item 1]} {PostAlarm $top $stat2}
    }
    foreach item $bstat {
	if {[lindex $item 1]} {PostAlarm $top $stat2}
    }

    after $ms "update $ctl $ms \"$top\""
}



set ctl [vhq::create $base]
entry .rampa -width 3 -relief sunken -bd 2 -textvariable chana
button .setrampa -text "RampSpeed a" -command {
    vhq::rampspeed $ctl a $chana
}
pack .rampa .setrampa
entry .rampb -width 3 -relief sunken -bd 2 -textvariable chanb
button .setrampb -text "RampSpeed b" -command {
    vhq::rampspeed $ctl b $chanb
}
pack .rampb .setrampb

entry .vtarga -width 6 -relief sunken -bd 2 -textvariable chanav
button .settarga -text "Ramp A" -command {
    vhq::setv $ctl a $chanav
}
pack .vtarga .settarga

entry .vtargb -width 6 -relief sunken -bd 2 -textvariable chanbv
button .settargb -text "Ramp B" -command {
    vhq::setv $ctl b $chanbv
}
pack .vtargb .settargb

entry .ilima -width 3 -relief sunken -bd 2 -textvariable ilima
button .setilima -text "I limit a" -command {
    vhq::limit $ctl c a $ilima
}
pack .ilima .setilima


entry .ilimb -width 3 -relief sunken -bd 2 -textvariable ilimb
button .setilimb -text "I limit b" -command {
    vhq::limit $ctl c b $ilimb
}
pack .ilimb .setilimb

update $ctl 1000 ""



