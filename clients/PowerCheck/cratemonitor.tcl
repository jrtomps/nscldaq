#!/bin/sh
# Start Wish. \
exec wish ${0} ${@}



set script    [info script]
set instdir   [file dirname $script]

set cratechecker [file join $instdir powerdetect]
set cratenumber  [lindex $argv 0]
if {![string is integer $cratenumber] | ("$cratenumber" == "") } {
   puts stderr "Usage:"
   puts stderr "   cratemonitor.tcl crate-number"
   exit
}

label .l1 -text "Monitoring crate $cratenumber"
button .e -text Exit -command exit
pack .l1 .e

proc AnalyzeStatus msg {
    set error ""
    global cratenumber
    if {![string is integer $msg]} {
	set do [tk_dialog .status "Unexpected problem" $msg error 0 Retry Abort]
	if {$do == 1} {
	    exit
	}
    } else {
	switch -exact --  $msg {
	    0 {}
            1 {set error "Power off in crate $cratenumber"}
            4 {set error "Could not get crate status for crate $cratenumber"}
	    default {}
	 
	}
	if {$error != ""} {
	    tk_dialog .status "Problem in VME $cratenumber" \
		$error error 0 Dismiss

	}
    }
}

proc Check ms {
    global cratechecker
    global cratenumber

    catch {exec $cratechecker $cratenumber} msg

    AnalyzeStatus $msg

    after $ms "Check $ms"
}

Check 500;				# Check status every 0.5 seconds.
