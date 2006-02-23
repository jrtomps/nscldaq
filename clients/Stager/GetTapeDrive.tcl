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


#
#  GetTapeDrive
#    This is a GUI which prompts for a host,
#    displays the set of tapedrives on that host and gets you to select
#    One of them.
#    CallBack mechanisms are used to indicate completion of this process
#    so that the gui need not be modal.

package provide GetTape 1.0


package require RemoteInfo 
namespace eval  GetTape {
    variable TapeDensity
    variable DefaultFtp "daq1ide.nscl.msu.edu"
#
#  Called when the Ok button is pressed on the listbox hierarchy.
#  If no item is selected, this is a no-op.
#  Otherwise, the entry is gotten, the listbox hierarchy is torn down and
#  the user callback is called.
#
    proc GetTapeOk {widget ftphost tapehost password Callback} {
	variable TapeDensity
	set sellist [$widget.listframe.drivelist curselection]
	if {[llength $sellist] > 0} {
	    set drive [$widget.listframe.drivelist get [lindex $sellist 0]]
	    set oldbind [bind $widget <Destroy>]
	    bind $widget <Destroy> {}
	    destroy $widget.listframe
	    destroy $widget.ok
	    destroy $widget.back
	    bind $widget <Destroy> $oldbind
	    $Callback $tapehost $ftphost $password $drive $TapeDensity
	}
    }
#
#  Called when the list box's back button is pressed.
#  The listbox hierarchy is destroyed and GetTapeDrive is called again
#  to re-fetch a hostname.
#
    proc GetTapeBack {widget Callback} {
	set oldbind [bind $widget <Destroy>]
	bind $widget <Destroy> {}
	destroy $widget.listframe
	destroy $widget.ok
	destroy $widget.back
	destroy $widget.capframe

	bind $widget <Destroy> $oldbind
	GetTape::GetTapeDrive $widget $Callback
    }
#
#  This function is called by the GUI when the host name has been specified.
#  Action is to fetch the hostname, destroy the widgets and
#  Attempt to get a list of tapedrives.
#  The list is put in a listbox with an OK, and a Back button.
#  If Ok is pressed, the tapedrive/host are returned, after the Gui is torn
#  down.
#  If Back is pressed, the GUI is torn down and GetTapeDrive is called
#  again to accept a different host.
#
    proc HostGotten   {widget Callback} {
	variable TapeDensity

	set DensityList {2.0 2.6 6.0 10.0 15.0 20.0 35.0 70.0 100.0}
	set TapeDensity 35.0

	set tapehostname [$widget.textframe.entry get]
	set ftphostname  [$widget.ftpframe.entry get]

	set password [$widget.pwframe.entry get]

	set oldbind [bind $widget <Destroy>]
	bind $widget <Destroy> {}

	destroy $widget.textframe
	destroy $widget.pwframe
	destroy $widget.button
	bind $widget <Destroy> $oldbind
	set drivelist ""
	
	catch {set drivelist [RemoteInfo::GetTapeDrives $tapehostname]} output
	frame $widget.listframe
	listbox $widget.listframe.drivelist -relief raised -borderwidth 2 \
		-yscrollcommand "$widget.listframe.scroller set" \
		-selectmode single
	scrollbar $widget.listframe.scroller \
		-command "$widget.listframe.drivelist yview"
	foreach drive $drivelist {
	    $widget.listframe.drivelist insert end $drive
	}
	frame $widget.capframe
	set DensityIndex 0
	foreach Density $DensityList {
	    radiobutton $widget.capframe.r$DensityIndex \
		    -text $Density-Gb -value $Density \
		    -variable GetTape::TapeDensity
	    incr DensityIndex
	}
	
	pack $widget.listframe
	pack $widget.listframe.drivelist -side left
	pack $widget.listframe.scroller -side right -fill y
	pack $widget.capframe -side top
	set DensityIndex 0
	foreach Density $DensityList {
	    pack $widget.capframe.r$DensityIndex -side top
	    incr DensityIndex
	}

	button $widget.ok   -text Ok \
	    -command "GetTape::GetTapeOk $widget $ftphostname $tapehostname $password $Callback"
	button $widget.back -text Back \
		-command "GetTape::GetTapeBack $widget $Callback"
	pack $widget.ok $widget.back
	
    }


#
#   Call this to instantiate this gui in the widget... could be a toplevel
#   or a frame in a toplevel etc. 
#   When the tapedrive is selected, the created widgets will be destroyed
#   and Callback will be called:
#        $CallBack  hostname drivespecialfilename gbytes
#

    proc GetTapeDrive {widget CallBack} {
	variable DefaultFtp

	frame $widget.ftpframe
	frame $widget.textframe
	frame $widget.pwframe

	label $widget.ftpframe.label -text "Ftp host: "
	entry $widget.ftpframe.entry 
	$widget.ftpframe.entry insert end $DefaultFtp

	label $widget.textframe.label -text "Host with tapedrive: "
	entry $widget.textframe.entry
	
	set user [exec whoami]
	label $widget.pwframe.label -text "Password for $user"
	entry $widget.pwframe.entry -show "*"

	button $widget.button -text "Ok" \
		-command "GetTape::HostGotten $widget $CallBack"

	#  Set up the widget layouts:
	
	pack $widget.ftpframe
	pack $widget.ftpframe.label $widget.ftpframe.entry -side left
	pack $widget.textframe
	pack $widget.textframe.label $widget.textframe.entry -side left
	pack $widget.pwframe
	pack $widget.pwframe.label $widget.pwframe.entry -side left
	pack $widget.button
    }
    namespace export GetTapeDrive
}

