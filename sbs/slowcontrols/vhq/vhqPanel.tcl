#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#
package require Vme
package require vhq
package require Tk
package require snit

package provide vhqPanel 1.0

#
#   This is a snit megawidget that controls a VHQ dual channel high voltage module.
# Appearance:
#          +-------------------------------------------------------+
#          |    Module name  @ Crate n  Base: base                 |
#          +-------------------------------------------------------+
#          |        Current:   nnnnnn
# Options:
#    -name     - The name to use for this module.
#    -crate    - The vme crate in which the vhq module being controlled is installed.
#    -base     - The base address of the vhq module in that crate.
#    -command  - The command to invoke when the module changes in a significatn way that
#                does not have to do with an alarm condition.
#                 The command will have 4 parameters tacked on to it.
#                   who   - This object.
#                   what  - A text string describing what changed.
#                           what can be:
#                               ramp   - Ramp initiated value is the target.
#                               off    - Scram to zero initiated, value is 0.
#                               ilimit - New current limit set value is the current limit in uA
#                               speed  - New ramp speed set, value is the ramp speed in V/s.
#                   which - Where appropriate information about which channel changed.
#                   value - The now value of the thing that changed.
#   -alarmscript - The script to invoke when an alarm condition occurs.  The script will be passed
#                  the following additional parameters.
#                       who  - this object.
#                       what - the alarm condition, which is one of:
#                               manual     - Channel has been flipped in/out of manual.
#                               error      - Channel has raised an error bit.
#                               Voverset   - Channel is over voltage set point.
#                               OverVorI   - Over voltage or overcurrent.
#                               ilimit     - Overcurrent limit.
#                               BadQuality - Bad regulation.
#                       which- The channel that raised the alarm if appropriate.
#                       value- set or unset to indicate which way the transition went.
#                  Note that this megawidget need not be visible or even managed
#                  to monitor for and report alarm conditions.
#   -monitorinterval -
#                  The  monitoring interval in seconds.  This can be changed
#                  at any time.  The monitoring interval determines how often the
#                  software checks for and reports (via the -alarmscript) trips and
#                  other alarm conditions.  This defaults to 5.
#   -model       - Describes the actual model number (e.g. 202M). This is used to determine
#                  The maximum voltage and how to convert current values from device units
#                  to actual mA.   The devices supported are:
#                                          Vmax        Imax     IRes
#                    202L-s                2000 V      0.1 mA   .05  uA
#                    202M (default)        2000 V      3 mA     1.5  uA
#                    202M-h                2000 V      6 mA     2.0  uA
#                    203M                  3000 V      2 mA     0.66 uA
#                    203M-h                3000 V      4 mA     1.33 uA
#                    204L                  4000 V      1 mA     0.33 uA
#                    204M-h                4000 V      3 mA     1.5  uA
#                    205L                  5000 V      1 mA     0.33 uA
#                    205L-h                5000 V      2 mA     0.66 uA
#   -rampspeed {a b}
#                - Sets the ramp speed of the a/b channels.  This can be
#                  dynamically reconfigured.  The ramp speed is in Volts/Sec.
#   -ilimit{a b} - Sets the current limit of the a/b channels in uAmps.
#
#
# TODO:
#    Need to fill in more details about the values of who, what,which for -command and -alarmscript.
#
# Methods:
#    setProperties    - Sets the property list for the module.  Property lists are a device independent
#                       mechanism for storing device settings.
#    getProperties    - Gets the property list that reflects the state of the module.
#
# Properties:
#       targeta       - Target voltage for channel a
#       targetb       - Target voltage for channel b
#       ilimita       - Current limit in uA for channel a
#       ilimitb       - Current limit in uA for channel b
#       rampspeeda    - Ramp speed in V/sec for channel a
#       rampspeedb    - Ramp speed in V/sec for channel b
#       model         - Model of the unit
#

snit::widget vhqPanel {
    option -name            {}
    option -crate           0
    option -base
    option -command         {}
    option -alarmscript     {}
    option -monitorinterval 1
    option -model           {202M}
    option -rampspeed       {10 10}
    option -ilimit          {50 50};

    variable timerid        -1;           # Id of timer for monitoring trips.
    variable updateid       -1;           # Id of timer for updating GUI.

    variable device         {}

    variable maxV           2000;         # Maximum voltage allowed (V)
    variable resI           1.5;          # Current resolution in uA.

    variable lastError
    variable lastIlimit
    variable lastManual
    variable lastVOverSet
    variable lastOverVorI
    variable lastBadQuality
    variable lastOff

    constructor args {
        set lastError(a)          0
        set lastError(b)          0

        set lastIlimit(a)     0
        set lastIlimit(b)     0

        set lastManual(a)     0
        set lastManual(b)     0

        set lastVOverSet(a)   0
        set lastVOverSet(b)   0

        set lastOverVorI(a)   0
        set lastOverVorI(b)   0

        set lastBadQuality(a) 0
        set lastBadQuality(b) 0

        set lastOff(a)        0
        set lastOff(b)        0


        #   Create the GUI.

        set id [frame $win.id]
        label $id.name
        set chanA [frame $win.a -relief groove -borderwidth 3]
        label $chanA.chan     -text {Channel A}
        label $chanA.ilabel   -text {Current}
        label $chanA.ivalue   -text {00000} -width 6
        label $chanA.iunits   -text {uAmps}

        label $chanA.vlabel   -text {Voltage}
        label $chanA.vvalue   -text {0000}  -width 6
        label $chanA.vunits   -text {Volts}

        label   $chanA.setlabel -text {Target V}
        spinbox $chanA.setvalue -from 0 -to $maxV -increment 1 -width 6
        label   $chanA.setunits -text {Volts}

        button $chanA.seek      -text {Ramp} -command [mymethod startRamp a $chanA.setvalue]
        button $chanA.off       -text {Off}  -command [mymethod scram     a]

        set chanB [frame $win.b -relief groove -borderwidth 3]
        label $chanB.chan       -text {Channel B}
        label $chanB.ilabel   -text {Current}
        label $chanB.ivalue   -text {00000} -width 6
        label $chanB.iunits   -text {uAmps}

        label $chanB.vlabel   -text {Voltage}
        label $chanB.vvalue   -text {0000}  -width 6
        label $chanB.vunits   -text {Volts}

        label   $chanB.setlabel -text {Target V}
        spinbox $chanB.setvalue -from 0 -to $maxV -increment 1 -width 6
        label $chanB.setunits -text {Volts}

        button $chanB.seek      -text {Ramp} -command [mymethod startRamp b $chanB.setvalue]
        button $chanB.off       -text {Off}  -command [mymethod scram     b]


        # Layout the GUI elements:


        pack $id.name -side top -fill x -expand 1
        pack $id      -side top -fill x -expand 1

        grid  $chanA.chan           -               -
        grid  $chanA.ilabel   $chanA.ivalue   $chanA.iunits
        grid  $chanA.vlabel   $chanA.vvalue   $chanA.vunits
        grid  $chanA.setlabel $chanA.setvalue $chanA.setunits
        grid  $chanA.seek           x         $chanA.off

        pack  $chanA    -side top -fill x -expand 1

        grid  $chanB.chan           -               -
        grid  $chanB.ilabel   $chanB.ivalue   $chanB.iunits
        grid  $chanB.vlabel   $chanB.vvalue   $chanB.vunits
        grid  $chanB.setlabel $chanB.setvalue $chanB.setunits
        grid  $chanB.seek           x         $chanB.off

        pack $chanB     -side top -fill x -expand 1

        # Construct the device and configure.


        $self configurelist $args
        $id.name configure -text "Name: $options(-name) @ Crate: $options(-crate) Base: $options(-base)"
        set device [vhq::create $options(-base) $options(-crate)]
        ::vhq::stat2 $device

        #   Start the ball rolling to update the display of V/I actual.
        #
        $self updateGui 500;                          # Maybe should parameterize this?

        #  Set intial value of rampspeed and current limit:

        $self setRampSpeed $options(-rampspeed)
        $self setLimits    $options(-ilimit)

    }
    #  If the timer is active it must be destroyed.
    #
    destructor {
        if {$timerid != -1} {
            after cancel $timerid
        }
        if {$updateid != -1} {
            after cancel $updateid
        }
    }
    # name configure -rampspeed volts
    #      Sets the rampspeeds for the module.
    #      This determines how quickly a channel seeks
    #      its target voltage.
    # Parmeters:
    #   volts  - A 2 element list.  The first element is for
    #            channel a, the second for channel b.  The
    #            values are integer volts/sec in the range [2, 255].
    #
    #
    onconfigure -rampspeed volts {
        # Require ramp speeds be  integers in [2,255]

        foreach limit $volts {
            if {![string is integer -strict $limit] || ($limit < 2) || ($limit > 255)} {
                error "-rampspeed must be a pair of integers in the range 2..255 inclusive"
            }
        }
        $self setRampSpeed $volts
        set options(-rampspeed) $volts

    }
    # name configure -ilimit limits
    #      Modifies the configuration parameter that describes the
    #      current limits for each channel.  Current limits are specified
    #      in uAmps.
    # Parameters:
    #   limits  - The new current limits.  This is a 2 element list
    #             The first element is the current limit for channel a
    #             the second for channel b.
    #
    #
    onconfigure -ilimit limits {
        set maxIlimit [expr 65535*$resI]
        foreach limit $limits {
            if {![string is integer -strict $limit] || ($limit < 0) || ($limit > $maxIlimit)} {
                error "-ilimit limits must be a pair fo integers in the range 0..10 inclusive."
            }
        }
        $self setLimits $limits
        set options(-ilimit) $limits
    }

    # name configure -model   202M | 203M | 204L | 205L
    #       Handles an attempt to set the model of the HV unit.
    #       The HV model determines the maximum V request allowed
    #       as well as the current resolution.
    #
    onconfigure -model model {
        #  Model list and capabilities:
        #                         name     maxV iRes
        set modelList [list [list "202L-s" 2000 0.05] \
                            [list "202M"   2000 1.5 ] \
                            [list "202M-h" 2000 2.0 ] \
                            [list "203M"   3000 0.66] \
                            [list "203M-h" 3000 1.33] \
                            [list "204L"   4000 0.33] \
                            [list "204M-h" 4000 1.5]  \
                            [list "205L"   5000 0.33]  \
                            [list "205L-h" 5000 0.66] ]
        foreach type $modelList {
            lappend typeList [lindex $type 0]
        }
        set index [lsearch -exact $typeList $model]
        if {$index >= 0} {
            set maxV [lindex [lindex $modelList $index] 1]
            set resI [lindex [lindex $modelList $index] 2]
        } else {
            error "Model $model is not in the list of known models:\n $typeList"
        }
        $win.a.setvalue configure -to $maxV
        $win.b.setvalue configure -to $maxV

        set options(-model) $model

    }
    # name configure -monitorinterval seconds
    #       Handles the attempt to configure the monitor interval.
    #       We pass this along to startTimer after validating
    #       seconds.
    # Parameters:
    #    seconds   - The number of seconds between monitor passes
    #                desired.  Must be an integer.
    onconfigure -monitorinterval seconds {
        if {![string is integer -strict $seconds]} {
            error "-monitorinterval parameter $seconds must be a positive integer"
        }
        if {$seconds <= 0} {
            error "-monitorinterval parameter $seconds must be greater than zero"
        }
        set options(-monitorinterval) $seconds
        $self startTimer

        set options(-monitorinterval) $seconds
    }


    # startRamp channel widget
    #       Initiates a ramp to the voltage requested.
    #       This ramp will be done at the most recently set rampspeed...
    #       as determined by the -rampspeed configuration parameters.
    # Parameters:
    #   channel   - a or b... the channel to ramp.
    #   widget    - A widget that responds to the get method by returning
    #               a value that is the target of the ramp.
    # Errors:
    #     Message boxes are displayed and the ramp does not occur if any
    #     of the following erroneous conditions occurs:
    # NOTE:
    #    A ramp will be attempted if the channel is tripped, but the hardware
    #    will prevent the ramp.
    #
    method startRamp {channel widget} {
        set target [$widget get]

        # Validate the target voltage:

        if {![string is integer -strict $target]} {
            tk_messageBox -name {Bad Target} -icon error \
                -message "Invalid, non-integer voltage target for ramp of channel $a -> $target"
            return
        }
        set target [expr abs($target)]
        if {$target > $maxV} {
            tk_messageBox -name {Bad Target} -icon error \
                -message "Target voltage $target is too large, maximum allowed is $maxV"
            return
        }
        #  Now do the ramp:

        $self setRampSpeed $options(-rampspeed);      # In case channel was last scrammed.
        ::vhq::setv $device $channel $target
        $self Log ramp $channel $target



    }
    # scram channel
    #       Scrams the voltage to zero.  This is done by specifying a maximum allowed
    #       ramp speed and starting a ramp -> 0.
    # Parameters:
    #   channel  - a or b the channel to scram.
    #
    method scram channel {
        set speeds $options(-rampspeed)
        if {$channel == "a"} {
            set speeds [lreplace $speeds 0 0 255]
        } else {
            set speeds [lreplace $speeds 1 1 255]
        }
        ::vhq::stat2 $device
        $self setRampSpeed $speeds
        ::vhq::setv $device $channel 0
        $self Log off $channel 0
    }

    # startTimer
    #       This internal method is used to initiate the timer
    #       that monitors the module.  The timer reschedules itself.
    #       This method just starts the ball rolling by cancelling any
    #       pending timer and scheduling the first monitor pass.
    #       Normally we'd call the monitor method itself and bury
    #       everything there, however in this case we might not be fully
    #       constructed... or constructed enough to do the initial monitor.
    #
    method startTimer {} {
    }
    # Log    what which value
    #      Logs a change. See the -command configuration option for
    #      more information about this.
    # Parameters:
    #   what  - What changed.
    #   which - Which channel changed.
    #   value - Value of the change.
    #
    method Log {what which value} {
        set script $options(-command)
        if {$script ne ""} {
            eval $script $self $what $which $value
        }
    }
    # alarm  what who state
    #      Reports a single alarm to the user if they have established an
    #      alarm script.
    #
    # Parameters:
    #   what     - Name of the alarm condition.
    #   who      - Channel (A or B).
    #   state    - State of the alarm (1 - Transitioning to set, 0 Transitioning to clear).
    #
    method alarm {what who state} {
        set script $options(-alarmscript)

        if {$script != ""} {
            eval $options(-alarmscript) $self $what $who $state
        }
    }
    # reportAlarms channel stat1 stat2
    #      Reports all alarm transitions on a particular channel
    # Parameters:
    #   channel  - The channel name (a or b).
    #   stat1    - The stat 1 list for this channel.
    #   stat2    - The stat 2 list for this channel.
    #
    method reportAlarms {channel stat1 stat2} {


        # Stat 1 has global error and manual:

        set alarmcount 0

        foreach bit [list "manual" "off" "error"] last [list lastManual lastOff lastError] {
            set state [getBit $stat1 $bit]
            set prior [set ${last}($channel)]
            if {$state} {
                incr alarmcount
            }
            if {$state != $prior} {
                $self alarm $bit $channel $state
                set ${last}($channel) $state
            }
        }
        # Stat 2 has ilimit Voverset OverVorI BadQuality.

        foreach bit  [list "ilimit"         "OverVorI"    "BadQuality"] \
                last [list lastIlimit      lastOverVorI  lastBadQuality] {
            set state [getBit $stat2 $bit]
            set prior [set ${last}($channel)]
            if {$state} {
                incr alarmcount
            }
            if {$state != $prior} {
                $self alarm $bit $channel $state
                set ${last}($channel) $state
            }
        }
        #   If there are alarm conditions, then turn the frame background red...else white.

        if {$alarmcount} {
            $win.$channel configure -background red
        } else {
            $win.$channel configure -background [. cget -background]
        }
    }
    # monitorTrips
    #      This method monitors the module for trip conditions.
    #      Trip conditions are then reported via the -alarmscript if it has been
    #      specified. More than one -alarmscript call can be generated, if there is
    #      more than one state transition.
    #
    method monitorTrips {} {
        set timerid -1

        set stat1 [vhq::stat1 $device]
        set stat2 [vhq::stat2 $device]



        $self reportAlarms "a" [lindex $stat1 0] [lindex $stat2 1]
        $self reportAlarms "b" [lindex $stat1 1] [lindex $stat2 2]

    }
    # updateGui ms
    #     Updates the gui elements from the HV module.
    #     Also reschedules itself to update again.
    # Parameters:
    #  ms   - Milliseondsto next update.
    #
    method updateGui ms {
        $self monitorTrips
        set stat1 [::vhq::stat1 $device]

        foreach channel [list a b] index [list 0 1] {
            set actual [::vhq::actual $device  $channel]
            set volts   [lindex $actual 0]
            set current [lindex $actual 1]
            set current [expr $current * $resI]

            # The voltage sign is in the plus bit of stat1:

            set plus [getBit [lindex $stat1 $index] plus]
            if {!$plus} {
                set volts [expr -1*$volts]
            }

            $win.$channel.ivalue configure -text $current
            $win.$channel.vvalue configure -text $volts
        }


        after $ms [mymethod updateGui $ms]
    }
    # setLimits    limits
    #     Sets new current limits for the module.
    #     It was up to the caller to ensure the correctness of the limits.
    # Parameters:
    #   limits   - List of current limits for each channel.
    #
    method setLimits limits {
        if {$device == {}} return
        foreach channel [list a b] limit $limits {
            set reglimit  [expr int($limit/$resI)]
            ::vhq::limit $device i $channel $reglimit
            $self Log ilimit $channel $limit
        }
    }
    # setRampSpeed rate
    #     Set the module ramp rate.  It is up to the caller to ensure
    #    the parmaeter is legal.
    # Parameters:
    #  rate - A 2 element list of integers.  Each integer is in the range [2..255].
    #
    method setRampSpeed rates {
        if {$device == {}} return
        foreach channel [list a b] rate $rates {
            ::vhq::rampspeed $device $channel $rate
            $self Log speed $channel $rate
        }

    }
    # setProperties properties
    #      Receives a property list and loads it into
    #      the module.
    # Parameters:
    #   properties - The property list to load.
    #
    method setProperties properties {
        set ilimits [$self cget -ilimit]
        set speeds  [$self cget -rampspeed]
        set targets [list [$win.a.setvalue get] [$win.b.setvalue get]]

        foreach property $properties {
            set name  [lindex $property 0]
            set value [lindex $property 1]
            set chan  [string index $name end];     # For per channel properties.
            #
            #  While this does not give the right answer if chan is not a channel
            #  selector, in that case the index is not used.
            if {$chan eq "a"} {
                set index 0
            } else {
                set index 1
            }

            switch -glob $name {
                {ilimit[ab]} {
                    set ilimits [lreplace $ilimits $index $index $value]
                }
                {speed[ab]} {
                    set speeds [lreplace $speeds $index $index $value]
                }
                {target[ab]} {
                    set targets [lreplace $targets $index $index $value]
                }
                model {
                    $self configure -model $value
                }
                default {
                    error "Unrecoginzed property $name"
                }
            }

        }
        $self configure -rampspeed $speeds
        $self configure -ilimit    $ilimits
        foreach channel [list a b] index [list 0 1] {
            $win.$channel.setvalue set [lindex $targets $index]
        }
    }
    #
    # getProperties
    #      Retrieves the module property list.
    #
    method getProperties {} {
        lappend properties [list model [$self cget -model]]

        set limit [$self cget -ilimit]
        set speed [$self cget -rampspeed]

        foreach channel [list a b] index [list 0 1] {
            lappend properties [list ilimit$channel    [lindex $limit $index]]
            lappend properties [list speed$channel [lindex $speed $index]]
            lappend properties [list target$channel    [$win.$channel.setvalue get]]
        }

        return $properties
    }
    #
    # mSec seconds
    #     Converts seconds to milliseconds.  This is used by timer functions
    #     since we ask the user to specify the monitor interval in seconds
    #     but after expects ms.
    #
    proc mSec seconds {
        return [expr {1000 * $seconds}]
    }
    # getBit bitlist name
    #      Given a bit list (list of named bits);
    #      returns the value of the named bit.  It is an
    #      error for the named bit to not exist.
    # Parameters:
    #   bitlist - A list of name bit-value pairs.
    #   name    - The name of the bit being asked for.
    # Returns:
    #    The value of the requested bit (0 or 1).
    # Errors:
    #    If the named bit is not found in the list.
    #
    proc getBit {bitlist name} {
        set idx [lsearch -glob $bitlist $name*]
        if {$idx < 0} {
            error "Not able to find bit $name in the bitlist: $bitlist"
        }
        return [lindex [lindex $bitlist $idx] 1]
    }

}
