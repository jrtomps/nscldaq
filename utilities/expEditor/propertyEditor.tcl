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
# @file propertyEditor.tcl
# @brief Provide a property editor widget.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide propertyEditor 1.0
package require Tk
package require properties
package require DataSourceUI
package require BWidget
##
# @class propertyEditor
#
#   Provide a property editor for property lists.  The editor looks like this:
# /verbatim
#   +--------------------------------------------------------------------+
#   |                            Title                                   |
#   +--------------------------------------------------------------------+
#   |    Property                   |    Value                           |
#   +--------------------------------------------------------------------+
#   |  A propname                   |  [ Entry stocked with value ]      |
#   +--------------------------------------------------------------------+
#                               ....
#
# /endverbatim
#
# OPTIONS:
#    *  -title    - Title displayed at the top of the widget.
#    *  -proplist - Provides the property list to display.
#    *  -command  - Command fired when a property value changes.
#                   The script can take the following substitutions:
#                   %W   - The widget.
#                   %E   - Entry subwidget that is the editor for the value.
#                   %L   - The property list.
#                   %P   - Property object
#                   %V   - Current propertly value.
#                   %N   - Value in the corresponding entry widget.
#
# @note Modifying the table does not modify the underlying property list if
#       -command won't do that.
# @note Entry contents will be validated against the property's validation
#
#
snit::widgetadaptor propertyEditor {
    option -title -default ""
    option -proplist -default "" -configuremethod _newPropertyList
    option -command -default [list]
    
    variable contents;    #Widget containing name/values.
    
    ##
    # constructor:
    #    Build the initial UI..that consists of a title and the column titles.
    #    The title is in a frame of it's own while the column titles are in a
    #    BWidget scrollable frame.  The actual
    #    property pairs are created by _rebuild which kills off any existing
    #    propertypair set and creates new ones based on the current property list.
    #
    constructor args {
        installhull using ttk::frame
        
        # Create fill and grid the title frame:
        
        ttk::frame $win.top
        ttk::label $win.top.title -textvariable [myvar options(-title)]
        grid $win.top.title
        grid $win.top  -sticky new
        
        #  Make the scrolled frame, insert a ttk::frame in it. Put the row
        #  titles at the top of it.
        
        ScrolledWindow $win.bottom
        ScrollableFrame $win.bottom.frame
        $win.bottom setwidget $win.bottom.frame
        set contents [$win.bottom.frame getframe];   # where the frame payload lives.
        
        ttk::label $contents.nametitle -text {Property Name} -relief solid -borderwidth 1
        ttk::label $contents.valtitle  -text {Value} -relief solid -borderwidth 1
        
        grid $contents.nametitle $contents.valtitle -sticky new
        grid $win.bottom  -stick nsew
        grid rowconfigure $win 1 -weight 1
        
        
        #  configure the widget - if there's a -proplist config/-title config
        #  the widget will get built.
        
        $self configurelist $args
    }
    ##
    # _newPropertyList
    #   configure a new property list:
    #   - Save the list.
    #   - Rebuild the widget's bottom.
    #
    # @param optname - name of the option being  configured.
    # @param value   - new value.
    #
    method _newPropertyList {optname value} {
        set options($optname) $value
        
        $self _rebuild
    }
    ##
    # _rebuild
    #   Rebuild the appearance of the bottom frame.   This means
    #   - Killing off child widgets of contents other than the title.
    #   - building new child widgets based on the property name/values in the
    #     configured list.
    #
    #  @note if an empty string is configured as the property list, the name/value
    #        pair area will be left empty.
    #
    method _rebuild {} {
    
        # Kill off the non title widgets:
        
        foreach w [winfo children $contents] {
            if {$w ni [list $contents.nametitle $contents.valtitle]} {
                destroy $w
            }
        }
        #  If the property list has been defined make new widgets:
        
        set i 0
        if {$options(-proplist) ne ""} {
            foreach property [$options(-proplist) get] {
                ttk::label $contents.n$i \
                    -text [$property cget -name] -relief solid -borderwidth 1
                
                if {[$property cget -editable]} {
                    set state normal
                } else {
                    set state disabled
                }
                ttk::entry $contents.v$i                                  \
                    -validate focusout                                    \
                    -validatecommand [mymethod _validate $property $contents.v$i]
                $contents.v$i insert end [$property cget -value]
                $contents.v$i configure -state  $state
                
                grid $contents.n$i $contents.v$i -sticky new
                
                incr i
            }
        }
    }
    ##
    #  _validate
    #     Called to validate a new entry.
    #     - invoke the property's validation.
    #     - if invalid restore the prior value _and_ pop up a dialog.
    #     - if valid invoke any -command script at the global call level.
    #
    #  @note - It is up to the -command script to decide if the property value
    #          is actually modified or not.  See the class comments for i
    #          the substitution strings that are allowed.
    #
    # @param prop - property associated with the modification.
    # @param e    - entry widget that holds the property value.
    #
    method _validate {prop e} {
        
        set v [$prop cget -validate]
        set proposed [$e get]
        
        # validate the object...no validator means anything goes.
        
        if {($v eq "") || ([catch {$v validate $proposed}] == 0)} {
            
            # valid proposed string.
            
            set script $options(-command)
            if {$script ne ""} {
                set script [string map  \
                    [list %W $win %E $e %L $options(-proplist) %P $prop \
                    %V [list "[$prop cget -value]"] %N [list "$proposed"]] $script]
                
                uplevel #0 $script
                
                
            }
            return 1
        } else {
            
            #invalid:
            
            tk_messageBox -icon error -parent $win -title {Invalid value} -type ok \
                -message "Invalid value '$proposed' for property [$prop cget -name]"
            
            $e delete 0 end
            $e insert end [$prop cget -value]
            
            return 0
        }
    }
}
##
# @class propertyDialog
#
#   Provides a dialog that contains a property editor.  The editor supports
#   Ok and Cancel.  On Ok, the underlying property list is updated from the
#   changes made in the editor while on Cancel, the property list remains
#   unchanged. Destroying the widget is the same as cancel.
#
# Options:
#  -proplist - supply the property list.
#  -title    -  Title on property list.
#  -wintitle - Title of the dialog window.
# METHODS:
#    modal - run the dialog as a modal dialog
#
#
snit::widgetadaptor propertyDialog {
    component editor
    component wrapper
    
    option -wintitle -configuremethod _setTitle
    delegate option -title to editor
    delegate option -proplist to editor
    
    variable changes -array [list]
    
    ##
    # constructor
    #   -   Hull is a top level.
    #   -   Make the dialog wrapper under the hull
    #   -   install/create the editor under the wrapper.
    #   -   Configure the megawidget.
    #
    constructor args {
        installhull using toplevel
        
        install wrapper using DialogWrapper $win.wrapper
        set controlArea [$win.wrapper controlarea]
        
        install editor using propertyEditor $controlArea.editor \
            -command [mymethod _maintainChanges %P %N]
        
        pack $editor -fill both -expand 1
        pack $wrapper        -fill both -expand 1
        
        $self configurelist $args
    }
    #-------------------------------------------------------------------------
    #  Internal (private) methods.
    
    ##
    # _setTitle
    #   Set the window title:
    #
    # @param optname - name of the option being sert (-wintitle).
    # @param value   - new value.
    #
    method _setTitle {optname value} {
        set options($optname) $value
        wm title $win $value
    }
    ##
    # _maintainChanges
    #   Called when a property has been changed in the editor.  We
    #   make an array element keyed by the property object with the new value.
    #
    # @param p  - property object.
    # @param v  - New value.
    #
    method _maintainChanges {p v} {
        set changes($p) $v
    }
    ##
    # _commitChanges
    #   Commit the changes array to the property list.
    #
    method _commitChanges {} {
        foreach property [array names changes] {
            $property configure -value $changes($property)
        }
        array unset changes  
    }
    #-------------------------------------------------------------------------
    #  modal
    #    Enter modal mode and handle the return value.
    #
    method modal {} {
        set result [$wrapper modal]
        
        if {$result eq "Ok"} {
            $self _commitChanges
        }
        return $result
    }
    
}
