#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# @file LogView.tcl
# @brief Provide a view of log messages.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide LogView 1.0

package require Tk
package require snit

##
# @class LogView
#    Provides a view of log messages.  This is just a pretty dumb displayer
#    -  It's a scrolling treeview.
#    -  It provides a method to clear all treeview entries.
#    -  It provides a method to load a bunch of messages into the tree view.
#    -  It provides a method to add a message to the tree view.
#
#  Filtering is done by the controller (LogController in the MVC triad) in
#  conjunction with the filtering capabilities of the model (LogModel).
#
# OPTIONS
#    -colors - specifies the colors that various severity types are given.
#              this is of the form [dict create sev color ...]
#              The default foreground color of each entry is the foreground color
#              of the style.  Note that changes are dynamic.
#
snit::widgetadaptor LogView {
    component table
    
    option -colors -default [dict create] -configuremethod _configColors
    
    ##
    # constructor
    #    Construct the user interface.  Scroll bars will be added for both
    #    axes in case someone shrinks the widget down too much.
    #
    #
    constructor args {
        installhull using ttk::frame;           # So bg color is consistent.
        
        # Set up the table and its column headings/
        
        set colNames [list {Date Time} Severity Application Source Message]
        set widths   [list 200 100 200 200 500]
        install table using ttk::treeview $win.tree \
            -columns $colNames \
            -displaycolumns #all  -height 25 -selectmode none \
            -show headings \
            -yscrollcommand [list $win.vscroll set] \
            -xscrollcommand [list $win.hscroll set]
        
        foreach colname $colNames wid $widths {
            $table heading $colname -text $colname -anchor w
            $table column  $colname -stretch 1 -anchor w -width $wid
        }
            
        
        
        # Set the table up for vertical/horizontal scrolling
        
        ttk::scrollbar $win.vscroll -orient vertical \
            -command [list $win.tree yview]
        
        ttk::scrollbar $win.hscroll -orient horizontal \
            -command [list $win.tree xview]
        
        # Layout the widgets:
        
        grid $table $win.vscroll -sticky nsew
        grid $win.hscroll        -sticky sew
        
        grid columnconfigure $win 0 -weight 1
        grid rowconfigure    $win 0 -weight 1
       
        
        $self configurelist $args
    }
    #--------------------------------------------------------------------------
    # Private methods:
    #
    
    ##
    #  _configColors
    #     Configurea the colors for the severities.
    #
    # @param option - the -colors option.
    # @param value  - dict whose keys are severities and whose values are
    #                 the colors for the severity.
    #
    method _configColors {option value} {
        dict for {sev color} $value {
            $table tag configure $sev -foreground $color
        }
        set options($option) $value
    }
    
    #--------------------------------------------------------------------------
    # Public methods:
    #
    
    
    ##
    # add
    #   Adds a message to the tree view.  The message is added to the end
    #   of the table and, if necessary, made visible.
    #
    # @param message - dict containing the message.  The keys of this dict are:
    #                  - severity    - the message severity.
    #                  - application - The application that emitted the message.
    #                  - source      -  Source of the message (hostname).
    #                  - timestamp   -  [clock seconds] at which message was emitted.
    #                  - message     -  Messages.
    # @note - to support colorizing each item gets tagged with the severity
    #{Date Time} Severity Application Source Message]
    method add {message} {
        $table see [$table insert {} end \
            -tags [dict get $message severity]   \
            -values [list                                              \
                 [clock format [dict get $message timestamp] -format {%D %T}] \
                 [dict get $message severity] [dict get $message application]   \
                 [dict get $message source] [dict get $message message]         \
            ]]
    }
    ##
    # load
    #    Loads a bunch of messages into the table:
    #
    # @param messages - list of messages. See add for the format of a message.
    #
    method load messages {
        foreach message $messages} {
            $self add $message
        }
    }
    ##
    # clear
    #   Clear the widget of all messages. This might be done when
    #   one is about to apply a filter.  In that case the widget is first
    #   cleared and then the filtered data set loaded.
    #
    method clear {} {
        $table delete [$table children {}]
    }
}