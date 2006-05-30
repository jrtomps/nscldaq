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

#   This package provides the non-dialog widgets used
#   by bufdump.
#
# Add the canonicalized path to the script to the front of auto_path.

set here [file dirname [info script]]
set wd [pwd]
cd $here
set here [pwd]
cd $wd

if {[lsearch -exact $auto_path $here] == -1} {
    set auto_path [concat $here $auto_path]
}

package provide bufdumpWidgets 1.0
package require snit
package require BWidget
package require eventData




namespace eval bufdumpWidgets {
}
#----------------------------------------------------------------------
# bufdumpWidgets::formatHex list
#      Formats the numbers in list as a set of hex numbers
#      8 to a line.
# Parameters:
#    list   - list of values to format
#
proc bufdumpWidgets::formatHex list {
    set item 0
    set result ""
    foreach word $list {
        set word [expr $word & 0xffff]
        append result [format "0x%04x " $word]
        if {($item % 8) == 7} {
            append  result "\n"
        }
        incr item

    }
    return $result
}
#----------------------------------------------------------------------
#  bufdumpWidgets::AppendText widget text
#      Appends text to a text widget... the widget may or may not be
#      disabled.  Returns a count of the number of lines of text
#      appended by counting the \n chars.
# Parameters
#  widget    - the widget to which the data are appended.
#  text      - the text to append.
# Returns:
#    Number of lines of data appended to the widget.
#
proc bufdumpWidgets::AppendText {widget text} {

    # Append the text.

    set state [$widget cget -state]
    $widget configure -state normal
    $widget insert end $text
    $widget configure -state $state

    # Count lines:

    set ltext [split $text "\n"]
    set lines [llength $ltext]
    incr lines -1
    return $lines
}
#----------------------------------------------------------------------
# bufdumpWidgets::DeleteFront widget endIndex
#      Deletes lines from the front of a widget.  The widget may be
#      disabled.
# Parameters:
#   widget   - The widget to delete from.
#   endIndex - The index that describes where to delete to.
#
proc bufdumpWidgets::DeleteFront {widget endIndex} {
    set state [$widget cget -state]
    $widget configure -state normal

    $widget delete 0.0 $endIndex

    $widget configure -state $state
}
#----------------------------------------------------------------------
# bufdumpWidgets::SecondsToElapsed secs
#    Converts time in seconds to an elapsed time of the form
#    d-hh:mm:ss
# Parameters:
#   secs   - Number of seconds to convert.
proc bufdumpWidgets::SecondsToElapsed secs {
    set seconds [expr $secs % 60]
    set secs    [expr $secs / 60]
    set minutes [expr $secs % 60]
    set secs    [expr $secs / 60]
    set hours   [expr $secs % 24]
    set days    [expr $secs / 24]

    return [format "%d-%02d:%02d:%02d" $days $hours $minutes $seconds]
}
#----------------------------------------------------------------------
# unformattedDump
#      This widget provides an unformatted dump of event data.
#      This is basically a text widget with associated scrollbars
#      for vertical and, if necessary horizontal formatting.
#
# Options:
#    Delegated to text widget:
#      -height      - Height of the text widget.
#      -width       - width of the text widget.
#    Locally implemented:
#      -radix       - Choses the output radix... can be one of:
#                      decimal     - base 10.
#                      hex         - base 16.
#                      octal       - base 8.
#      -maxlines    - Maximum number of lines that can be put in the text widget
#                     without some lines being scrolled off the top.
# Member functions:
#
#    append      data    - Append data.  Data is a list of word numerical data.
#    appendText  string  - Append literal string to the widget.
#    clear               - Clear the widget.
#
#
#
snit::widget unformattedDump {

    option -radix     hex
    option -maxlines  100000
    delegate option -width  to  text
    delegate option -height to text
    delegate option -auto   to swin
    delegate option * to hull

    variable formats;              # For each format, the format string for each word.
    variable scanformats
    variable charsPerWord;         # For each format chars required per word.
    variable lines          0;     # Num lines of text in widget.

    variable marknum        0;     # Mark used to keep track of where the dumped stuff is.

    constructor  args {
        install swin using   ScrolledWindow $win.frame
        install text using   text           $win.frame.text -wrap none \
                                                            -state disabled \
                                                            -font "Terminal -12"
        $win.frame setwidget                $win.frame.text

        $self configurelist $args

        pack $win.frame -fill both -expand 1

        # The formats array contains format strings for
        # each of the radix types supported:
        #
        set formats(hex)     "%04x "
        set formats(decimal) "%05u "
        set formats(octal)   "%06o "

        set scanformats(hex)     "%x"
        set scanformats(decimal) "%u"
        set scanformats(octal)   "%o"
        #
        #   The charsperWord array determines the number of char
        #   positions each word will take up as a function of the
        #   radix:
        #
        set charsPerWord(hex)     7
        set charsPerWord(decimal) 7
        set charsPerWord(octal)   7

    }
    ####
    #  configure -radix rad
    #       Ensure the radix is legal and if so set it...
    #       if not emit an error:
    # Parameters:
    #    rad   - The radix, must be an element of the formats array.
    # Errors:
    #     {Invalid Radix}  Radix is not valid.
    #
    onconfigure -radix rad {
        if {[array names formats $rad] == $rad} {
            set oldradix $options(-radix)
            set options(-radix) $rad
            if {$oldradix ne $rad} {
                $self Reformat $oldradix
            }
        } else {
            error [concat Invalid Radix valid values are [array names formats]]
        }
    }
    #####
    #  appendText string
    #      Append a text string to the end of the text widget.  The string
    #      is appended without any interpretation...however \n's are counted
    #      in order to keep the linecount accurate, and to ensure the
    #      linecount does not exceed -maxlines
    #      NOTE:  Wrapping is not considered when calculating the line count.
    #             since the text wrap mode is off and it's hard for the caller
    #             to turn it on without inspecting this code.
    # Parameters:
    #    string - The string to append.
    #    args   - A little piece of dirt to allow excess parameters... See
    #             reformat to understand why we resort to this dirty trick.
    #
    method appendText {string args} {
        $win.frame.text configure -state normal
        $win.frame.text insert end $string
        set lineList [split $string "\n"]

        set appendlines [bufdumpWidgets::AppendText $win.frame.text $string]
        incr lines $appendlines
        $self TrimContents
    }
    ######
    #  append list radix=decimal
    #      Append a list of numbers to the end of the text widget.
    #      The location of line feeds is automatically computed
    #      Line feeds are placed after the largest power of 2 items
    #      that fits in the width of the text widget.
    #      The width of each item is given by the charsPerWord Array
    #      A linefeed always leads the data.
    #      A linefeed is always placed at the end of the final item.
    #      If necessary the text widget is trimmed.
    #      Unique marks are generated and inserted prior and after the
    #      data so that dumped data can be found and reformatted
    #      as needed.
    # Parameters:
    #  list  - A list of numbers to append to the widget.
    #  radix - The radix in which the input data is represented
    #          (defaults to hex)
    #
    method append {list {radix hex}} {
        # Figure out how many fit on a line:

        set itemSize       $charsPerWord($options(-radix))
        set widgetWidth    [$self cget -width]
        set items          [expr {int($widgetWidth/$itemSize)}]
        if {$items <=0} {
            set items 1
        }
        set log2Items      [expr {int(log($items)/log(2))}]
        set itemsPerLine   [expr {int(pow(2,$log2Items))}]

        # Now build up the string to output:

        set scanstring     $scanformats($radix)
        set formatString   $formats($options(-radix))
        set output ""
        set i 0
        foreach item $list {
            if {($i % $itemsPerLine) == 0} {
                append output "\n"
            }
            scan $item $scanstring datum
            append output [format $formatString $datum]
            incr i
        }
        append output "\n"

        set viewpoint m$marknum

        $win.frame.text configure -state normal
        $win.frame.text mark set m$marknum [list end - 1 chars]
        $win.frame.text mark gravity m$marknum left
        incr marknum
        $self appendText $output
        $win.frame.text mark set m$marknum [list end - 1 chars]
        $win.frame.text mark gravity m$marknum left

        incr marknum

        $win.frame.text see $viewpoint
        $win.frame.text configure -state disabled
    }
    #####
    #  clear
    #      empty the widget.
    #
    method clear {} {
        bufdumpWidgets::DeleteFront $win.frame.text end

        $win.frame.text configure -state normal
        $win.frame.text mark unset [$win.frame.text mark names]
        set marknum 0
        $win.frame.text configure -state disabled
        set lines 0
    }
    ######
    #  TrimContents
    #      If necessary, trim buffers off the top of the display.
    #      This is a buffer oriented dumper, so we can do this.
    #      If lines is too long then we trim from 0.0 to the first
    #      odd mark...deleting the first even mark and this odd mark
    #      until done.
    #
    #
    method TrimContents {} {
        set marks [$win.frame.text mark names]
        set marks [SortMarks $marks]
        while {($lines > $options(-maxlines)) && ([llength $marks] > 1)} {
            set evenMark [lindex $marks 0]
            set oddMark  [lindex $marks 1]
            set marks    [lrange $marks 2 end]


            set clearEnd    [$win.frame.text index [list $oddMark lineend]]
            scan $clearEnd "%d.%d"  linesCleared junk


            bufdumpWidgets::DeleteFront $win.frame.text $oddMark

            $win.frame.text mark unset $evenMark $oddMark
            set lines [expr $lines - $linesCleared]
        }

    }
    ######
    # Reformat oldradix
    #      Reformats the dump based on the current radix.. This is normally
    #      done if the radix changes.
    #      The contents of the widget are dumped.
    #      The widget is cleared
    #      Text that is not between marks is appended with appendText
    #      Text that is preceded by a mark is appened with append.
    # Parameters:
    #   oldradix - prior radix.
    method Reformat {oldradix} {
        set data [$win.frame.text dump 1.0 end]
        set location [$win.frame.text yview]
        set top      [lindex $location 0]

        $self clear

        # This loop runs a state-machine with the following states:
        #     literal  - Any text is literal and is just appended as is
        #     chunk    - We're reassembling a buffer.
        #  Starting state is literal... marks of the form m...
        #  cause transition to chunk and out of chunk.
        #
        set state "literal"
        set chunk [list]
        foreach {key value index} $data {
            switch -exact -- $key {
                text {
                    if {$state eq "literal"} {
                        $self appendText $value
                    } else {
                        append chunk $value " "
                    }
                }
                mark {
                    if {[string index $value 0] eq "m"} {
                        if {($state eq "literal")} {
                            set state "chunk"
                        } else {
                            # Chunk finished.
                            $self append $chunk $oldradix
                            set chunk [list]
                            set state "literal"
                        }
                    }
                }
                default {
                }
            }
        }
        if {[string length $chunk] != 0} {
            $self append $chunk $oldradix
        }
        $win.frame.text yview moveto $top
    }
    ####
    # SortMarks  marklist
    #      Takes a set of marks.. removes the ones that don't mark the start/end of buffers
    #      and sorts them by location.. in doing this, we assume that the marks we want are of
    #      the form m0 m1 ...mnnn
    # Parameters:
    #   marklist   - The input list of marks
    # Returns:
    #   The list of sorted marks.
    #
    proc SortMarks marklist {

        # Filtered marks will have the standard non-buffer marks removed.
        #

        set filteredMarks [list]
        foreach mark $marklist {
            if {[string index $mark 0] eq "m"} {
                lappend filteredMarks $mark
            }
        }
        #  Now sort...they probably already are, but why take a chance.
        #
        return [lsort -dictionary -increasing $filteredMarks]
    }
}
#------------------------------------------------------------------------------
#
#  formattedDump
#       This widget is a text widget that produces  formatted
#       dump of an event buffer.   The shape of this dump depends on the
#       underlying buffer:
#           State Change Buffer:
#               Time Stamp : Run <number>  has <state change>,  dd hh:mm:ss into the run
#                          Title : <run title>
#          Scaler buffer:
#                     Scalers for period starting dd hh:mm:ss through dd hh:mm:ss in the run
#                        Scaler <n>     <value>  (Rate : <value>) ...
#          Run/State Variables:
#                    (Run | State) Variables:
#                           Name      Value
#          Packet Description buffer:
#                    Packets used in this run are:
#                       id (ShortName)  : <Long name>  Version <version> instantiated : timestamp
#          Event Buffer:
#                  Event buffer with n Events:
#                       Event 1:  size
#                               <Formatted dump of Events>
# Packets are formatted either as a leader that includes the packet id and name
# followed by a hex dump of the body, or as determined by a plugin if available.
# A plugin is just a script (proc usually) that is registered with the widget that
# is passed the body of the packet as a parameter.  The script returns a string
# That is the formatted representation of the packet that will be displayed.
# Plugins are registered via the registerPlugin method.
# Options:
#     -buffer      - The current buffer.
#     -radix       - The radix for displaying numbers.
#     -maxlines    - Maximum number of lines we will allow on the display.

snit::widget formattedDump {
    delegate method dump   to text
    delegate method get    to text
    delegate method mark   to text
    delegate method see    to text
    delegate method search to text
    delegate method tag    to text
    delegate method index  to text


    option -buffer {}
    option -radix  16
    option -maxlines 100000

    variable lines 0
    variable formats
    variable longformats
    variable currentBuffer
    variable currentEvent
    variable plugins

    constructor args {

        # Create the user interface and lay it out.

        install swin using   ScrolledWindow $win.frame
        install text using   text           $win.frame.text -wrap none \
                                                            -state disabled \
                                                            -font "Terminal -12"
        $win.frame setwidget                $win.frame.text
        pack $win.frame -fill both -expand 1

        # Set the formatting information:

        set formats(hex)     "%04x"
        set formats(octal)   "%06ox"
        set formats(decimal) "%05d"

        set longformats(hex)     "%08x"
        set longformats(octal)   "%011o"
        set longformats(decimal) "%010d"


        set currentBuffer [nsclBuffer %AUTO%]
        $currentBuffer addTypeName 1 {Event Data}
        $currentBuffer addTypeName 2 {Scaler}
        $currentBuffer addTypeName 3 {Snapshot Scaler}
        $currentBuffer addTypeName 4 {State variables}
        $currentBuffer addTypeName 5 {Run Variables}
        $currentBuffer addTypeName 6 {Packet Type definitions}
        $currentBuffer addTypeName 11 {Begin Run}
        $currentBuffer addTypeName 12 {End Run}
        $currentBuffer addTypeName 13 {Pause Run}
        $currentBuffer addTypeName 14 {Resume Run}

        set currentEvent  [Event %AUTO%]

        # Process options - which will display the buffer if it is not null.

        $self configurelist $args
    }
    #####
    # configure -buffer contents
    #      Replaces the current buffer with a new buffer.
    #      This results in appending a formatted buffer dump of the new
    #      buffer to the widget.
    #
    onconfigure -buffer contents {
        set options(-buffer) $contents
        $currentBuffer configure -buffer $contents
        $self formatBuffer
    }
    #####
    # clear
    #     Clear the text window.
    #
    method clear {} {
        bufdumpWidgets::DeleteFront $win.frame.text end
        set lines 0
    }
    #####
    #  formatControlBuffer
    #     format a control buffer.  currentBuffer is assumed to
    #     contain a control buffer.
    #
    method formatControlBuffer {} {
        set type    [$currentBuffer type]
        set run     [$currentBuffer run]
        set runTime [$currentBuffer runTime]
        set stamp   [$currentBuffer absoluteTime]
        set title   [$currentBuffer title]

        set     formatText "$stamp : $type occurred for run number $run.\n"
        append  formatText "                      [bufdumpWidgets::SecondsToElapsed $runTime] into the run.\n"
        append  formatText "                      Title: $title\n"

        $self AppendText $formatText
    }
    #####
    #  formatScalerBuffer
    #     Format a scaler buffer for the user.
    #
    method formatScalerBuffer {} {
        set type  [$currentBuffer type]
        set stime [$currentBuffer startTime]
        set etime [$currentBuffer endTime]
        set delta [expr ($etime - $stime)*1.0]
        set entities [$currentBuffer entities]
        #
        #  Don't allow / 0.0
        #
        if {$delta == 0.0} {
            set delta 1.0
        }

        set    formatText "$type Buffer for interval [bufdumpWidgets::SecondsToElapsed $stime] through [bufdumpWidgets::SecondsToElapsed $etime]\n"
        append formatText "$entities scalers follow:\n"
        append formatText "Scaler           Increment           Rate over interval\n"


        for {set scaler 0} {$scaler < $entities} {incr scaler} {
            set value [$currentBuffer getEntity $scaler]
	    set value [format %u $value]
            set rate [expr $value/$delta]
            append formatText [format "%6d           %9u           %f\n" $scaler $value $rate]
        }
        $self AppendText $formatText

    }
    #####
    #   formatTclVariables
    #      Formats a buffer that contains Tcl Variables.
    #      Tcl Variable buffers contain entities that consist of a 3 element list.
    #      The first element is the constant string "set"  The second element is the
    #      name of the variable.  The third element is the value of the variable.
    #
    method formatTclVariables {} {
        set type     [$currentBuffer type]
        set entities [$currentBuffer entities]

        set    formatString "$type buffer containing $entities variable values\n"
        append formatString "   Variable             Value\n"
        for {set i 0} {$i < $entities} {incr i} {
            set entity [$currentBuffer getEntity $i]
            set name  [lindex $entity 1]
            set value [lindex $entity 2]
            append formatString [format "   %16s     %s\n" $name $value]

        }
        $self AppendText $formatString


    }
    #####
    #   formatPackets
    #       Formats a buffer full of packet definitions.
    #
    method formatPackets {} {
        set type     [$currentBuffer type]
        set defCount [$currentBuffer entities]

        set    output "$type buffer containing $defCount packet descriptions\n"
        append output "Name      Id         Description                Version   Instantiated\n"
        for {set i 0} {$i < $defCount} {incr i} {
            set packet [$currentBuffer getEntity $i]
            set list   [split $packet ":"]
            set name   [lindex $list 0]
            set id     [lindex $list 1]
            set desc   [lindex $list 2]
            set vers   [lindex $list 3]
            set tsList [lrange $list 4 end]
            set time   [join $tsList ":"]
            append output [format "%-5s 0x%04x     %-28s %-6s  %s" $name $id $desc $vers $time]

            # Add new packet to the list of known packets...so
            # so we know how to decode them.

            $self addDocumentedPacket $name $id $desc $vers $time
        }
        $self AppendText $output
    }
    ###
    #  format1Event num contents
    #        Formats a single event for output.
    # Parameters:
    #    num      - Event number (part of the event header)
    #    contents - Contents of the event.. note that the first
    #               list element is the word count.... well first 2
    #               if the event uses 32 bit counts see below.
    #    size32   - True if the buffer uses 32 bit size counts.
    #
    method format1Event {num contents {size32 0}} {

	set eventSize   [llength $contents];    # Independent of size32 .
	$currentEvent configure -size32 $size32
        set result "Event: $num  Size: $eventSize :\n"
        set item 0
        $currentEvent configure -event $contents
        set words 1

        set packets [$currentEvent packetCount]
        for {set i 0} {$i < $packets} {incr i} {
            set packet [$currentEvent getPacket $i]
            set definition [lindex $packet 0]
            set body       [lindex $packet 1]
            set id [$definition getId]
            set name [$definition getDescription]
            incr words [expr [llength $body] + 2]
            append result "   Packet id $id : $name Body size: [llength $body] body: \n"
            set pluginIndex [format %x $id]
            if {[array names plugins $pluginIndex] eq $pluginIndex} {
                append result [eval $plugins($pluginIndex) [list $body]]
            } else {
                append result [bufdumpWidgets::formatHex $body] "\n"
            }
        }
        if {($words+1) < [llength $contents]} {
            set residual [lrange $contents $words end]
            append result "   Event trailer not part of any documented packets: \n"
            append result [bufdumpWidgets::formatHex $residual] "\n"
        }
        append result "\n"
        return $result


    }
    ####
    #  formatEvents
    #      Formats an event buffer.
    #
    method formatEvents {} {
        set type    [$currentBuffer type]
        set events  [$currentBuffer entities]
	set revlevel [$currentBuffer revLevel]
	if {$revlevel < 6} {
	    set size32 0
	} else {
	    set size32 1
	}

        set output  "$type buffer with $events events\n"

        for {set i 0} {$i < $events} {incr i} {
            set event [$currentBuffer getEntity $i]
            append output [$self format1Event $i $event $size32]
        }
        $self AppendText $output
    }
    #####
    #  formatBuffer
    #      Add a formatting of the buffer to the widget.
    #
    method formatBuffer {} {
        set typeid [$currentBuffer typeid]
        set here [$win.frame.text index end]

        # Formatting is based on buffer type
        #
        switch -exact -- $typeid {
            11 - 12 - 13 - 14 {
                $self formatControlBuffer
            }
            2 - 3 {
                $self formatScalerBuffer
            }
            4 - 5 {
                $self formatTclVariables
            }
            6 {
                $self formatPackets
            }
            1 {
                $self formatEvents
            }
            default {
                $self AppendText "Unrecognized buffer type [$currentBuffer type]\n"
            }

        }
        $win.frame.text see $here
    }
    ######
    # addDocumentedPacket name id description version stamp
    #        Adds a packet definition that was picked up from
    #        the buffer.  The definition is only added if
    #        there is no packet with a matching id defined already.
    # Parameters:
    #    name          - The short name of the packet.
    #    id            - The id of the packet.
    #    description   - The packet description (long comment).
    #    version       - The packet version.
    #    stamp         - The packet object creation time stamp.
    #
    method addDocumentedPacket {name id description version stamp} {
        set knownPackets [$currentEvent cget -packets]
        foreach packet $knownPackets {
            if {$id == [$packet getId]} {
                return
            }
        }
        set packet [packetDescription %AUTO% -packet $name:$id:$description:$version:$stamp]
        $self addPacketType $packet
    }


    ######
    # addPacketType description
    #     Add a packet type definition to the set understood by our event
    #     parser.
    # Parameters:
    #    description - a packet object to add to the list.
    #
    #
    method addPacketType description {
        set packets [$currentEvent cget -packets]
        lappend packets $description
        $currentEvent configure -packets $packets
    }
    ######
    # addBufferType id name
    #      Add a buffer type description to the buffer.
    # Parameters:
    #  id   - The id number of the buffer.
    #  name - The textual string to associate with the buffer type.
    #
    method addBufferType {id name} {
        $currentBuffer addTypeName $id $name
    }
    #######
    #  registerPlugin id script
    #       Register a plugin to handle a packet type.
    #       any existing plugin is replaced.  If script is empty,
    #       the plugin registration if, any  is removed.
    #
    #  Parameters:
    #    id      - Id of the packet to register for.
    #    script  - Script to register.
    #NOTES:
    # 1. Plugins are stored in the array plugins which is indexed
    #    by the hex representation of the id.
    # 2. Due to the way in which the program works,
    #    a plugin will only be effective if the packet for which it
    #    operates is known and defined.
    #
    method registerPlugin {id script} {
        set id [format %x $id]

        #  Destroy any existing registration if the script is empty.

        if {$script eq [list]} {
            if {[array names plugins $id] eq $id} {
                unset plugins($id)
            }
        } else {
            # Register the plugin.

            set plugins($id) $script
        }

    }

    #######
    # AppendText text
    #     Appends a line of text to the widget, and trims the contents
    #     if necessary.
    # Parameters:
    #    text  - The lines to append.
    #
    method AppendText txt {
        set textlines [bufdumpWidgets::AppendText $win.frame.text $txt]
        incr lines $textlines
        $self Trim
        $win.frame.text see end
    }
    ######
    # Trim
    #    Trim the text widget down to the desired number of lines.
    #
    method Trim {} {
        set maxLines $options(-maxlines)
        if {$lines > $maxLines} {
            # Must trim shave off the excess + 10%

            set excessLines [expr {$lines - $maxLines}]
            set excessLines [expr {int($lines*(1.1))}]
            bufdumpWidgets::DeleteFront $win.frame.text  $excessLines.0
            incr lines -$excessLines

        }
    }


}
