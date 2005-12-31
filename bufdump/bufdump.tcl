#!/bin/sh
# start tclsh: \
    exec tclsh ${0} ${@}


# Note that tclsh is fine since we package require to get Tk.
#


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

#  bufdump.tcl
#      This file supports dumping buffers from online and offline event sources.
#
# Add the canonicalized path to the script to the front of auto_path.

set version 1.0


# Add the canonicalized path to the script to the front of auto_path.

set here [file dirname [info script]]
set wd [pwd]
cd $here
set here [pwd]
cd $wd

#  Prepend here to the front of auto-path so that
#  if we're not living in a starkit we can still
#  find all our toys.
#

if {[lsearch -exact $auto_path $here] == -1} {
    set auto_path [concat [file join $here .. lib]  $auto_path]
}




package require Tk
package require BWidget
package require Iwidgets
package require dataSources
package require bufdumpDialogs
package require bufdumpWidgets
package require eventData


#--------------------------------------------------------------------------
# Global data

#    The data source information.

set sourceObject [list]
set sourceType   [list]
set sourceName   [list]

# Information needed to filter buffers:


set filterBufferTypes     [list]
set filterOnBuffers 0
set patternList     [list]
set matchType        any
set knownBufferTypes [list {1 Event} {11 Begin} {12 End} {13 Pause} {14 Resume}  \
                           {2 Scaler} {3 Snapshot-Scaler}                        \
                           {4 State-variables} {5 Run-variables} {6 Packet-types}]


#  Information required to do searches:

set haveSearchPattern 0
set searchType        {text exact}
set searchPatterns    [list]
set searchText        [list]

#  Packets file:

# If we are in a starkit, our
# packet files are in $starkit::topdir/etc
# and helpfiles       $starkit::topdir/help
#   Otherwise they are ../etc ./help

if {[info exists starkit::topdir]} {
    set packetDefinitionFile [file join $starkit::topdir etc packets.def]
    set helpDirectory        [file join $starkit::topdir help]
} else {
    set packetDefinitionFile [file join $here .. etc packets.def]
    set helpDirectory [file join $here .. help]
}

# Help directory:





#-------------------------------------------------------------------------
# showAbout
#    Displays the about dialog box that shows all the versioning info.
#    etc.
#
proc showAbout {} {
    global version

    set    info      {bufdump - NSCL Buffer dumper}
    append info "\n" "Version: $version"
    append info "\n" {(c) 2005 NSCL/Michigan State University}
    append info "\n" {This software may be redistributed under}
    append info "\n" {the Gnu Public License}
    append info "\n" {See  http://www.gnu.org/licenses/gpl.txt}
    append info "\n" {for terms and conditions}
    append info "\n" {Author: Ron Fox}

    tk_messageBox -icon info -type ok -title {About} -message $info

}
#-------------------------------------------------------------------------
# helpTopics
#     If the hyper help widget is not already displays, displays it
#     and points the topic to the overview.
proc helpTopics {} {
    global helpDirectory
    set topics [list intro overview gui datasource plugins packets]
    if {![winfo exists .topics]} {
        ::iwidgets::hyperhelp .topics
        .topics configure    -topics $topics  \
                             -helpdir $helpDirectory

    }
    wm     deiconify .topics
    focus .topics
    .topics showtopic intro
}
#-------------------------------------------------------------------------
# listToMasks  patterns
#     Turns a set of filter patterns into  a list of mask match pattern
#     pairs.
# Parameters:
#    patterns  - a list of pattern strings.  Each string is 16 'bits' long.
#                where each bit can be 0, 1 or x where 0/1 specify exact
#                matching and x means don't care.
# Returns:
#     For each pattern a mask of the bits that matter and the pattern
#     with x's converted to 0's.
#
proc listToMasks patterns {

    set result [list]
    foreach pattern $patterns {
        set mask  0
        set match 0
        set bitnum 15
        for {set i 0} {$i < 16} {incr i} {
            set bit [string index $pattern $i]
            switch -exact -- $bit {
                1 - 0 {
                    set mask  [expr  {$mask | (1 << $bitnum)}]
                    set match [expr  {$match | ($bit << $bitnum)}]

                }
                x {
                }
                default {
                    error "Invalid 'bit' in pattern $pattern in $patterns"
                }
            }
            incr bitnum -1
        }
        lappend result [list $mask $match]
    }
    return $result
}
#-------------------------------------------------------------------------
# matchAny entity  patterns
#     Matches if an entity contains any of the bit patterns in patternList
# Parameters
#   entity   - the entity to match.
#   patterns - A list of 2 element sublists containing mask and match values
proc matchAny {entity patterns} {
    foreach word $entity {
        foreach pattern $patterns {
            set mask  [lindex $pattern 0]
            set match [lindex $pattern 1]
            if {($word & $mask) == $match} {
                return 1
            }
        }
    }
    return 0
}
#-------------------------------------------------------------------------
# matchSequence entity patterns
#
proc matchSequence {entity patterns} {
    set sequenceLength [llength $patterns]
    set entityLength   [llength $entity]

    #  Must have an entity at least as long as the
    #  sequence.
    #
    if {$sequenceLength > $entityLength} {
        return 0
    }
    set start 0
    set end   [expr {$sequenceLength - 1}]
    while {$end < $entityLength} {
        set candidate [lrange $entity $start $end]
        set failed 0
        foreach word $candidate pattern $patterns {
            set mask  [lindex $pattern 0]
            set match [lindex $pattern 1]
            if {($word & $mask) != $match} {
                set failed 1
                break
            }
        }
        if {!$failed} {
            return 1
        }
        incr start
        incr end
    }
    return 0
}

#-------------------------------------------------------------------------
# filterBuffer buffer
#      Determines if the buffer meets the user's buffer match criterion.
# Parameters:
#   buffer    - Buffer to test against the match.
# Returns:
#   0         - Buffer does not meet filter criterion
#   1         - Buffer does meet filter criteria.
#
proc filterBuffer buffer {\
    global filterOnBuffers
    global filterBufferTypes
    global patternList
    global matchType

    filter configure -buffer $buffer
    set type [filter type]
    if {$filterOnBuffers} {
        if {[lsearch -exact $filterBufferTypes $type] == -1} {
            return 0
        }
    }
    #  If there is no bit pattern, short circuit checking for matches.
    #
    if {($patternList eq "") || ($patternList eq "xxxxxxxxxxxxxxxx")} {
        return 1
    }
    set patternMasks [listToMasks $patternList]

    # Figure out which proc will be matching.


    if {$matchType eq "any"} {
        set matchMethod matchAny
    } else {
        set matchMethod matchSequence
    }
    #  Hunt for a match in the entities in the body of the buffer.
    #
    set entities [filter entities]
    for {set e 0} {$e < $entities} {incr e} {
        set entity [filter getRawEntity $e]
        if {[$matchMethod $entity $patternMasks]} {
            return 1
        }
    }

    return 0
}

#-------------------------------------------------------------------------
# readPacketFile fd
#     Read a packet definition file and provide its contents as
#     definitions to the .fdumper  A packet definition file
#     is an ordinary text file.  Each line of the file should be
#     a propery formatted Tcl list containing the following elements
#     for each packet type defined:
#       short_name id long_name version
# Parameters:
#   fd   - File descriptor open on the packet definition file.
#
proc readPacketFile fd {
    while {![eof $fd]} {
        gets $fd line
        set stamp [clock format [clock seconds]]
        if {[llength $line] == 4} {
            set short [lindex $line 0]
            set id    [lindex $line 1]
            set long  [lindex $line 2]
            set vers  [lindex $line 3]

            .fdumper addDocumentedPacket $short $id $long $vers $stamp
        }
    }
}
#-------------------------------------------------------------------------
# addPlugin
#      Prompts for and sources a file to add as a plugin for the script.
#      normally the source file will contain a tcl script that defines
#      and registers packet formatters, however in theory there's no
#      limit to what this script can do.. If this sounds a bit scary
#      good, it should
#
proc readPluginFile {} {
    set file [tk_getOpenFile   -defaultextension .tcl              \
                               -filetypes [list                    \
                                            [list "Plugin files" .plugin] \
                                            [list "Tcl scripts"  .tcl]    \
                                            [list "All Files"     *]]     \
                               -initialdir .]
    if {$file ne ""} {
        source $file
    }
}
#-------------------------------------------------------------------------
# readPacketDefinitions
#     Reads packet definitions and addes them to the buffer formatter.
#          addDocumentedPacket {name id description version stamp}
#
proc readPacketDefinitions {} {
    set file [tk_getOpenFile   -defaultextension .def                       \
                               -filetypes [list                             \
                                            [list "Definition files" .def]  \
                                            [list "Text files"       .txt]  \
                                            [list "All files"        *]]    \
                               -initialdir .]
    if {$file ne ""} {
        if {[catch {open $file r} fd] == 0} {
            readPacketFile $fd
            close $fd
        } else {
            tk_messageBox -icon error -title {Can't open file} -type ok \
                    -message "Unable to open $file : $fd"
        }
    }
}

#-------------------------------------------------------------------------
# openDataSource
#     Responds to the File->Open  by opening a data source.
#
proc openDataSource {} {
    global sourceObject
    global sourceType
    global sourceName


    dataSourceDialog .source  -cancelcommand [list destroy .source]
    .source modal


    if {[winfo exists .source]} {
        catch {theEvents destroy}
        dataSource theEvents
        set online [.source cget -online]
        set file   [.source cget -path]
        set host   [.source cget -host]
        set buffersize [.source cget -buffersize]
        destroy .source

        if {$online} {
            if {$host eq ""} {
                .controls.next configure -state disabled
                return
            }
            if {[catch {theEvents openOnline $host -buffersize $buffersize} msg] == 0} {
                set sourceObject theEvents
                set sourceType   online
                set sourceName   $host
                .controls.next configure -state normal
                .fdumper clear
            } else {
                tk_messageBox -icon error -type ok -title {Online open failed} \
                    -message "Failed to open online data source: $msg"
            }
        }  else {
            if {$file eq ""} {
                .controls.next configure -state disabled
                return
            }
            if {[catch {theEvents openOffline $file -buffersize $buffersize} msg] == 0} {
                set sourceObject theEvents
                set sourceType   offline
                set sourceName   $file
                .controls.next configure -state normal
                .fdumper clear
            } else {
                tk_messageBox -icon error -type ok -title {Online open failed} \
                    -message "Failed to open offline data source: $msg"
            }
        }
    }
}
#--------------------------------------------------------------------------
# getFilter
#     Get information about how to filter the data.
#
proc getFilter {} {
    global knownBufferTypes
    global filterBufferTypes
    global filterOnBuffers
    global patternList
    global matchType


    foreach type $knownBufferTypes {
        set btype [lindex $type 1]
        if {[lsearch -exact $filterBufferTypes $btype] == -1} {
            lappend buffers [lindex $type 1]
        }
    }
    filterSetupDialog .filter -buffertypes $buffers \
                              -buffertypesrequired $filterBufferTypes \
                              -filteronbuffers $filterOnBuffers      \
                              -patternlist     $patternList          \
                              -patternmatchtype $matchType          \
                              -cancelcommand [list destroy .filter]
    .filter modal

    if {[winfo exists .filter]} {
        set filterBufferTypes  [.filter cget -buffertypesrequired]
        set filterOnBuffers    [.filter cget -filteronbuffers]
        set patternList        [.filter cget -patternlist]
        set matchType          [.filter cget -patternmatchtype]
        destroy .filter
    }
}
#--------------------------------------------------------------------------
# searchText start text how
#      Searches the formatted text widget for some text.  The text
#     can be either be a pattern that must be exactly matched, or it can
#     be a regular expression.
# Parameters:
#     start     -  Text widget style index that says where to start searching.
#     text      -  Text to search for.
#     how       - Either 'exact' or 'regexp' to indicate how the text is
#                 to be interpreted.
# Returns:
#    A text widget index that is a suitable place for starting the next search.
#
proc searchText  {start text how} {

    # Remove any current tagging of the data

    .fdumper tag remove search 1.0 end

    append howswitch - $how
    set where [.fdumper search -forwards $howswitch -- $text $start end]

    if {$where ne [list]} {
        .fdumper see $where
        .fdumper tag add search [list $where linestart] [list $where lineend]
        .fdumper mark set search [list $where wordend]
        return {search}
    } else {
        tk_messageBox -icon info -title {Not Found} -type ok   \
            -message [list $how search for $text found no matches.]
        return $start
    }

}
#--------------------------------------------------------------------------
# searchAnyPattern text masks
#     Searches the integers in text for any occurence of the patterns in
#     masks.
# Parameters:
#   text   - the text to search.
#   masks  - A set of search patterns represented as a list of pairs.
#            the first item of each pair is a mask to and the data with.
#            the second the value the result of the and should give in order
#            to match.
# Returns:
#    -1    - None of the patterns matched any integers in text.
#    other - The string index of the first pattern match.
#
proc searchAnyPattern {text masks} {
    regsub -all "\n" $text " " text
    append text " "
    foreach word $text {
        if {[string is integer -strict  $word]} {
            foreach pattern $masks {
                set mask  [lindex $pattern 0]
                set value [lindex $pattern 1]
                if {($mask & $word) == $value} {
                    set start [string first " $word " $text]
                    incr start
                    return $start
                }
            }
        }
    }

    return -1
}
#--------------------------------------------------------------------------
# offsetToIndex  text base index
#     Converts a string index into a text string into a text widget
#     compatible index (line.column).
# Parameters:
#   text    - The text in which the index lives.
#   base    - A base text index relative to which this conversion is done.
#             this must be in line.col format.
#   index   - The index to convert.
# Returns:
#   a line.column string describing where the index is.
#
proc offsetToIndex {text base index} {
    append text "\n"
    set lineNumber 0
    set lineFeed [string first "\n" $text]

    # Figure out which line we're in..

    while {$index > $lineFeed} {
        if {$lineFeed < 0} exit
        set text [string range $text [incr lineFeed] end]
        incr index -$lineFeed
        incr lineNumber
        set lineFeed [string first "\n" $text]
    }
    # index is the offset into that line now.

    set base [split $base .]
    if {$lineNumber == 0} {
        incr index [lindex $base 1]
        return [lindex $base 0].$index
    } else {
        incr lineNumber [lindex $base 0]
        return $lineNumber.$index
    }


}
#--------------------------------------------------------------------------
# sequenceMatch words masks
#      Returns true if the words match the sequence pattern described by the
#      masks.
# Parameters
#  words   - The list of words that make up the candidate sequence.
#  masks   - The masks that define the bit sequence matching criteria.
#
# Returns:
#   0    - no match
#   1    - match.
#
proc sequenceMatch {words masks} {
    foreach word $words mask $masks {
        if {![string is integer -strict $word]} {
            return 0
        }
        set m [lindex $mask 0]
        set v [lindex $mask 1]
        if {($word & $m) != $v} {
            return 0
        }
    }
    return 1
}
#--------------------------------------------------------------------------
# searchSeqencePattern text masks
#       Search text for a set of consecutive integers that match
#       corresponding instances of the masks.
# Parameters:
#   text   - The text to search
#   masks  - The bitmasks as mask value pairs.
#
proc searchSequencePattern {text masks} {
    set patternLength [llength $masks]
    set textLength    [string length $text]
    set index         0
    while {$textLength > 0} {
        if {[regexp -indices "\\w" $text wordstart] != 0} {

            # Remove leading whitespace but keep track of it.

            set wordstart [lindex  $wordstart 0]
            incr index $wordstart
            set  text  [string range $text $wordstart end]
            incr textLength -$wordstart

            # See if the next patternLength 'words' match the pattern.

            set words [lrange $text 0 [expr {$patternLength -1}]]
            if {[sequenceMatch $words $masks]} {
                return $index
            }
            # If not .. start with the next word:

            set nextWord [string wordend $text 0]
            if {$nextWord == -1} {
                break
            }
            incr nextWord
            set text     [string range  $text $nextWord end]
            incr index $nextWord
            incr textLength -$nextWord

        } else {
            # No words left in the text so no match possible...

            break
        }

    }


    return -1
}
#--------------------------------------------------------------------------
# searchBitPatterns start patterns how
#     Searches the formatted text widget for a set of bit patterns.  The
#     patterns can be interpreted either as a set of patterns, any of which
#     can be matched or a sequence of patterns which must be located.
#     Each item in the pattern list is a binary number with bits that
#     have values 1, 0, or x indicating that that bit must be either
#     1, 0, or anything.
# Parameters:
#  start    - Text widget compatible index  form that indicates
#             where the search should start.
# patterns  - The list of bit patterns in the match criteria.
# how       - either "any" or "sequence" indicating how the pattern should be
#             interpreted.
# Returns:
#    a text widget index that is a suitable place for starting the next search.
#
proc searchBitPatterns {start patterns how} {

    # First thing we do is get the text from the start of search index to the end
    # of the text widget.

    set  text [.fdumper get $start end]

    # Remove any current tagging of the data

    .fdumper tag remove search 1.0 end


    set masks [listToMasks $patterns]
    if {$how eq "any"} {
        #
        # Search for a match with any of the bit patterns in all numeric 'words'
        # in the text.
        #
        set index [searchAnyPattern $text $masks]

    } else {
        set index [searchSequencePattern $text $masks]
    }
    #  If we found a match, we must convert the match location to a text index,
    #  tag it and mark the next search location etc.

    if {$index >= 0} {
        set start [.fdumper index $start]
        set textIndex [offsetToIndex $text $start $index]
        .fdumper see $textIndex
        .fdumper tag add search [list $textIndex] [list $textIndex lineend]
        .fdumper mark set search [list $textIndex wordend]
        return search
    } else {
        tk_messageBox   -icon info -type ok -title {Not Found} \
                        -message [list Unable to find $how of $patterns in remainder of buffer]
        return $start
    }


}
#--------------------------------------------------------------------------
# searchFirst
#      Prompt for search critera... and hunt the current set data in the
#      display for that criteria.
#
proc searchFirst {} {
    global haveSearchPattern
    global searchType
    global searchPatterns
    global searchText
    global searchStart

    searchDialog .search -cancelcommand [list destroy .search]
    if {$haveSearchPattern} {
        .search configure   -searchtype $searchType \
                            -patterns $searchPatterns  \
                            -searchtext $searchText
    }
    .search modal

    if {[winfo exists .search]} {
        set haveSearchPattern 1
        set searchType      [.search cget -searchtype]
        set searchPatterns  [.search cget -patterns]
        set searchText      [.search cget -searchtext]
        destroy .search
        set searchStart 1.0

        set type [lindex $searchType 0]
        set how  [lindex $searchType 1]

        if {$type eq "text"} {
            set searchStart [searchText $searchStart $searchText $how]
        } else {
            set searchStart [searchBitPatterns $searchStart $searchPatterns $how]
        }
    }


}
#--------------------------------------------------------------------------
# searchNext
#     Search for the next occurence of the current search criteria.
#
proc searchNext {} {
    global haveSearchPattern
    global searchType
    global searchPatterns
    global searchText
    global searchStart

    if {!$haveSearchPattern} {
        tk_messageBox -icon error -title {No pattern} -type ok     \
            -message {No initial search pattern has been set.
You must first use Filter->Search.. to establish one}
    } else {
        set type  [lindex $searchType 0]
        set how   [lindex $searchType 1]
        if {$type eq "text"} {
            set searchStart [searchText $searchStart $searchText $how]
        } else {
            set searchStart [searchBitPatterns $searchStart $searchPatterns $how]
        }
    }
}
#--------------------------------------------------------------------------
# GUISetup
#     Set up the user interface.
#
proc GUISetup {} {
    global radix
    set radix hex

    formattedDump   .fdumper
    .fdumper tag configure  search -foreground green -background black
    pack .fdumper -expand 1 -fill both


    frame .controls  -relief groove -borderwidth 4
    ArrowButton .controls.next -command nextBuffer \
                                -dir right -height 25 \
                                -state disabled -helptext {Next Buffer}

    pack .controls.next -side left


    frame  .status -relief flat -borderwidth 4
    label .status.type  -textvariable sourceType
    label .status.label -text " data Source: "
    label .status.name  -textvariable sourceName
    pack  .status.type .status.label .status.name -side left
    pack  .status .controls -side bottom  -anchor w -expand 0 -fill x



    menu .menu
    menu .menu.file -tearoff 0
    .menu.file add command -label {Open...}             -command openDataSource
    .menu.file add command -label {Read Packet defs...} -command readPacketDefinitions
    .menu.file add command -label {Add Plugin...}       -command readPluginFile
    .menu.file add separator
    .menu.file add command -label {Exit}             -command exit

    menu .menu.filter -tearoff 0
    .menu.filter add command -label {Filter...}        -command {getFilter}
    .menu.filter add command -label {Search...} -command {searchFirst} \
                             -accelerator {Ctrl-F}
    .menu.filter add command -label {Search Next}  -command searchNext \
                             -accelerator {F3}

    menu .menu.help -tearoff 0
    .menu.help add command -label {About...}   -command showAbout
    .menu.help add command -label {Topics...}  -command helpTopics \
                            -accelerator {F1}

    .menu add cascade -label {File}   -menu .menu.file
    .menu add cascade -label {Filter} -menu .menu.filter
    .menu add cascade -label {Help}   -menu .menu.help


    . configure -menu .menu


    # Application accelerator keys:

    bind . <F3>        searchNext
    bind . <Control-f> searchFirst

    bind . <F1>        helpTopics

}



#-----------------------------------------------------------------------
# showUnformattedBuffer widget data
#      Display a full data buffer in unformatted form.
#      The data are displayed 8 items/line in hex notation.
# Parameters:
#   widget - An unformatted dumper.
#   data   - The data buffer.
#
proc showUnformattedBuffer {widget data} {
    set radix [$widget cget -radix]
    $widget appendText "----------------------"
    $widget append $data
}
#----------------------------------------------------------------------
#   Called in response to the next button.  The next buffer is loaded
#   from the data source and dumped.
#
proc nextBuffer {} {
    global sourceObject
    global patternList
    global matchType
    global searchStart

    set here [.fdumper index end]
    while 1 {
        if {$sourceObject ne ""} {
            if {![catch {$sourceObject getNext} data]} {
                if {[filterBuffer $data]} {
                    .fdumper configure -buffer $data
                    #
                    #  If we filter on patterns, we are doing
                    #  an implied search to help the user
                    #  find the pattern in the buffer.
                    #
                    if {$patternList ne [list]} {
                        set searchStart [searchBitPatterns $here $patternList $matchType]
                    }
                    return
                }
            } else {
                tk_messageBox -title {Read Failure} -icon info -type ok \
                              -message [concat unable to read data source: $data]
                .controls.next configure -state disabled
                return
            }
        } else {
            return
        }
    }
}
#--------------------------------------------------------------------
# registerPlugin id script
#     Plugin the user can register to process a packet type in an
#     event buffer.
# Parameters:
#   id     - The id for which the formatter is registered.
#   script - The script to run for the packet.
#            The body of the packet is appended as a single parameter
#            to the end of the script... this implies that the script
#            should probably be a procedure call.
#            the script should return the formatted body text.
#
proc registerPacketFormatter {id script} {
    .fdumper registerPlugin $id $script
}


GUISetup

#   Read the system wide packet descriptions


if {[catch {open $packetDefinitionFile r} fd] == 0} {
    readPacketFile $fd
    close $fd
}
nsclBuffer filter
foreach type $knownBufferTypes {
    set id   [lindex $type 0]
    set text [lindex $type 1]
    filter addTypeName $id $text
}

