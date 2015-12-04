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
# @file ObjectInstaller.tcl
# @brief Installer for object like DAQ objects.
#
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide objectInstaller 1.0
package require snit
package require properties
package require propertyEditor

##
# @class  ObjectInstaller
#
#   Provides installation from a toolbar to a canvas for DaqObject
#   related things:
#   - Puts the object on the destination canvas next to the tool (to its right).
#   - Enables dragging the object around.
#   - Enables double-click to bring up a property list for the object.
#
#    OPTIONS:
#     *  -installcmd  - script called when an object was installed. Substitutions:
#            %W  - The canvas being installed on.
#            %O  - The object that was installed.
#            %I  - the canvas ID associated with the object.
#     *  -deletecmd - Script invoked when an object is about to be deleted.
#           substitutions are the same as for -installcmd with the addition of
#           %I the id of the canvas object.  Note that the script
#           called can abort the delete by returning false and allow it to proceed
#           by returning true.
#
#   METHODS:
#      *   install  installs an object clone onto a canvas.
snit::type ObjectInstaller {
    option -installcmd [list]
    option -deletecmd  [list]
    
    #
    #  The array below maintains the object context menus for
    #  each canvas.  As objects are created, if there is not a canvas context
    #  menu one is created and added  to this array:
    
    variable contextMenus -array [dict create]
    variable objectContext ""
    #---------------------------------------------------------------------------
    # Private
    #
    
    ##
    # _drag
    #    Respond to an object drag by asking the object to draw itself elsewhere:
    #
    # @param object Object being dragged.
    # @param x      New x coordinate of object.
    # @param y      New y coordinate of object.
    #
    method _drag {object x y} {
        $object moveto $x $y
    }
    ##
    # _editProperties
    #
    #   Brings up the property editor for the object.
    #
    # @param object Object whose properties are being edited.
    #
    method _editProperties object {
        set props [$object getProperties]
        propertyDialog .p \
            -wintitle {Edit Properties} -title Properties: -proplist $props
        
        update idletasks
        
        .p modal
        destroy .p
    }
    ##
    # _dispatch
    #    Dispatch a script.
    #
    # @param optname  - name of the option containing the script,.
    # @param submap   - subst map.
    
    method _dispatch {optname submap} {
        set script $options($optname)
        if {$script ne ""} {
            set script [string map $submap $script]
            return [uplevel #1 $script]
        } else {
            return true;               # for -delcommand.
        }
    }
    ##
    # _delObject
    #   context menu handler to delete an object.
    #
    method _delObject {} {
        
        ##
        # TODO - provide a mechanism for connectors to kill themselves off if
        # they have one end attached to the object.
        set objProperties [$objectContext getProperties]
        set nameProp      [$objProperties find name]
        set objName       [$nameProp cget -value]
        set confirm [tk_messageBox                                \
            -title {Destroy object} -type yesno -default no -icon question \
            -message "Are you sure you want to delete '$objName'?"  \
        ]
        if {$confirm eq "yes"} {
            set confirm [$self _dispatch \
                 -deletecmd              \
                 [list %W [$objectContext cget -canvas] %O $objectContext \
                       %I [$objectContext getId]]]
            if {$confirm} {
                $objectContext destroy                       
            }
           
        }
        
        set objectContext ""
    }
    
    ##
    # _editProps
    #   context menu handler to edit the properties of an object.
    #
    method _editProps {} {
        $self _editProperties $objectContext
        set objectContext ""
    }
    ##
    # _getContextMenu
    #    Returns a context menu for the canvas.
    #    If necessary a new context menu is created.
    #
    # @param c   - canvas for which the context menu is being created.
    # @note the context menu will have the following commands:
    #        *  Delete...    - deletes this object.
    #        *  Properties.. - edit object properties.
    #
    method _getContextMenu {c} {
       
        if {[array names contextMenus $c] eq "" }    {
            set m [menu $c.objectContext -tearoff 0 -takefocus 1]
            $m add command -label Delete...     -command [mymethod _delObject ]
            $m add command -label Properties... -command [mymethod _editProps]
            
            set contextMenus($c) $m
        }
        return $contextMenus($c)
    }
    ##
    # _popupContextMenu
    #
    #  Pop up a context menu, setting our object context as well so the
    #  menu commands know what to operate on.
    #
    # @param menu   - menu widget id.
    # @param x,y    - Where in the root window to post the menu.
    # @param object - The object for which the menu is being posted.
    #
    method _popupContextMenu {menu x y object} {
        set objectContext $object
        tk_popup $menu $x $y
        
    }
    #-------------------------------------------------------------------------
    # Public methods:
    
    
    ##
    # install
    #  Install an object to a destination canvas.
    #  - Clone the object.
    #  - Draw it on the destination canvas
    #  - set up the event handling.
    #
    # @param object - object to clone/install
    # @param from   - dict containing canvas, x, y of tool.
    # @param to     - canvas to install the clone on.
    #
    # @return the newly created object.
    #
    method install {object from to} {
        set newObject [$object clone]
        
        
        #  put the object its width from the left edge at the height of the tool:
        
        set y [dict get $from y]
        set x [lindex [$object size] 0]

        $newObject configure -canvas $to
        $newObject drawat $x $y
        
        
        #  Now set up the object's behavior in the GUI.
        
        set id [$newObject getId]
        set ctxMenu [$self _getContextMenu $to] 
        
        $newObject bind <B1-Motion> [mymethod _drag $newObject %x %y ]; # Drag.
        $newObject bind <Button-3>  [mymethod \
            _popupContextMenu $ctxMenu %X %Y $newObject \
        ];                                                      # context menu.
        
        $self _dispatch -installcmd [list %W $to %O $newObject %I $id]
        
        return $newObject
    }
}