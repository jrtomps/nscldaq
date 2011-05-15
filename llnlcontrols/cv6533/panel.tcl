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
source [file join $here channelParams.tcl]
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
#  Loads the setpoints from the devices
#  into the channel widgets.  This allows
#  the panel to be dropped and then picked up
#  again with no loss of information.
# Parameters:
#   device - Proxy for the server device driver.
#   wid    - base widget name. The actual channel i
#            widget name is $wid$i
proc loadSetpoints {device wid} {
    set setpoints [$device getSetpoints]

    for {set i 0} {$i < 6} {incr i} {
	set setpt [lindex $setpoints $i]
	set widget $wid$i
	$widget configure -setpoint $setpt
    }
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

	for {set i 0} {$i < 6} {incr i} {
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
# Given the output of one of the simple get
# commands from v6533 return an appropriate output list
# The list will be either the values from the device
# or, if there was an error, the reason is output
# to stderr and the list is filled with 'undef'
# Parameters:
#   result  - The result of the get operation
# Returns:
#   a 6 element list.
proc getListValue {result} {
    if {[lindex $result 0] ne "OK"} {
	puts stderr "Get request failed: $result"
	for {set i 0} {$i < 6} {incr i} {
	    lappend retval "undef"
	}
	return  $retval
    } else {
	return [lrange  $result 1 end]
    }
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
      set result [$device on $chan]
    } else {
	set result [$device off $chan]
    }
}
#
#  Action function called when the setpoint changes on a
#  channel.  The new value will be taken care of by the
#  widget. What is left for us to do is to set the
#  new value in the device.
# Parameters:
#   wid  - Channel control widget.
#   val  - New setpoint value.
#   chan - Channel number
# Implicit inputs:
#   device - proxy object for the server's device driver
#            for us.
proc onNewSetpoint {wid val chan} {
    global device
    $device setpoint $chan $val
}
#
#  Action function called when a properties request
#  is made.  A properties non-modal dialog is raised
#  for that channel
# Parameters:
#    widget   - The channel widget for which the dialog
#               is requested.
#    channel  - The channel for which the widget is
#               requested
#
proc onProperties {widget channel} {
    global Ilimit
    global Ttime
    global RupRate
    global RdnRate
    global PoffMode

    toplevel .properties
    channelParams .properties.controls \
	-ilimit     [lindex $Ilimit $channel] \
	-triptime   [lindex $Ttime  $channel] \
	-rampup     [lindex $RupRate $channel] \
	-rampdown   [lindex $RdnRate $channel] \
	-offmode    [lindex $PoffMode $channel]

    pack .properties.controls
}
#
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
	-command [list onButton %W $i]   \
	-setchanged [list onNewSetpoint %W %V $i] \
	-properties [list onProperties %W $i]

}
#  Each row has three widgets:

set row 1
set col 1
grid [rowLabel .r$row] -row $row -column 0 -sticky n
for {set i 0} {$i < 6} {incr i} {
    grid .c$i -row $row  -column $col
    incr col
    if {($col > 3) && ($i < 5)} {
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

#  Get the setpoints from the device and load them into
#  the widgets.

loadSetpoints  $device .c

#
#  Starts the update process.  The update process
#  periodically requests the monitored data from the
#  the device and updates the GUI using that data:
#


updateChannels $device .c 2;	

#
#  Retrieve the channel parameters that can be 
#  adjusted by the pop up...we get them for all
#  channels (lists indexed by channel number
#  so that we don't need to interact with the server
#  to populate the popup.
#  Globals are:
#   Ilimit  - channel current limits.
#   Ttime   - channel tript times.
#   RupRate - Channel ramp up rates.
#   RdnRate - Channel Ramp down rates.
#   PoffMode - Channel power off modes.

set Ilimit   [getListValue [$device getIlimit]]
set Ttime    [getListValue [$device getTripTimes]]
set RupRate  [getListValue [$device getRupRate]]
set RdnRate  [getListValue [$device getRdnRate]]
set PoffMode [getListValue [$device getOffMode]]
