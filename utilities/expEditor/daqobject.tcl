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
# @file daqobject.tcl
# @brief Base functionality for an object in the daq  configuration editor.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide DaqObject 1.0
package require Tk
package require snit
##
# @class
#   Provides base functionality for an object in a daq configuration editor.
#   The base functionality allows an icon to be drawn on a canvas, moved or
#   duplicated.  When duplicated the object is unbound fromt he canvas of its
#   parent.  Duplication is intended to be used when dragging an object from
#   e.g. a toolbar to a working area.
#
#  OPTIONS:
#     -image   - Image of the object (created via e.g. image create).
#     -canvas  - Canvas on which the object is  supposed to be drawn.
#
#  METHODS:
#     drawat   - Draw the object at the specified position on a canvas.
#     moveto   - Move the object to a specified canvas position.
#     moveby   - Move the object by a specified delta.
#     addtag   - Add a tag to the object.
#     rmtag    - Remove a tag from an object.
#     tags     - Reports the tags associated with an object..
#     clone    - Return an object with this image that is not on a canvas.
#
snit::type DaqObject {
    option -image ""   -configuremethod _unbind
    option -canvas ""  -configuremethod _unbind
    
    # Current position.
    
    variable x ""
    variable y ""
    
    variable id "";                # Id of object on canvas.
    
    ##
    # destructor
    #
    #   If the image is associated with a canvas,.. it is destroyed as well:
    #
    destructor {
        if {$options(-canvas) ne "" } {
            $options(-canvas) delete $id
        }
    }
    #---------------------------------------------------------------------------
    # private methods
    #
    
    ##
    # _unbind
    #   If the image or the canvas change, then the item is deleted (if drawn)
    #   and id, x, y set to nothing again.
    #
    # @param optname - option name being configured that causes this,.
    # @param optval  - value of new option.
    #
    method _unbind {optname optval} {
        
        #  Figure out if we are already drawn:
        
        set c $options(-canvas)
        set i $options(-image)
        
        if {($c ne "") && ($i ne "") && ($id ne "")} {
            $c delete $id
            set options(-canvas) ""
            set x ""
            set y ""
            set id ""
        }
        set options($optname) $optval
        
    }
    #---------------------------------------------------------------------------
    #  Public methods
    
    ##
    # drawat
    #    Draw the object at a specific position.
    #    - Both canvas and image must be non empty.
    #    - If the object is visible, this is just a moveto.
    #    - If the object is not visible it is added to the canvas at the
    #      specified location.
    #
    #  An object is visible if it has a nonempty id.
    #
    # @param xx  - X Coordinate at which the upper left corner of the object's image 
    #             should be drawn.
    # @param yy  - Y Coordinate at which the upper left corner of the object's image
    #             should be drawn.
    #
    
    method drawat {xx yy} {
        set c $options(-canvas)
        set i $options(-image)
        
        if {($c eq "") || ($i eq "") } {
            error "To draw an image it must have a -canvas and a -image."
        }
        
        if {$id ne ""} {
            $self moveto $xx $yy
        } else {
            
            set id [$c create image  $xx $yy -image $i -anchor center]
            set x $xx
            set y $xx
        }
    }
    ##
    # moveto
    #   Move an object to a specified  position.
    #
    #   - The object must have an image.
    #   - The object must have a canvas.
    #   - The object must have an id (be visible).
    #
    # @param xx - New x coordinate of the nw corner of the image.
    # @param yy - New y coordinate of the nw corner of the image.
    #
    method moveto {xx yy} {
        set c $options(-canvas)
        set i $options(-image)
        
        
        if {($c eq "") || ($i eq "") || ($id eq "")} {
            error "Objects must exist and be visible to move"
        }
        
        $c coords $id $xx $yy
        set x $xx
        set y $yy
    }
    ##
    # moveby
    #   Move the image by some amount.
    #
    # @param dx  - Change in x
    # @param dy  - Change in y.
    #
    method moveby {dx dy} {
        set c $options(-canvas)
        set i $options(-image)
        
        if {($c eq"") || ($i eq "") || ($id eq "")} {
            error "Objects must exist and be visible to move"
        }
        
        $c move $id $dx $dy
        
        set newCoords [$c coords $id]
        set x [lindex $newCoords 0]
        set y [lindex $newCoords 1]
    }
    ##
    # addtag
    #   Add a tag to the object.
    #   - Object must be visible.
    #
    # @param tag - new tag to add.
    #
    # @note - It is a no-op to add an existing tag to the object.
    #
    method addtag tag {
        set c $options(-canvas)
        set i $options(-image)
        
        if {($c eq"") || ($i eq "") || ($id eq "")} {
            error "Objects must exist and be visible to move"
        }
        
        set tags [$c itemcget $id -tags]
        if {$tag ni $tags} {
            lappend tags $tag
        }
        $c itemconfigure $id -tags $tags
    }
    ##
    # rmtag
    #   Removes the specified tag from the object.
    #   - Item must exist and be visible.
    #   - If the tag is not associated with the object this is a no-op.
    #
    # @param tag - name of tag.
    #
    method rmtag tag {
        set c $options(-canvas)
        set i $options(-image)
        
        if {($c eq"") || ($i eq "") || ($id eq "")} {
            error "Objects must exist and be visible to move"
        }
        
        $c dtag $id $tag
    }
    ##
    # tags
    #   Returns the set of tags that are associated with the object.
    #   the object must exist and be visible
    #
    # @return list - associated tags.
    #
    method tags {} {
        set c $options(-canvas)
        set i $options(-image)
        
        if {($c eq"") || ($i eq "") || ($id eq "")} {
            error "Objects must exist and be visible to move"
        }
        
        return [$c itemcget $id -tags]
    }
    
    ##
    # clone
    #   Create/return a copy of ourself but:
    #   - Unbind from the canvas
    #   - lose positioning information.
    #
    # @return DaqObject
    #
    method clone {} {
        return [DaqObject %AUTO% -image $options(-image)]
    }
    
}