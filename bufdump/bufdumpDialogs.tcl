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

#   This file provides the dialog classes used by the
#   bufdump program.
#

package provide bufdumpDialogs 1.0
package require snit
package require Iwidgets
package require BWidget

# Common functionality that is factored out of all dialogs:

namespace eval  bufdumpDialogs  {

}
#------------------------------------------------------------------------------
# bufdumpDialogs::setListBox widget list
#     Sets the contents of a list box to the specified list of values
# Parameters:
#    widget  - the widget to fill
#    list    - the list to fill it with.
# NOTE
#   Any existing values are removed.
proc bufdumpDialogs::setListBox {widget list} {
    set state [$widget cget -state]

    $widget configure -state normal
    $widget delete 0 end
    eval $widget insert end $list

    $widget configure -state $state
}
#------------------------------------------------------------------------------
#  bufdumpDialogs::moveSelection from to
#     Moves the selected widgets from one list box to another.
#
proc bufdumpDialogs::moveSelection {from to} {
    set sel [$from curselection]
    if {[llength $sel] > 0} {
        foreach i $sel {
            set value [$from get $i]
            $to insert end $value
        }
        #  Need to remove from back to front or indices get perverted:

        set sel [lsort -decreasing -integer $sel]
        foreach i $sel {
            $from delete $i
        }
    }
}
#------------------------------------------------------------------------------
#  bufdumpDialogs::selectionUp widget
#     If a pattern in a list box is  selected, it is moved upwards
#     if possible.
# Parameters:
#  widget - The list box.
#
proc bufdumpDialogs::selectionUp {widget} {
    set sel [$widget curselection]
    if {[llength $sel] == 1} {
        if {$sel != 0} {
            set value [$widget get $sel]
            $widget delete $sel
            incr sel -1
            $widget insert $sel $value
        }
    }
}
#------------------------------------------------------------------------------
# bufdumpDialogs::selectionDown widget
#    If a pattern in a list box is selected, it is moved downwards if
#    possible.
# Parameters:
#  widget  - A list box.
#
proc bufdumpDialogs::selectionDown {widget} {
    set last [$widget index end]
    set sel  [$widget curselection]
    if {[llength $sel] == 1} {
        if {$sel != $last} {
            set value [$widget get $sel]
            $widget delete $sel
            incr sel
            $widget insert $sel $value
        }
    }
}

#------------------------------------------------------------------------------
# bufdumpDialogs::removeListboxSelection widget
#      If there's a selection in the listbox, it is removed
#      from the list box.
# Parameters:
#   widget  - The lisbox.
#
proc bufdumpDialogs::removeListboxSelection widget {
    set sel [$widget curselection]
    if {[llength $sel] == 1} {
        $widget delete $sel
    }
}

#-------------------------------------------------------------------
# dataSourceDialog
#      This dialog provides a way for the user to select a
#      data source... online or offline.
#      The dialog layout is shown below:
#
#   +----------------------------------------------+
#   |    [ ] Online       [        ]^V Buffer size |
#   +----------------------------------------------+
#   |    Hostname  <                         >     |
#   +----------------------------------------------+
#   |     File chooser box                         |
#   +----------------------------------------------+
#   |   [OK]                [Cancel]               |
#   +----------------------------------------------+
#
# Behavioral constraints:
#     If online is checked the file chooser box is disabled.
#     If not, the hostname entry is disabled.
#
# Options:
#    -online      - True if the user selects online.
#    -buffersize  - Current value of the buffer size spinbox.
#    -host        - Current value of the host entry.
#    -path        - Current value of the filename in the file chooser box.
#    -command     - Script called when ok is clicked.
#    -cancelcommand
#                 - Script that is executed when the cancel button is clicked.
# Methods:
#    modal        - turns the dialog box into a modal dialog.
#
snit::widget dataSourceDialog {
    hulltype toplevel

    option -online        0
    option -buffersize    8192
    option -host          localhost
    option -path          {}
    option -command       {}
    option -cancelcommand {}

    constructor args {
        # Setup the widgets...

        set invariant  [frame $win.inv  -relief groove -borderwidth 4]
        checkbutton $invariant.online   -variable [$self MyVar options(-online)] \
                                        -command  [mymethod onOnlineToggle]      \
                                        -text Online
        spinbox     $invariant.bufsize  -values {8192 16348 32768 65536 131072 262144 524288 1048576 
	                                         2097152 4194304 8388608 1677216}         \
                                        -textvariable [$self MyVar options(-buffersize)]
        label       $invariant.buflabel -text " Byte buffers"

        set online     [frame $win.online  -relief groove -borderwidth 4]
        label $online.label -text {Host: }
        entry $online.entry -textvariable [$self MyVar options(-host)]

        set offline    [frame $win.offline -relief groove -borderwidth 4]
        ::iwidgets::fileselectionbox $offline.file -mask *.evt


        set action     [frame $win.action   -relief groove -borderwidth 4]
        button $action.ok     -text Ok     -command [mymethod onOk]
        button $action.cancel -text Cancel -command [mymethod onCancel]


        grid $invariant.online $invariant.bufsize $invariant.buflabel

        grid $online.label $online.entry

        pack $offline.file -fill both -expand 1

        pack $action.ok $action.cancel -side left

        pack $invariant $online $offline $action -side top -fill x -expand 1


        # configure them...

         $self configurelist $args

        #  Setup the ghosting.

        $self onOnlineToggle
    }
    ####
    #  configure -path name
    #     Puts a name in the file path.
    # Parameters:
    #   name  - The name to put.
    #
    onconfigure -path name {
        $offline.file.selection clear
        $offline.file.selection insert end $name
    }
    ####
    #  cget -path
    #      Returns the current name of the file in the file selection box.
    #
    oncget -path {
        return [$win.offline.file   get]
    }
    #####
    # MyVar  name
    #     Fully qualifies a variable name.
    # Parameters:
    #   name   - The name to add namespace decorations too.
    #
    method MyVar name {
        append fullname $selfns :: $name
        return $fullname
    }
    ######
    # setEnabledWidgets
    #     Using the value of the -online configuration option
    #     enable and disable the appropriate widgets.
    #
    method onOnlineToggle {} {
        if {$options(-online)} {
            $win.online.entry configure -state normal
            $win.offline.file.selection configure -state disabled
            $win.offline.file.files     configure -state disabled
            $win.offline.file.dirs      configure -state disabled
            $win.offline.file.filter    configure -state disabled
        } else {
            $win.online.entry configure -state disabled
            $win.offline.file.selection configure -state normal
            $win.offline.file.files     configure -state normal
            $win.offline.file.dirs      configure -state normal
            $win.offline.file.filter    configure -state normal

        }
    }
    ########
    #  dispatch  option
    #      Dispatch a script that is in an option
    # Parameters:
    #   option  - the option name e.g. -command
    #
    method dispatch option {
        set script $options($option)
        if {$script ne [list]} {
            eval $script
        }
    }
    ########
    #  modal
    #     Makes the dialog a modal one.
    #
    method modal {} {

        frame $win.hidden
        focus $win
        wm deiconify $win
        after 10 [list grab $win]

        tkwait window $win.hidden

        grab release $win
    }
    #######
    #  endmodal
    #       Ends the modality of a dialog.
    #
    method endmodal {} {
        if {[winfo exists $win.hidden]} {
            destroy $win.hidden
        }
    }
    #######
    #  onOk
    #     Called in response to the Ok button being clicked.
    #
    method onOk {} {
        $self endmodal
        $self dispatch -command
    }
    ######
    # onCancel
    #    Called in response to the Cancdel button being clicked.
    #
    method onCancel {} {
        $self endmodal
        $self dispatch -cancelcommand
    }


}

#----------------------------------------------------------------
# verticalSpinBox
#     This widget creates a vertical spinbox.
#     The vertical spinbox looks like this:
#         ^
#        [  ]
#         V
#  Where the ^/V arrow buttons are used to flip between
#  the allowed values of the (not editable) data field.
#
# Options:
#    -values    - A list of legal values for the spinbox (the set the
#                 arrows flip between.. This is considered a circular list.
# Methods:
#    get        - Get the current value of the spinbox.
#
snit::widget verticalSpinBox {
    option -values {0 1 2 3 4 5 6 7 8 9}
    option -state  normal
    variable valueIndex    0

    constructor args {
        $self configurelist $args

        # The label is big enough to hold the largest value:

        set width 1

        foreach value $options(-values) {
            set length [string length $value]
            if {$length > $width} {
                set width $length
            }
        }


        ArrowButton $win.inc    -dir top -command [mymethod next]
        label       $win.value  -width $width  -text {}
        ArrowButton $win.dec    -dir bottom -command [mymethod previous]

        pack $win.inc $win.value $win.dec -anchor w

        $self setValue

    }
    ####
    #  next  - Advance to next element in the list.
    #
    method next {} {
        incr valueIndex
        if {$valueIndex >= [llength $options(-values)]} {
            set valueIndex 0
        }
        $self setValue
    }
    ####
    #  previous - drop back to the previous value.
    #
    method previous {} {
        incr valueIndex -1
        if {$valueIndex < 0} {
            set valueIndex [llength $options(-values)]
            incr valueIndex -1
        }
        $self setValue
    }
    #####
    #   setValue - Set the text of the label to the value indicated
    #              by valueIndex
    #
    method setValue {} {
        set value [lindex $options(-values) $valueIndex]
        $win.value configure -text $value
    }
    #####
    #   get   - Return the current value of the spinbox
    #
    method get {} {
        return [$win.value cget -text]
    }
    ######
    # configure -state normal|disabled|active
    #     Set the state of the widgets.
    #
    onconfigure -state state {
        foreach widget [list $win.inc $win.dec $win.value] {
            $widget configure -state $state
        }
        set options(-state) $state
    }
}
#------------------------------------------------------------------------
#  bitPattern
#     Presents a horizontal list of vertical spinboxes... one for each
#     bit containing the possible values 0,1,x.
# Options:
#    -bits   - Number of bits to provide selectors for.
# Methods:
#    get     - Gets the current bit pattern mask string.
#
snit::widget bitPattern {
    option  -bits 8
    option  -state normal

    constructor args {
        $self configurelist $args

        for {set bit [expr {$options(-bits) -1}]} {$bit >= 0} {incr bit -1} {
            lappend packlist [verticalSpinBox $win.bit$bit -values {x 0 1}]
            if {(($bit % 4) == 0) && ($bit != 0)} {
                lappend packlist [label $win.sep$bit -text {} -width 1]
            }
        }
        eval grid $packlist

    }
    #    Control the state of each subwidget.
    #    Values are enforced all the way down by the primitive widgets.
    #    These can be:  normal, disabled, or active.
    # Parameter:
    #  which  - Desired state.
    #
    onconfigure -state which {

        for {set i 0} {$i < $options(-bits)} {incr i} {
            $win.bit$i configure -state $which
        }
        set options(-state) $which
    }
    ####
    # get
    #    Gets the pattern mask
    #
    method get {} {
        for {set bit [expr {$options(-bits) -1}]} {$bit >= 0} {incr bit -1} {
            append mask [$win.bit$bit get]
        }
        return $mask
    }
}
#-------------------------------------------------------------------------
# filterSetupDialog
#    This dialog provides the user with a way to describe filter patterns
#    Filter patterns are pattern of data that are of interest to a user.
#    Filters can be buffer specific, and are only matched in the body of the
#    buffer.
#
#   This dialog is rather complicated and depends on the bitPattern
#   widget above:
#
#   +-----------------------------------------------------------------+
#   |   +--------------+              +--------------+                |
#   |   | Buffer type  |      [>]     | Selected     |                |
#   |   | list         |      [<]     | buffer types |                |
#   |   +--------------+              +--------------+                |
#   |   [ ] Filter on buffer types                                    |
#   +-----------------------------------------------------------------+
#   |                                  Data Patterns                  |
#   |                                  +--------------------+         |
#   |   bit pattern                    |                    |         |
#   |  <bitPattern -bits 16>   [>]     |                    | [^]     |
#   |                                  |                    | [V]     |
#   |                                  |                    |[remove] |
#   |                                  +--------------------+         |
#   |    ( ) Any pattern     ( ) Sequence of matches                  |
#   +-----------------------------------------------------------------+
#   |   [Ok]      [Clear]     [Cancel]                                |
#   +-----------------------------------------------------------------+
#
# Options:
#    -buffertypes     - List of buffer types to stock the buffer type list
#                       box with initially.
#    -buffertypesrequired
#                     - List of buffer types in the selected buffer types
#                       listbox.
#    -filteronbuffers - True if the user has selected filter on buffers.
#    -patternlist     - The list of current data match patterns.
#    -patternmatchtype- One of "Any" or "Sequence" indicating the state of
#                       the match radio buttons.
#    -command         - Script invoked when OK is clicked.
#    -clearcommand    - Script invoked when clear is clicked.
#    -cancelcommand   - Script invoked when cancel is clicked.
# Methods
#     modal     - Turns the dialog into amodla dialog.
#
#
snit::widget filterSetupDialog {
    hulltype toplevel

    option   -buffertypes          {}
    option   -buffertypesrequired  {}
    option   -filteronbuffers      0
    option   -patternlist          {}
    option   -patternmatchtype     any
    option   -command              {}
    option   -clearcommand         {}
    option   -cancelcommand        {}

    # Since some of the configuration items require
    # Widgets, processing options is done after the
    # dialog is laid out.
    #
    constructor args  {

        # Setup the dialog widgets and geometry:

        #  Frame with buffer matching stuff:

        set bufmatch [frame $win.bufmatch -relief groove -borderwidth 4]
        label $bufmatch.available  -text {Buffer Types}
        label $bufmatch.acceptlabel -text {Accepted Types}
        ::iwidgets::scrolledlistbox $bufmatch.buftypes -vscrollmode dynamic  \
                                                       -hscrollmode dynamic  \
                                                       -selectmode multiple
        ArrowButton $bufmatch.add   -dir right                          \
                                    -command [mymethod AddBufferTypes]
        ArrowButton $bufmatch.remove -dir left                          \
                                     -command [mymethod RemoveBufferTypes]
        ::iwidgets::scrolledlistbox $bufmatch.selbuftypes -vscrollmode dynamic \
                                                          -hscrollmode dynamic \
                                                          -selectmode multiple
        checkbutton $bufmatch.enable -variable [$self MyVar options(-filteronbuffers)] \
                                     -text {Enable Buffer Filtering}                   \
                                     -command [mymethod onFilterToggles]

        # Frame that composes matching patterns.

        set pattmatch [frame $win.pattmatch -relief groove -borderwidth 4]
        label       $pattmatch.bplabel -text {Bit match pattern}
        bitPattern  $pattmatch.composer -bits 16
        ArrowButton $pattmatch.add     -dir right                           \
                                       -command [mymethod PatternToList]
        ::iwidgets::scrolledlistbox $pattmatch.patterns -vscrollmode dynamic \
                                                        -hscrollmode dynamic \
                                                        -selectmode single
        ArrowButton $pattmatch.up      -dir top                             \
                                       -command [mymethod PatternUp]
        ArrowButton $pattmatch.down    -dir bottom                          \
                                       -command [mymethod PatternDown]
        button      $pattmatch.remove  -text {Remove}                       \
                                       -command [mymethod RemovePattern]
        radiobutton $pattmatch.any     -text Any                            \
                                       -value any                           \
                                       -variable [$self MyVar options(-patternmatchtype)]
        radiobutton $pattmatch.seq     -text Sequence                       \
                                       -value sequence                      \
                                       -variable [$self MyVar options(-patternmatchtype)]

        # Action frame:

        set action [frame $win.action -relief groove -borderwidth 4]
        button $action.ok     -text Ok     -command [mymethod onOk]
        button $action.clear  -text Clear  -command [mymethod onClear]
        button $action.cancel -text Cancel -command [mymethod onCancel]


        # Layout the widget:  Each frame is gridded but pack is just fine
        # to stack the frames into the dialog.
        #
        grid $bufmatch.available       x         $bufmatch.acceptlabel
        grid $bufmatch.buftypes        x         $bufmatch.selbuftypes
        grid        ^            $bufmatch.add          ^
        grid        ^            $bufmatch.remove       ^
        grid  $bufmatch.enable         x                x


        grid        x                  x         $pattmatch.patterns       x
        grid  $pattmatch.bplabel       x                ^            $pattmatch.up
        grid  $pattmatch.composer $pattmatch.add        ^            $pattmatch.down
        grid        x                  x                ^            $pattmatch.remove
        grid  $pattmatch.any      $pattmatch.seq        x                  x

        pack $action.ok $action.clear $action.cancel -side left

        pack $bufmatch $pattmatch $action -side top -fill x -expand 1

        # Process configuration:

        $self configurelist $args

        # Set the state of the buffer type match controls:

        $self onFilterToggles
    }
    # Some of the options require dynamic configuration.
    # This set of methods deals with that.

    #####
    #   configure -buffertypes list
    #       Sets the buffertypes available in the buffertypelist list box.
    #       otherwise known as $win.bufmatch.buftypes.
    # Parameters:
    #    List of buffer types to add (in order to be added).
    # NOTE:
    #    Any existing buffer types in the list box are removed.
    #
    onconfigure -buffertypes list {
        bufdumpDialogs::setListBox $win.bufmatch.buftypes $list
    }
    #####
    #  cget -buffertypes
    #      Returns the list of widgets in the buffertypes listbox
    #
    oncget      -buffertypes {
        return [$win.bufmatch.buftypes get 0 end]
    }
    #####
    #  configure  -buffertypesrequired list
    #       Sets the contents of the buffer types requested listbox.
    # Parameters:
    #   list  - Set of values to load in the box.
    onconfigure -buffertypesrequired list {
        bufdumpDialogs::setListBox $win.bufmatch.selbuftypes $list
    }
    #####
    #  cget -buffertypesrequired
    #       Returns the contents of the buffer types requested listbox.
    #
    oncget      -buffertypesrequired {
        return [$win.bufmatch.selbuftypes get 0 end]
    }
    #####
    #  configure -patternlist list
    #       Sets the contents of the pattern list listbox.
    # Parameters
    #    list   - Values to load in the patternbox.
    #
    onconfigure -patternlist list {
        bufdumpDialogs::setListBox $win.pattmatch.patterns $list
    }
    ######
    #  cget -patternlist
    #       Returns the contents of the pattern list listbox.
    #
    oncget      -patternlist {
        return [$win.pattmatch.patterns get 0 end]
    }

    #  Public member.

    #####
    #  modal
    #     makes the dialog box modal and does not exit until either
    #     ok or cancel have been clicked.
    #
    method modal {} {
        frame $win.hidden

        focus        $win
        wm deiconify $win
        grab         $win

        tkwait window $win.hidden

        grab release $win
    }

    # 'protected' members

    #####
    # onOk
    #      Called in response to the ok button.  Modality is removed
    #      if set and the -command script is dispatched.
    #
    method onOk {} {
        $self endmodal
        $self dispatch -command
    }
    #####
    #  onClear
    #      Clears the widgets back to a default state as follows:
    #    -  All selected buffer types are removed from the
    #       selected buffer list box...and put back in the
    #       buffer type list box.
    #    -  The filter on buffer types toggle is set false
    #    -  The patterns are cleared from the pattern list box.
    #    -  The match type is set to any.
    #    - the clearcommand script is called.
    #
    method onClear {} {

        # Push the requested buffers back into the
        # available types.

        set bufs [$self cget -buffertypes]
        set req  [$self cget -buffertypesrequired]
        $self configure -buffertypes [concat $bufs $req]
        $self configure -buffertypesrequired [list]

        # Turn off the filter on buffer types checkbox.

        set options(-filteronbuffers) false
        $self onFilterToggles

        # clear the patterns list box:

        $self configure -patternlist [list]

        # set match type to any

        $self configure -patternmatchtype any

        $self dispatch -clearcommand

    }
    ####
    # onCancel
    #     Called in response to the cancel button being clicked.
    #     Modality is ended and the -cancelcommand script is called.
    #
    method onCancel {} {
        $self endmodal
        $self dispatch -cancelcommand
    }

    # 'private' members

    ####
    # dispatch option
    #     If $options(option) is not empty it is executed like a script.
    #
    method dispatch option {
        set script $options($option)
        if {$script ne [list]} {
            eval $script
        }
    }
    #####
    # endmodal
    #    End any modality of the widget.
    #
    method endmodal {} {
        if {[winfo exists $win.hidden]} {
            destroy $win.hidden
        }
    }
    #####
    #  PatternToList
    #     Adds the current bit pattern to the end of the current list of
    #     patterns.
    method PatternToList {} {
        set pattern [$win.pattmatch.composer get]
        $win.pattmatch.patterns insert end $pattern

    }
    #####
    # RemovePattern
    #     Removes the currentl selected pattern from the pattern list.
    # ASSUMPTIONS:
    #    We rely on the listbox being set to single selection mode.
    method RemovePattern {} {
        bufdumpDialogs::removeListboxSelection $win.pattmatch.patterns
    }
    method PatternUp {} {
        bufdumpDialogs::selectionUp $win.pattmatch.patterns
    }
    method PatternDown {} {
        bufdumpDialogs::selectionDown $win.pattmatch.patterns
    }
    ######
    # onFilterToggles
    #     The state of the buffer type filter widgets is set
    #     according to the state of the filter on buffertypes toggle.
    #     If not checked, everything is ghosted... otherwise enabled.
    #
    method onFilterToggles {} {
        if {$options(-filteronbuffers)} {
            set state normal
        } else {
            set state disabled
        }
        set widgets [list $win.bufmatch.buftypes             \
                          $win.bufmatch.add                  \
                          $win.bufmatch.remove               \
                          $win.bufmatch.selbuftypes]
        foreach widget $widgets {
            $widget configure -state $state
        }
    }
    ####
    # addBufferTypes:
    #     Adds all buffer types in the buftypes list box to the
    #     end of the selbuftypes listbox and removes them from the
    #    selbuftypes listbox.
    #
    method AddBufferTypes {} {
        bufdumpDialogs::moveSelection $win.bufmatch.buftypes $win.bufmatch.selbuftypes
    }
    #####
    # removebufferTypes
    # move the buffer types selected in the selbuftypes listbox to the
    # buftypes listbox.
    method RemoveBufferTypes {} {
        bufdumpDialogs::moveSelection $win.bufmatch.selbuftypes $win.bufmatch.buftypes
    }
    ####
    # MyVar name
    #     Fully qualifies a local variable name or option.
    #     needed because earlier versions of snit don't have this...
    #
    method MyVar name {
        append fqn $selfns :: $name
        return $fqn
    }
    #  Helper procs:


}
#------------------------------------------------------------------------------
#  searchDialog
#      This dialog prompts for search criteria.   The criteria can then be
#      fetched from the dialog.
#      Search criteria can by any of the following:
#        textual   - Which can be an exact match or a regular expression.
#        binary    - Which can be a set of bit patterns (including don't cares)
#                    or sequence of bit patterns.
# Visual Appearance:
#   +----------------------------------------------------------------------+
#   |        Search String [                               ]               |
#   |                       ( ) Exact    ( ) Regular expression            |
#   +----------------------------------------------------------------------+
#   |                                +--------------------------+          |
#   |    <bit pattern 16 bits>  >    |                          |  ^       |
#   |                                |                          |  V       |
#   |                                |                          | [delete] |
#   |                                +--------------------------+          |
#   |    ( ) match any   ( ) Match sequence
#   +----------------------------------------------------------------------+
#   |   ( ) Text search   ( ) Pattern Search                               |
#   +----------------------------------------------------------------------+
#   |   [ Ok ]    [ Cancel ]                                               |
#   +----------------------------------------------------------------------+
#
# Options:
#    -searchtext     - The textual search string
#    -searchtype     - A two element list that describes the search type.
#                      possibles are:
#                       text    exact    - Search for an exact text match
#                       text    regexp   - Search for a regular expression  match
#                       binary  any      - Search for a match with any of the
#                                          binary patterns.
#                       binary  sequence - Search for a match on a sequence of
#                                          binary patterns.
#   -patterns        - Get/set the bit pattern list.
#   -command         - Invoked when ok is clicked.
#   -cancelcommand   - Invoked when cancel is clicked.
#
# Methods:
#    modal           - Sets the dialog to be application modal.
#
snit::widget searchDialog {
    hulltype toplevel
    option -searchtext    {}
    option -searchtype    {}
    option -patterns      {}
    option -command       {}
    option -cancelcommand {}

    variable exactText    0;        # Exact/regexp radio button group value.
    variable sequenceBits 0;        # Any or sequential bit matching.
    variable searchType   0;        # text/pattern radio button group value.


    ####
    #  SelfVar   - Returns the fully qualified name of a variable
    #              required because some versions of snit out there
    #              don't implment this built in.
    #
    method SelfVar var {
        append name $selfns :: $var
        return $name
    }
    constructor args {
        set text   [frame $win.textmatching  -relief groove -borderwidth 3]
        set bits   [frame $win.bitmatching   -relief groove -borderwidth 3]
        set type   [frame $win.searchtype    -relief groove -borderwidth 3]
        set action [frame $win.action        -relief groove -borderwidth 3]

        #   Construct the text matching frame and lay it out.

        label $text.label -text {Search String}
        entry $text.text     -width 32
        radiobutton $text.exact  -text Exact                 \
                                 -variable [$self SelfVar exactText]   \
                                 -value 0
        radiobutton $text.regexp -text {Regular Expression}             \
                                 -variable [$self SelfVar exactText]    \
                                 -value 1
        grid  $text.label $text.text   - -
        grid       x      $text.exact  $text.regexp

        #   Construct the bit maatching frame and lay it out.

        ::iwidgets::scrolledlistbox $bits.patterns  -vscrollmode dynamic \
                                                    -hscrollmode dynamic \
                                                    -selectmode single
        bitPattern  $bits.composer -bits 16
        ArrowButton $bits.add      -dir right                            \
                                   -command [mymethod AddPattern]
        ArrowButton $bits.up       -dir top                               \
                                   -command [list bufdumpDialogs::selectionUp $bits.patterns]
        ArrowButton $bits.down     -dir bottom                            \
                                   -command [list bufdumpDialogs::selectionDown $bits.patterns]
        button      $bits.remove   -text Remove                           \
                                   -command [list bufdumpDialogs::removeListboxSelection $bits.patterns]
        radiobutton $bits.any      -text {Match Any}                      \
                                   -variable [$self SelfVar sequenceBits] \
                                   -value 0
        radiobutton $bits.seq      -text {Match Sequence}                 \
                                   -variable [$self SelfVar sequenceBits] \
                                   -value 1

        grid        x           x         $bits.patterns
        grid        x           x               ^          $bits.up
        grid  $bits.composer $bits.add          ^          $bits.down
        grid        x           x               ^          $bits.remove
        grid        x           x         $bits.any        $bits.seq

        #  Construct and layout the search type selection frame.
        #

        radiobutton $type.text      -text {Text Search}                   \
                                    -value 0                              \
                                    -variable [$self SelfVar searchType]  \
                                    -command [mymethod EnableSearchWidgets]
        radiobutton $type.pattern   -text {Bit Pattern Search}            \
                                    -value 1                              \
                                    -variable [$self SelfVar searchType]  \
                                    -command  [mymethod EnableSearchWidgets]

        grid $type.text    x       $type.pattern     x

        # Create and layout the action part of the dialog.

        button $action.ok           -text Ok       -command [mymethod OnOk]
        button $action.cancel       -text Cancel   -command [mymethod OnCancel]

        grid $action.ok $action.cancel    x     x

        #  Layout the frames in the dialog top to bottom filling x.

        pack $text   -side top -fill x
        pack $bits   -side top -fill x
        pack $type   -side top -fill x
        pack $action -side top -fill x

        $self configurelist $args
        $self EnableSearchWidgets
    }
    #######
    # configure -searchtext  <text>
    #       Configure the search text.  This is loaded into the search text entry widget.
    #
    onconfigure -searchtext text {
        $win.textmatching.text delete 0 end
        $win.textmatching.text insert end $text
    }
    #######
    # cget -searchtext
    #       Get the value of the search text from the widget.
    #
    oncget -searchtext {
        return [$win.textmatching.text get]
    }
    #######
    # configure -patterns <list of patterns>
    #     Provides a list of patterns for the pattern list box.
    #
    onconfigure -patterns list {
        bufdumpDialogs::setListBox $win.bitmatching.patterns $list
    }
    ########
    #  cget -patterns
    #      Returns the list of patterns in the pattern listbox.]
    #
    oncget -patterns {
        return [$win.bitmatching.patterns get 0 end]
    }
    #######
    #  configure -searchtype [list which how]
    #      Configure the search type radio buttons.
    #      which and how determine which buttons are
    #      which can be either text or binary and
    #      determines the state of the type radiobutton
    #      how's value is dependent on the value of which:
    #      text   - how can be either exact or regexp and
    #               determines how the text type radio buttons are set.
    #      binary - Can be either any or sequence and determines how the
    #               bit pattern matching radio buttons are set.
    #
    onconfigure -searchtype choices {
        set how    [lindex $choices 0]
        set which  [lindex $choices     1]

        switch -exact -- $how {
            text {
                set searchType 0
                switch -exact -- $which  {
                    exact {
                        set exactText 0
                    }
                    regexp {
                        set exactText 1
                    }
                    default {
                        error [list configure -searchtype which for text must be exact or regexp was $which]
                    }
                }
            }
            binary {
                set searchType 1
                switch -exact -- $which {
                    any {
                        set sequenceBits 0
                    }
                    sequence {
                        set sequenceBits 1
                    }
                    default {
                        error [list configure -searchtype which for binary must be any or sequence was $which]
                    }
                }
            }
            default {
                error [list configure -searchtype how must be text or binary and is $how]
            }
        }
        $self EnableSearchWidgets
    }
    ######
    # cget -searchtype
    #       Returns the two element list that describes the search type.
    #
    oncget -searchtype {
        if {$searchType == 0} {
            set how {text}
            if {$exactText} {
                set which {regexp}
            } else {
                set which {exact}
            }
        } else {
            set how {binary}
            if {$sequenceBits} {
                set which {sequence}
            } else {
                set which {any}
            }
        }
        return [list $how $which]
    }
    #######
    #   modal
    #       Make the dialog application modal.
    #
    method modal {} {
        if {![winfo exists $win.modalframe]} {
            frame $win.modalframe
            wm deiconify $win
            focus -force $win
            set window [$self SelfVar $win]
            after 10 [list grab set $win]

            tkwait window $win.modalframe
            grab release $win
        }
    }
    ######
    #  EndModal
    #      IF the dialog is modal, end the modality.
    #
    method EndModal {} {
        if {[winfo exists $win.modalframe]} {
            destroy $win.modalframe
        }
    }
    #######
    #  AddPattern
    #      This method adds the current bit pattern in the bit pattern editor
    #      to the list of patterns
    #
    method AddPattern {} {
      set pattern [$win.bitmatching.composer get]
      $win.bitmatching.patterns insert end $pattern
    }
    #######
    # Dispatch switch
    #       This method dispatches a script that is stored in an option
    #       if the option is empty, no dispatch is done.
    #
    method Dispatch switch {
        set script $options($switch)
        if {$script ne [list]} {
            eval $script
        }
    }
    #######
    # EnableSearchWidgets
    #    Based on the state of the searchType variable, enable/disable the appropriate
    #    widget sets.
    # Implicit Inputs:
    #   searchType   - 0 if this is a text search,
    #                  1 if this is a bitpattern search.
    #
    method EnableSearchWidgets {} {
        #
        #  Figure out the state of each of the widgets sets.
        #   normal means the user can interact
        #   disabled means ghosted.
        #
        if {$searchType == 0} {
            set textstate {normal}
            set bitstate  {disabled}
        } else {
            set textstate {disabled}
            set bitstate  {normal}
        }
        #  Configure the text search widgets:
        #
        set text $win.textmatching

        foreach widget [list $text.text $text.exact $text.regexp] {
            $widget configure -state $textstate
        }
        #  Configure the bit pattern search widgets:
        #
        set bits $win.bitmatching
        foreach widget [list $bits.patterns $bits.composer $bits.add    \
                             $bits.up       $bits.down     $bits.remove \
                             $bits.any      $bits.seq] {
            $widget configure -state $bitstate
        }

    }
    #######
    # OnOk
    #     Ok was clicked.  end modality and dispatch the -command script
    #
    method OnOk {} {
        $self EndModal
        $self Dispatch -command
    }
    #####
    # OnCancel
    #     Cancel was clicked.  end modality and dispatch the -cancelcommand script.
    #
    method OnCancel {} {
        $self EndModal
        $self Dispatch -cancelcommand
    }
}
