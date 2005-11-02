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
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321
#


package require Tk
package require snit

#  Support packages for the CFD:

package require CFD812   2.0
package require CFDState 2.0


package provide caenv812Gui 1.0

# This is a GUI for the CAEN V812 constant fraction discriminator.
# unlike the previous >BIG FOOTPRINT< version, this is a small footprint
# version that allows you to control a single channel at a time.
# The GUI layout is as follows:
#
#   +--------------------------------------------------------------------+
#   |    CAENV812 @          base_address  Serial Number: serial_number  |
#   |                     +------------------------------+               |
#   |                     |  Channel:  |  |^             |               |
#   |                     |            |  |V             |               |
#   |                     |                              |               |
#   |                     |  Threshold |  |^             |               |
#   |                     |            |  |V             |               |
#   |                     |                              |               |
#   |                     | Enable [ ]  <Set All>        |               |
#   |                     +------------------------------+               |
#   | +------------------------+             +-----------------------+   |
#   | |Widths                  |             | Deadtimes             |   |
#   | |Channels 0-7  |  |^     |             | Channels 0-7  | |^    |   |
#   | |Channels 8-15 |  |^     |             | Channels 8-15 | |V    |   |
#   | +------------------------+             +-----------------------+   |
#   |                      Majority Threshold | |^                       |
#   +--------------------------------------------------------------------+
#
#  Options:
#      -crate     The VME crate that has the module.
#      -base      The base address of the module in the crate.
#      -name      The name of the module.  Used to set the name of the state
#                 of the module in CFDState.  If not supplied, we use the object
#                 name at the time of construction.
#      -command   Supplies a script that will be invoked to describe changes done
#                 the script will have a list appended to it of the following form:
#                 {action id newvalue}
#                 Where:
#                     action   is text describing what changed:
#                              threshold  - A threshold changed.
#                              width      - A width changed.
#                              deadtime   - A deadtime changed.
#                              majority   - The majority threshold changed.
#                              mask       - The enables mask.
#
#                     newvalue is a new control value associated with the
#                              change.
#                     id       Describes which item was changed.  This will be a channel number
#                              for thresholds, 0 or 1 for widths/deadtimes and an empty string for
#                              majority threshold and the mask.
#      -alarmscript  Supplies a script that will be invoked if the module has any alarms to report.
#                    Note that this module has no alarm conditions however this option is provided
#                    to unify the interface of all control widgets
#
# Properties:
#     threshold0 .. threshold15  - The thresholds of the module's 16 channels.
#     width0, width1             - The widths of the lower and upper 1/2 of the channels.
#     deadtime0, deadtime1       - The deadtimes of the lower and upper 1/2 of the channels.
#     majority                   - The majority threshold.
#     enables                    - The enables mask.
# Standard methods:
#     getProperties              - Returns the current property list.
#     setProperties              - Sets the module's current property lists.
#
snit::widget cfdv812Gui {
    option -crate 0
    option -base  {}
    option -name  {}
    option -command {}
    option -alarmscript {}


    variable cfd;        # holds the handle to the cfd object.

    constructor args {
        set options(-name) $self
        $self configurelist $args


        # Get the device and state 'objects' made.

        $self createDevice
        $self createState

        # Since all that worked we can now layout the GUI.

        # all the labels at the top of the window:

        label $win.baselabel   -text {Base Addr: }
        label $win.baseaddress -text $options(-base)

        label $win.seriallabel -text {Serial Number: }
        label $win.serialnum   -text [::CFD812::GetSerialNumber $cfd]

        # The channel selector and threshold box:
        #

        set channel [frame $win.channel -relief groove -borderwidth 3]
        label   $channel.chanlabel      -text {Channel }
        spinbox $channel.channel        -from 0 -to 15 -increment 1 -width 2   \
                                        -command [mymethod changeChannel $channel.channel]
        label   $channel.threshlabel    -text {Threshold (V)}
        spinbox $channel.threshold      -from -255 -to -1 -increment 1 -width 4 \
                                        -command [mymethod setThreshold $channel.channel $channel.threshold]
        checkbutton $channel.enable    -text {Enable} -command [mymethod flipDisable $channel.channel]
        button      $channel.setall     -text {Set All} \
                                        -command [mymethod setAll $channel.threshold]

        #   The Widths:

        set widths [frame $win.widths -relief groove -borderwidth 3]
        label $widths.label    -text {Width (Arb)}
        label $widths.w07label -text {Channels 0-7}
        spinbox $widths.w07    -from 0 -to 255 -increment 1 -width 3 \
                                -command [mymethod setWidth $widths.w07 0]
        label $widths.w815label -text {Channels 8-15}
        spinbox $widths.w815    -from 0 -to 255 -increment 1 -width 3 \
                                -command [mymethod setWidth $widths.w815 1]


        #  The deadtimes.

        set deadtimes [frame $win.deadtime -relief groove -borderwidth 3]
        label $deadtimes.label    -text {Deadtime (Arb)}
        label $deadtimes.d07label -text {Channels 0-7}
        spinbox $deadtimes.d07    -from 0 -to 255 -increment 1 -width 3 \
                                -command [mymethod setDeadtime $deadtimes.d07 0]
        label $deadtimes.d815label -text {Channesl 8-15}
        spinbox $deadtimes.d815   -from 0 -to 255 -increment 1 -width 3  \
                                -command [mymethod setDeadtime $deadtimes.d815 1]

        # Majority threshold:

        label   $win.majoritylabel -text {Majority Threshold}
        spinbox $win.majority      -from 1 -to 16 -increment 1 -width 3 \
                                -command [mymethod setMajorityLevel $win.majority]

        # Setup the geometry:

        #  The channel subframe.

        grid $channel.chanlabel   $channel.channel
        grid $channel.threshlabel $channel.threshold
        grid $channel.enable      $channel.setall

        # The widths subframe.

        grid $widths.label              x
        grid $widths.w07label     $widths.w07  -sticky w
        grid $widths.w815label    $widths.w815 -sticky w

        # The deadtimes subframe

        grid $deadtimes.label           x
        grid $deadtimes.d07label  $deadtimes.d07  -sticky w
        grid $deadtimes.d815label $deadtimes.d815 -sticky w

        # The overall layout.


        grid $win.baselabel $win.baseaddress $win.seriallabel $win.serialnum
        grid      x         $win.channel           -              x -sticky ew
        grid $widths             -           $deadtimes           - -sticky w
        grid      x         $win.majoritylabel $win.majority      x

        # Set bindings so that Enter and Focus out will also change values.
        #
        bind $channel.threshold <FocusOut> [mymethod setThreshold $channel.channel $channel.threshold]
        bind $channel.threshold <Return>   [mymethod setThreshold $channel.channel $channel.threshold]

        bind $widths.w07        <FocusOut> [mymethod setWidth $widths.w07 0]
        bind $widths.w07        <Return>   [mymethod setWidth $widths.w07 0]

        bind $widths.w815       <FocusOut> [mymethod setWidth $widths.w815 1]
        bind $widths.w815       <Return>   [mymethod setWidth $widths.w815 1]

        bind $deadtimes.d07     <FocusOut> [mymethod setDeadtime $deadtimes.d07 0]
        bind $deadtimes.d07     <Return>   [mymethod setDeadtime $deadtimes.d07 0]
        bind $deadtimes.d815    <FocusOut> [mymethod setDeadtime $deadtimes.d815 1]
        bind $deadtimes.d815    <Return>   [mymethod setDeadtime $deadtimes.d815 1]

        bind $win.majority      <FocusOut> [mymethod setMajorityLevel $win.majority]
        bind $win.majority      <Return>   [mymethod setMajorityLevel $win.majority]

        $self selectChannel 0


    }
    # destructor:
    #    Called when one of these widgets is destroyed.
    #

    destructor {
        $self destroyDevice
        $self destroyState

    }
    # setThreshold channel threshold
    #        Sets the threshold to a new value.
    # Parameters:
    #    channel    - spinbox widget that holds the current channel
    #    threshold  - spinbox widget that holds the current threshold
    # NOTE:
    #    Actualy the widgets can be anything that supports a get method.
    #
    method setThreshold {channel threshold} {
        set chanNum     [$channel   get]
        set threshValue [$threshold get]
        $self setChannelThreshold $chanNum $threshValue
    }
    # setAll threshold
    #     Sets the threshold for all channels to a new value.
    #
    # Parameters:
    #   threshold  - A widget that supports a get method that will return the
    #                requested threshold.
    #
    method setAll threshold {
        set threshValue  [$threshold get]
        for {set chanNum 0} {$chanNum < 16} {incr chanNum} {
            $self setChannelThreshold $chanNum $threshValue
        }
    }
    # createDevice
    #       Create the initial cfd device 'object.'
    #
    method createDevice {} {
        set cfd [::CFD812::Map $options(-base) $options(-name) $options(-crate)]

    }
    # createState
    #      Create the intial cfd state 'object'
    #
    method createState  {} {
        ::CFDState::Create $options(-name)
    }
    # destroyDevice
    #     Destroy the cfd device object.
    #
    method destroyDevice {} {
        ::CFD812::Unmap $options(-name)
    }
    # destroyState
    #    Destroy the state object.
    #
    method destroyState {} {
        ::CFDState::Destroy $options(-name)
    }
    # setChannelThreshold channel value (private)
    #     Set the threshold of a channel to value
    # Parameters:
    #    channel  - The >number< of the channel.
    #    value    - the >value< of the threshold.
    #
    method setChannelThreshold {channel value} {
        ::CFD812::SetThreshold   $cfd            $channel $value
        ::CFDState::SetThreshold $options(-name) $channel $value
        $self log threshold $channel $value
    }
    # selectChannel channel
    #       Change the channel to the one in the spinbox.  We
    #       fetch the new channel's threshold and enable state from
    #       our state object (the cfd is write-only) and set the
    #       corresponding gui widgets.
    # Parameters:
    #   channel  - The channel we are switching to
    method selectChannel channel {
        set threshValue [::CFDState::GetThreshold $options(-name) $channel]
        set mask        [::CFDState::getMask      $options(-name)]

        $win.channel.threshold set $threshValue
        if {($mask & [expr 1 << $channel]) != 0} {
            $win.channel.enable select

        } else {
            $win.channel.enable deselect
        }

    }
    # changeChannel spinbox
    #         Change the channel to the one on the spinbox.. We fetch the
    #         channel and call selectChannel to do the actual work.
    # Parameters
    #   spinbox  - A widget that on being given a 'get' method will return
    #              the desired channel number.
    #
    method changeChannel spinbox {
        set channel [$spinbox get]
        $self selectChannel $channel
    }
    # setModuleEnables mask
    #        Set the module's enable mask
    # Parameters:
    #   mask  - The new mask.
    #
    method setModuleEnables mask {
        ::CFD812::SetMask   $cfd $mask
        ::CFDState::setMask $options(-name) $mask
        $self log mask {} [format "0x%x" $mask]
    }
    # flipDisable spinbox
    #         Toggles the state of the disable bit from the one memorized in the mask.
    # Parameters:
    #   spinbox  - A widget which, when get is invoked, returns the channel.
    #
    method flipDisable spinbox {
        set channel  [$spinbox get]
        set mask     [::CFDState::getMask $options(-name)]
        set chanbit  [expr 1 << $channel]
        set mask     [expr $mask ^ $chanbit]
        if {($mask & $chanbit) != 0} {
            $win.channel.enable select
        } else {
            $win.channel.enable deselect
        }
        $self setModuleEnables $mask

    }
    # setBankWidth bank value
    #        Set the width of a bank of 8 channels.
    # Parameters:
    # bank    - The bank number (0 for 0-7, 1 for 8-15).
    # width   - The new width value in arbitrary units.
    #
    method setBankWidth {bank width} {
        ::CFD812::SetWidth $cfd $bank $width
        ::CFDState::SetWidth $options(-name) $bank $width
        $self log width $bank $width
    }
    # setWidth spinbox bank
    #        Updates the width of a discriminator bank.  See Parameters
    #        below for more on what constitutes a bank.
    # Parameters:
    #  spinbox   - A widget which, when the 'get' method is applied to it returns
    #              the new width.
    #  bank      - 0 if the width is for channels 0-7, 1 if for channels 8-15. Groups of
    #              8 channels share a single width.
    #
    method setWidth {spinbox bank} {
        set width [$spinbox get]
        $self setBankWidth $bank $width
    }
    # setBankDeadtime bank deadtime
    #      Set the deadtime for a bank of the discriminator.
    # Parameters:
    #    bank    - The bank to set (0 for chans 0-7, 1 for chans 8-15).
    #    deadtime- The new deadtime in arbitrary units.
    #
    method setBankDeadtime {bank deadtime} {
        ::CFD812::SetDeadtime    $cfd $bank $deadtime
        ::CFDState::SetDeadtime  $options(-name) $bank $deadtime
        $self log deadtime $bank $deadtime
    }
    # setDeadtime spinbox bank
    #       Updates the deadtime value of a discriminator bank.  See the
    #       Parameters comments for a definition of discriminator bank.
    # Parameters:
    #    spinbox  - A widget which, when the 'get' method is applied to it
    #               returns the new deadtime.
    #    bank     - The discriminator bank to set.  Bank 0 is channels 0-7,
    #               Bank 1 is channels 8-15.
    #
    method setDeadtime {spinbox bank} {
        set deadtime [$spinbox get]
        $self setBankDeadtime $bank $deadtime

    }
    # setModuleMajority majority
    #       Set the threshold for the majority logic unit.
    # Parameters
    #   majority - The new majority threshold.
    #
    method setModuleMajority majority {
        ::CFD812::SetMultiplicityThreshold $cfd $majority
        ::CFDState::SetMultiplicity $options(-name) $majority
        $self log majority {} $majority
    }
    # setMajorityLevel spinbox
    #        Set a new majority level for the module.
    # Parameters:
    #   spinbox - A widget that supports the get method to return its value.
    #
    method setMajorityLevel spinbox {
        set majority [$spinbox get]
        $self setModuleMajority $majority
    }
    # log  what which value
    #     Logs a module change.   If -options(-command) is non-null the
    #     callback script is constructed and invoked.
    # Parameters (see the -command option description for more detail):
    #   what   - What has changed.
    #   which  - Which one of them has changed
    #   value  - New value of the thing that changed.
    #
    method log {what which value} {
        set script $options(-command)
        if {$script ne ""} {
            eval $script [list $what $which $value]
        }
    }
    # setProperties list
    #      Asks the module to set values from the attached property list.
    #      see the module header for more information about the supported properties.
    # Parameters:
    #   list  - a list of {name value} pairs that make up the property list.
    #
    method setProperties {list} {
        foreach item $list {
            set property [lindex $item 0]
            set value    [lindex $item 1]
            switch -glob $property {
                threshold* {
                    scan $property "threshold%d" channel
                    $self setChannelThreshold $channel $value
                }
                width* {
                    scan $property "width%d" bank
                    $self setBankWidth $bank $value
                }
                deadtime* {
                    scan $property "deadtime%d" bank
                    $self setBankDeadtime $bank $value
                }
                majority {
                    $self setModuleMajority $value
                }
                enables {
                    $self setModuleEnables $value
                }
                default {
                    error "Bad property in CAEN V812 propertylist: $property $value"
                }
            }
        }
        $self selectChannel [$win.channel.channel get]
        $win.widths.w07  set [::CFDState::GetWidth $options(-name) 0]
        $win.widths.w815 set [::CFDState::GetWidth $options(-name) 1]

        $win.deadtime.d07  set [::CFDState::GetDeadtime $options(-name) 0]
        $win.deadtime.d815 set [::CFDState::GetDeadtime $options(-name) 1]

        $win.majority       set [::CFDState::GetMultiplicity $options(-name)]
    }
    # getProperties
    #      Returns the module's current property list.
    #
    method getProperties {} {
        for {set i 0} {$i < 16} {incr i} {
            set property [format threshold%d $i]
            set value    [::CFDState::GetThreshold $options(-name) $i]
            lappend properties [list $property $value]
        }
        foreach i {0 1} {
            set property [format width%d $i]
            set value    [::CFDState::GetWidth $options(-name) $i]
            lappend properties [list $property $value]
            set property [format deadtime%d $i]
            set value    [::CFDState::GetDeadtime $options(-name) $i]
            lappend properties [list $property $value]
        }
        lappend properties [list majority [::CFDState::GetMultiplicity $options(-name)]]

        return $properties
    }
}
