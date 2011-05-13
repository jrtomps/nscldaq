#
#   This is the control panel application for the
#  CAEN V6533 power supply.
# Usage:
#    panel name
# Where
#    name - is the name of the device as known to 
#           Readout.
#

package require Tk

set here [file dirname [info script]]
source [file join $here channel.tcl]
source [file join $here v6533.tcl]
#
#  Print the program usage:
#

proc usage {} {
    puts stderr "Usage:"
    puts stderr "   panel name"
    puts stderr "Where:"
    puts stderr "   name - the name of a V6533 device in controlconfig.tcl"

}

# 
#  Produce a row label. This serves to 
#  describe the contents of the channel widgets:
#
# Parameters:
#   wid  - widget name of containing frame.
# Returns:
#   wid
#
proc rowLabel wid {
    frame $wid -relief groove -borderwidth 2
    label $wid.name -text name
    label $wid.set  -text setpoint
    label $wid.v    -text voltage
    label $wid.i    -text current

    grid $wid.name
    grid $wid.set
    grid $wid.v
    grid $wid.i

    return $wid
}

#
#  Update the channel state periodically:
#  - Request the monitored data
#  - Update each widget from that data.
#  - Schedule the next run
#
# Parameters:
#   device   - Device object (v5633 snit type.).
#   widget   - The base widget name for the channels.
#              To get each channel widget the channel
#              number must be appended.
#   resched  - Number of seconds between reschedules.
#
proc updateChannels {device widget resched} {

    # Get most recent monitored data
    #  - Report errors.
    #  - break apart the lists into stuff we need.
    #

    set data [$device monitor]
    if {[lindex $data 0] ne "OK"} {
	puts stderr "monitor failed: $data"
    } else {
	set channelStatuses [lindex $data 2]
	set channelVoltages [lindex $data 3]
	set channelCurrents [lindex $data 4]
	# Update each chaennel widget:

	for {set i 0} {$i < 5} {incr i} {
	    set cw $widget$i
	    $cw configure -actualv [lindex $channelVoltages $i]
	    $cw configure -actuali [lindex $channelCurrents $i]
	    
	    #Status as color
	    # - Off green  Button label: On state normal
	    # - On  amber  Button label: Off state normal
	    # - Problem Red Button label On state disabled.


	    set stat [lindex $channelStatuses $i]
	    if {[expr $stat & 1] != 0} {
		$cw configure -bg gold -blabel Off \
		    -buttonstate normal
	    } elseif {[expr $stat & 0x1ff8] != 0} {

		$cw configure -bg red -blabel On \
		    -buttonstate disabled

	    } else {
		$cw configure -bg green -blabel On \
		    -buttonstate normal
	    }
    	}
    }

    after [expr {$resched * 1000}] \
	"updateChannels $device $widget $resched"
}
#
#  Action proc that is called when a channel button
#  is pushed action is as follows:
#  If the label is "On", the channel is turned on.
#  If the label is "Off" the channel is turned off.
#  Actual widget appearance changes are handled when
#  the periodic update notices the change in status.
# Parameters:
#   widget  - The channel widget.
#   chan    - The channel number affected.
# Implicit inputs:
#   device - global variable that contains the
#            object that is a proxy to the server's
#            driver.
#
proc onButton {widget chan} {
    global device

    set state [$widget cget -blabel]
    if {$state == "On"} {
	$device on $chan
    }
    else {
	$device off $chan
    }
}
#------------------------------------------------

#  If the parameter count is not right, error exit

if {[llength $argv] != 1} {
    usage
    exit -1
}
set name $argv

#-------------------------------------------------

# Gui layout.


label .l -text "V6533 control for $argv"
grid .l -columnspan 3

#  Layout the main part of the screen:
#  there are channel widgets for each of the
#  6 channels of the module.
#

for {set i 0} {$i < 6} {incr i} {
    v6533Channel .c$i -label "Ch $i" -blabel On -bg green \
	-setpoint 0 -actualv 0 -actuali 0 \
	-command [list onButton %W $i]

}
#  Each row has three widgets:

set row 1
set col 1
grid [rowLabel .r$row] -row $row -column 0 -sticky n
for {set i 0} {$i < 6} {incr i} {
    grid .c$i -row $row  -column $col
    incr col
    if {$col > 3} {
	set col 1
	incr row
	grid [rowLabel .r$row] -row $row -column 0 -sticky n

    }
}
#------------------------------------------------

# Setup communication.  Connect to 
# localhost unless $env(DAQHOST) is defined and
# port 27000 unless $env(CONTROLPORT) is defined.
#

set host localhost
set port 27000

if {[array names env DAQHOST] ne ""} {
    set host $env(DAQHOST)
}
if {[array names env CONTROLPORT] ne ""} {
    set port $env(CONTROLPORT)
}

#  Make the socket connection with the control server
#  If the connection fails, readout is not running
#  as specified:

set failed [catch {set sock [socket $host $port]}]
if {$failed} {
    error "Failed to connect to control server@$host:$port"
}
fconfigure $sock -buffering line

# Create an object to communicate with the server
# on behalf of this device:

set device [v6533 %AUTO% -socket $sock  -name $name]

#
#  Starts the update process.  The update process
#  periodically requests the monitored data from the
#  the device and updates the GUI using that data:
#

updateChannels $device .c 2;	
