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

# This package implements the n568Panel megawidget.  The
# megawidget allows you to control an N568b shaper module.
# The shaper module is assumed to be attached to a VME CAENNET controller
# The layout of the megawidget is as follows:
#
# +----------------------------------------------------------------+
# |  controller base@crate      Node: id     Name: Modulename      |
# |  +-------------------------------------------------------+     |
# |  |                      +-+^                             |     |
# |  |                      | || Channel                     |     |
# |  |                      +-+V                             |     |
# |  | +-+^                 +-+^           +-+^              |     |
# |  | | || Coarse Gain     | || Fine Gain | || Pole 0       |     |
# |  | +-+V                 +-+V           +-+V              |     |
# |  |                      [ ] Invert     [ ] - Polarity    |     |
# |  |       +---Shaping Time--------------------+           |     |
# |  |       | ()200ns () 1us () 3us () 6us      |           |     |
# |  |       +-----------------------------------+           |     |
# |  +-------------------------------------------------------+     |
# |                        +-+^                                    |
# |                        | || Common Offset [] Monitor output    |
# |                        +-+V                                    |
# +----------------------------------------------------------------+
#
#   Where:
#      blablabla    - is just a label or a widget label
#      [ ]          - Is a checkbutton
#      ( )          - Is a radiobutton
#      +-+^
#      | ||         - Is a spinbox
#      +-+V
#
# Options:
#     -node            - Specifies the CAENnet node address of the module.
#     -controllercrate - Specifies the VME crate of the CAENnet controller.
#     -controllerbase  - Specifies the address within the VME crate where the CAEnnet
#                        controller is.
#     -name            - Provides a name for the device.
#     -command         - Provides a script to be called when a device setting changes.
#                        The script is called with 3 element list appended to it (best a proc).
#                        The list is of the form {action id newvalue} Where:
#                        action     - Describes what is changing and is one of the following:
#                                     coarse   - Coarse gain is changing.
#                                     fine     - Fine gain is changing.
#                                     pole0    - Pole 0 correction is changing.
#                                     invert   - The invert output is changing.
#                                     polarity - The polarity has changed.
#                                     shapetime - The shaping time has changed.
#                                     offset   - The  common offset has changed.
#                        id         - Is the channel number of the channel that is changing.
#                                     for the common offset which, by definition is common
#                                     across all channels, id is an empty string.
#                        value      - Is the new value of the control parameter.
#
#     -alarmscript     - Provides a script to be called when the device triggers an alarm.
#                        Note that this device does not trigger alarms.
# Methods:
#     getProperties
#     setProperties
# Properties
#    coarsen     - The coarse gain of channel n
#    finen       - The fine gain of channel n
#    polezeron   - The pole 0 correction for channel n.
#    invertn     - The inversion setting for channel n
#    polarityn   - The polarity setting for channel n
#    shapetimen  - The shaping time for channel n
#    offset      - The common offset value.
#
#




package provide n568Panel 1.0
package require Tk
package require snit
package require BWidget
package require titledSpinBox
package require caennet
package require n568b



snit::widget n568Panel {
    option -node
    option -controllercrate 0
    option -controllerbase
    option -name
    option -command
    option -alarmscript

    # The variables below are used to keep track of the checkbuttones and radio buttons that
    # are in the gui.. since these really work best with -variable configured in.
    #

    variable Inverted         0
    variable NegativePolarity 0
    variable ShapingTime      0
    variable Monitor          0

    #   The caennet controller handle:
    #
    variable ControllerHandle


    #  Note that $self myvar does not work in snit .93 (does work in .97).
    #  since at this time 0.93 is what's with debian:
    #

    method  myvar {var} {
        append name $selfns ::$var
        return $name
    }

    constructor {args} {


        #  Connect to the device:

        set ControllerHandle [caennet::create $options(-controllerbase)\
                                              $options(-controllercrate)]

        # Now that we have successfully connected to the device we can
        # construct the gui.  First the channel frame and its components.

        label $win.doc

        set channelFrame [TitleFrame $win.channels -text "Channel" -side left]
        set container    [$channelFrame getframe]

        titledSpinBox $container.channel     -from 0 -to  15  -increment 1 -width 2 -label {Channel} \
                                             -command [mymethod changeChannel $container.channel]
        titledSpinBox $container.coarse      -values {1.0 2.0 4.0 8.0 16.0 32.0 64.0 128.0} \
                                             -width 5 -label {Coarse Gain} \
                                             -command [mymethod setCoarseGain $container.channel $container.coarse]
        titledSpinBox $container.fine        -from 0 -to 255  -increment 1 -width 3 -label {Fine Gain} \
                                            -command  [mymethod setFineGain $container.channel $container.fine]
        titledSpinBox $container.pole0       -from 0 -to 255  -increment 1 -width 3 -label {Pole 0 adjust} \
                                            -command [mymethod setPole0 $container.channel $container.pole0]


        checkbutton $container.invert   -text {Invert Output}     -variable [$self myvar Inverted] \
                                        -command [mymethod setOutputConfiguration $container.channel]
        checkbutton $container.polarity -text {- Polarity} -variable [$self myvar NegativePolarity] \
                                        -command [mymethod setInputPolarity $container.channel]

        set shapeFrame     [TitleFrame $container.shapeframe -text "Shaping Time" -side left]
        set shapecontainer [$shapeFrame getframe]

        radiobutton $shapecontainer.ns200 -text 200ns -variable [$self myvar ShapingTime] -value 0.2 \
                                        -command [mymethod setShapingTime $container.channel 0]
        radiobutton $shapecontainer.us1   -text 1us   -variable [$self myvar ShapingTime] -value 1.0 \
                                        -command [mymethod setShapingTime $container.channel 1]
        radiobutton $shapecontainer.us3   -text 3us   -variable [$self myvar ShapingTime] -value 3.0 \
                                        -command [mymethod setShapingTime $container.channel 2]
        radiobutton $shapecontainer.us6   -text 6us   -variable [$self myvar ShapingTime] -value 6.0 \
                                        -command [mymethod setShapingTime $container.channel 3]

        titledSpinBox $win.offset -from -75  -to 210 \
                                  -increment [expr 285.0/255.0] -width 6 -label {Common Offset (mV)} \
                                  -format {%.2f} -command [mymethod setOffset $win.offset]

        checkbutton $win.monitor -text {Monitor channel} -variable [$self myvar Monitor] \
                                        -command [mymethod setMonitorState $container.channel]

        # Layout the widgets

        grid $win.doc       -         -

        grid        x           $container.channel         x            -sticky w -pady 5
        grid $container.coarse  $container.fine    $container.pole0     -sticky w -pady 5
        grid       x            $container.invert  $container.polarity  -sticky w -pady 5

        pack $shapecontainer.ns200 $shapecontainer.us1 \
             $shapecontainer.us3   $shapecontainer.us6          -side left   -fill x -expand 1
        grid $shapeFrame                    -                 -            -pady 5
        grid $channelFrame    -        -
        grid $win.offset      x   $win.monitor -pady 5

        $self configurelist $args

        $win.doc configure -text "$options(-name) Controller in crate $options(-controllercrate) @ $options(-controllerbase)"

        # Now create the caennet controller if it does not alread exist and
        # store the handle to it in ControllerHandle either way.

        set ControllerHandle [::caennet::create $options(-controllerbase) $options(-controllercrate)]
        ::caennet::reset $ControllerHandle
        set id [::n568b::Id $ControllerHandle $options(-node)]
        #DEBUG puts $id

        # Load the gui with the current values:

        $self changeChannel $container.channel

    }
    #--------------------
    # changeChannel  channelWidget
    #         Changes the channel by inquiring about the values and state of the channel
    #         and loading them into the appropriate gui elements.
    # Parameters:
    #    channelWidget - the path of a widget that contains the channel number.
    #                    the widget must support the get method.
    #
    method changeChannel channelWidget {

        set channel        [$channelWidget get]
        set container      [$win.channels     getframe]
        set shapecontainer [$container.shapeframe   getframe]


        # Get the info to load into the gui elements.  We get the common
        # elements as well for simplicity.

        set Parameters [::n568b::GetChannelParameters $ControllerHandle $options(-node) $channel]
        set Offset     [offsetSettingToMv [::n568b::GetOffset $ControllerHandle $options(-node)]]
        set MuxStatus  [::n568b::GetMuxStatus $ControllerHandle $options(-node)]

        # Load the channel specific gui elements.

        $container.fine   set [lindex $Parameters 0]
        $container.pole0  set [lindex $Parameters 1]
        setCheckButton    $container.polarity [lindex $Parameters 2]
        setCheckButton    $container.invert   [lindex $Parameters 3]
        set ShapingTime   [lindex $Parameters 4]
        $container.coarse set [lindex $Parameters 5]
        $win.offset       set $Offset
        setCheckButton    $win.monitor [lindex $MuxStatus 0]


    }
    #--------------------
    # setChannelCoarseGain  channel gain
    #         Sets and logs the setting of the coarse gain of a channel
    # Parameters:
    #   channel    - Number of the channel to set.
    #   gain       - New coarse gain.
    #
    method setChannelCoarseGain {channel gain} {
        ::n568b::SetCoarseGain $ControllerHandle $options(-node) $channel [gainValue $gain]
        $self Log coarse $channel $gain
    }
    #--------------------
    # setCoarseGain chanWidget gainWidget
    #         Responds to a change in the coarse gain widget.
    #         The coarse gain for the currently selected channel is
    #         set.
    # Parameters:
    #   chanWidget   - Widget containing the channel... must support get.
    #   gainWidget   - Widget containing the coarse gain... must support get.
    #
    method setCoarseGain {chanWidget gainWidget} {
        set channel [$chanWidget get]
        set gain    [$gainWidget get]

        $self setChannelCoarseGain $channel $gain
    }
    #--------------------
    # setChannelFineGain channel gain
    #        Sets the fine gain for a channel of the shaper.
    # Parameters:
    #    channel  - Channel number to set.
    #    gain     - New gain value.
    #
    method setChannelFineGain {channel gain} {
        ::n568b::SetFineGain $ControllerHandle $options(-node) $channel $gain
        $self Log fine $channel $gain
    }

    #--------------------
    # setFineGain  chanWidget gainWidget
    #        Sets the fine gain for a channel given a pair
    #        of widgets that determine what to do.
    # Parameters:
    #  chanWidget  - Widget that has a get method to tell which channel
    #                to change.
    #  gainWidget  - Widget that has a get method to tell the new fine gain
    #                for the channel.
    #
    method setFineGain {chanWidget gainWidget} {
        set channel [$chanWidget get]
        set gain    [$gainWidget get]

        $self setChannelFineGain $channel $gain
    }
    #--------------------
    # setChannelPole0  channel value
    #        Sets a new value for the Pole0 adjustment see
    #        SetPole0 for more information about what that means
    #        or read about amplifier transfer functions in any good
    #        amplifier text.
    # Parameters:
    #   channel - Number of the channel to modify.
    #   value   - New value for the pole 0 adjustment
    #
    method setChannelPole0 {channel value} {
        ::n568b::SetPole0 $ControllerHandle $options(-node) $channel $value
        $self Log pole0 $channel $value
    }
    #--------------------
    # setPole0  channelWidget valueWidget
    #         Sets a new value for the Pole0 adjustment.  This value adjusts
    #         the magnitude of the offset for the poles in the amplifier
    #         transfer function.
    # Parameters:
    #   channelWidget   - A widget that has a get method that returns the
    #                     number of the channel to adjust.
    #   valueWidget     - A widget that has a get method that returns the
    #                     new value of the pole 0 compensation.
    #
    method setPole0 {channelWidget valueWidget} {
        set channel [$channelWidget get]
        set value   [$valueWidget   get]

        $self setChannelPole0 $channel $value
    }
    #--------------------
    # setChannelOutputConfiguration channel value
    #         Sets the output configuration of the specified channel
    # Parameters:
    #  channel   - Channel number.
    #  value     - New value, 1 for inverted, 0 for normal.
    #
    method setChannelOutputConfiguration {channel value} {
        ::n568b::SetOutputConfiguration $ControllerHandle $options(-node) $channel $value
        $self Log invert $channel $value
    }

    #--------------------
    # setOutputConfiguration chanWidget
    #         Set the output configuration.
    # Parameters:
    #   chanWidget  - A widget whose get method returns the number of the channel
    #                to modify.
    # Implicit inputs:
    #    Inverted is a variable that corresponds to the new value of the output configuration.
    #
    method setOutputConfiguration chanWidget {
        set channel [$chanWidget get]
        $self setChannelOutputConfiguration $channel $Inverted
    }
    #--------------------
    # setChannelInputPolarity channel state
    #        Sets the input polarity of a channel to 'state'.
    # Parameters:
    #   channel  - The number of the channel to change.
    #   state    - the new state.
    #
    method setChannelInputPolarity {channel state} {
        ::n568b::SetPolarity $ControllerHandle $options(-node) $channel $state
        $self Log polarity $channel $state
    }

    #--------------------
    # setInputPolarity chanWidget
    #        Set an input polarity value.
    # Parameters:
    #   chanWidget  - A widget whose get member returns the number of the channel
    #                 to modify.
    # Implicit Inputs:
    #   NegativePolarity - True if the polarity should be negative.
    #
    method setInputPolarity chanWidget {
        set channel [$chanWidget get]
        $self setChannelInputPolarity $channel $NegativePolarity
    }
    #--------------------
    # setChannelShapingTime channel selector
    #         Sets the shaping time of an individual channel.
    # Parameters:
    #   channel   - Number of the channel to affect.
    #   selector  - Selects the new shaping time.
    #
    method setChannelShapingTime {channel selector} {
        ::n568b::SetShapingTime $ControllerHandle $options(-node) $channel $selector
        $self Log shapetime $channel $selector
    }
    #--------------------
    # setShapingTime  chanWidget selector
    #         Reacts to a hit on a shaping time radio button.
    # Parameters:
    #   chanWidget  -  A widget whose get method returns the number of the
    #                  current channel.
    #   selector    - A Selector for the new shapging time .
    #
    method setShapingTime {chanWidget selector} {
        set channel [$chanWidget get]
        $self setChannelShapingTime $channel $selector
    }
    #--------------------
    # setCommonOffset offset
    #         Set the common offset given a new value for it.
    # Parameters:
    #    offset   - The new offset in Mv
    #
    method setCommonOffset offset {
        set offset [offsetMvToSetting $offset]
        ::n568b::SetOffset $ControllerHandle $options(-node) $offset
        $self Log offset [list] $offset
    }
    #--------------------
    # setOffset valueWidget
    #         Responds to changes to the common offset widget.
    #         This is a common offset across the module so there
    #         is no need for a channel.
    # Parameters:
    #   valueWidget - A widget that will provide the common offset value
    #                 as a return value from its get method.
    #
    method setOffset valueWidget {
        set offset [$valueWidget get]
        $self setCommonOffset $offset
    }
    #--------------------
    # setMonitorState channelWidget
    #        Reacts to changes in the monitor state.
    # Parameters:
    #  channelWidget - A widget that will give the current channel
    #                  when selected. This is needed because the
    #                  monitor will monitor the channel that was last
    #                  operated on, and while changing the channel probably
    #                  ensures that, we always want to play it safe against
    #                  stuff like the module forgetting what we last told it
    #                  due to power being pulled e.g.
    #
    method setMonitorState channelWidget {
        set channel [$channelWidget get]
        if {$Monitor} {
            ::n568b::EnableMuxOut $ControllerHandle $options(-node)
            ::n568b::GetChannelParameters $ControllerHandle $options(-node) $channel
        } else {
            ::n568b::DisableMuxOut $ControllerHandle $options(-node)
        }
    }
    #--------------------
    # setProperties   list
    #         Provides this object with a property list that must be
    #         loaded into the device settings.  See the comment headers
    #         at the top of this package for more information about
    #         supported properties.
    # Parameters:
    #   list  - The property list to load.
    #
    method setProperties {list} {
        foreach property $list {
            set name  [lindex $property 0]
            set value [lindex $property 1]
            switch -glob $name {
                coarse* {
                    scan $name "coarse%d" channel
                    $self setChannelCoarseGain $channel $value
                }
                fine*   {
                    scan $name "fine%d" channel
                    $self setChannelFineGain $channel $value
                }
                polezero* {
                    scan $name "polezero%d" channel
                    $self setChannelPole0 $channel $value
                }
                invert*   {
                    scan $name "invert%d" channel
                    if {($value eq "inverted") || ($value == 1) } {
                        set invert 1
                    } else {
                        set invert 0
                    }
                    $self setChannelOutputConfiguration $channel $invert
                }
                polarity* {
                    scan $name "polarity%d" channel
                    if {($value eq "-") || ($value == 1)} {
                        set neg 1
                    } else {
                        set neg 0
                    }
                    $self setChannelInputPolarity $channel $neg
                }
                shapetime* {
                    scan $name "shapetime%d" channel
                    set  sel [lsearch -exact [list 0.2 1.0 3.0 6.0] $value]
                    if {$sel == -1} {
                        error "Invalid value for width in $name : $value must be 0.2 1.0 3.0 or 6.0"
                    }
                    $self setChannelShapingTime $channel $sel
                }
                offset {
                    $self setCommonOffset $value
                }
                default {
                    error "Unrecognized keyword in propertylist"
                }
            }


        }
        set container [$win.channels getframe]
        $self changeChannel $container.channel
    }
    #--------------------
    # getProperties
    #         Returns the current device settings in the form of a property list.
    # Returns:
    #         The property list.  See the comment headers at the top of this package
    #         for more information about the properties supported by this widget.
    #
    method getProperties {} {
        set information [::n568b::GetAllParameters $ControllerHandle $options(-node)]

        # Element 16 of the list is the offset:

        lappend properties [list offset [offsetSettingToMv [lindex $information 16]]]
        for {set chan 0} {$chan < 16} {incr chan} {
            set chaninfo [lindex $information $chan]
            lappend properties [list fine$chan      [lindex $chaninfo 0]]
            lappend properties [list polezero$chan  [lindex $chaninfo 1]]
            lappend properties [list polarity$chan  [lindex $chaninfo 2]]
            lappend properties [list invert$chan    [lindex $chaninfo 3]]
            lappend properties [list shapetime$chan [lindex $chaninfo 4]]
            lappend properties [list coarse$chan    [lindex $chaninfo 5]]

        }

        return $properties
    }
    #---------------------
    #  Log  action id value
    #        Common method to trigger a user trace on modification
    #        requested by the user via the -command option.
    # Parameters:
    #   action    - What was done (see comment headers).
    #   id        - Who it was done to (usually the channel number).
    #   value     - New value of the control parameter.
    #
    method Log {action id value} {
        set script $options(-command)
        if {$script ne ""} {
            eval $options(-command) [list $action $id $value]
        }
    }
    #---------------------
    #  setCheckButton widget value
    #         Utility proc to set a checkbutton to a value
    #  Parameters:
    #    widget   - The checkbutton to set.
    #    value    - A boolean. If true the widget is selected other wise, deselected.
    #               Note the following are also recognized values:
    #               -   - True
    #               +   - False
    #               inverted - True
    #               normal   - False
    #     Intended to support the needs of this widget alone.
    #
    proc setCheckButton {widget value} {

        if {$value eq "-"} {
            set value 1
        }
        if {$value eq "+"} {
            set value 0
        }
        if {$value eq "inverted"} {
            set value 1
        }
        if {$value eq "normal"} {
            set value 0
        }
        if {$value} {
            $widget select
        } else {
            $widget deselect
        }
    }
    #-------------
    #  gainValue mask
    #         Returns the value of the gain (what the controller wants)
    #         given a gain  mask... This must determines which bit number is
    #         set in mask
    # Parameters:
    #  mask   - The gain bitmask.
    #
    proc gainValue mask {
        set bit 1
        set mask [expr int($mask)]
        for {set value 0} {$value < 8} {incr value} {
            if {$bit & $mask} {
                return $value
            } else {
                set bit [expr {$bit << 1}]
            }
        }
        error "Bad gain mask [format %x $mask]"
    }
    #---------------
    #  offsetMvToSetting mv
    #              Converts an offset in mv to the appropriate
    #              settings value.
    # Parametesr:
    #   mv   - The mv to convert.
    #
    proc offsetMvToSetting mv {

        set setting [expr {int(($mv + 75.0)*255.0/285.0 + 0.5)}]
        return $setting
    }

    #---------------
    #   offsetSettingToMv setting
    #              Converts an offset in settings units to mv.
    #  Parameters:
    #   seeting - the settings value.
    proc offsetSettingToMv setting {
        set mv [expr {($setting*285.0/255.0) - 75.0}]
        return [format "%.2f" $mv]
    }

}
