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
# @file connector.tcl
# @brief provide a connector class.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide connector 1.0
package require Tk
package require snit

##
# @class connector
#  A connector is a graphical object that connects two other graphical objects.
#  As such it has the following data:
#
#  *  fromId  - Id of the object where this connector originates.
#  *  toId    - Id of the object where this connector ends.
#  *  fromCoords - Attachment coordinates at the fromId.  This represents the
#                coordinates at which the line originates.
#  *  toCoords   - Attachment coordinates at the toId object.
#
#   tags are added to the from and to objects and <B1-Motion> events are added
#   so that if the from/to objects are dragged around the connector end points
#   move appropriately.
#
#  TODO:
#     This connector does not do a good job when the object is 'wrapped'
#     what really needs to happen is for graphical objects to have a
#     set of 'preferred' attachment points and
#     -  On creation the closest pair of attachment points are used.
#     -  On each move the closest pair of attachment point is chosen anew.
#
# OPTIONS
#    -from       - Id of the from object.
#    -to         - ID fo the to object.
#    -fromcoords - x/y pair defining the start of the line.
#    -tocoords   - x/y pair defining the end of the line.
#    -arrow      - Arrow specification of the line (none, first, last, both).
#    -canvas     - Canvas on wich the line is drawn (mandatory/readonly).

snit::type connector {
    option -from -readonly 1 -default ""
    option -to   -readonly 1 -default ""
    option -fromcoords -readonly 1 -default ""
    option -tocoords   -readonly 1 -default ""
    option -arrow -default last  -configuremethod _changeArrow
    option -canvas -readonly 1 -default ""
    
    variable id "";                 # Id of my line.
    variable priorX;                # To compute dx
    variable priorY;                # To computer dy.
    
    ##
    # constructor
    #    - Configure ourself.
    #    - Check that required configuration parameters have been provided.
    #    - Draw the connector.
    #    - Bind the events that let us adjust the connector endpoints when
    #      the object moves:
    constructor args {
        $self configurelist $args
        
        $self _checkLegalOptions
        $self _drawConnector
        
        # Take care of bindings -- We'll generate tags like fromId_from etc.
        #
        
        set c $options(-canvas)
        set f $options(-from)
        set t $options(-to)

        # This set of bindings assumes you can't move both at the same time.
        # This assumption is invalid if:
        # - -from is the same as -to
        # - We have a multi-touch sort of device.
        
        $c addtag t${id}_from withtag $f
        $c bind t${id}_from <Button-1>  [mymethod _startMove %x %y]
        $c bind t${id}_from <B1-Motion> [mymethod _moveFrom %x %y]
        
        $c addtag t${id}_to withtag $t
        $c bind t${id}_to <Button-1>  [mymethod _startMove %x %y]
        $c bind t${id}_to <B1-Motion> [mymethod _moveTo %x %y]
        
    }
    #--------------------------------------------------------------------------
    #  Private methods.
    
    ##
    #  _checkNonBlank
    #    Throw an error if the stated option is empty (blank).
    #
    # @param optname - name of the option to check.
    #
    method _checkNonBlank optname {
        if {$options($optname) eq ""} {
            error "$optname must be supplied at construction time."
        }
    }
    
    ##
    # _checkLegalOptions
    #
    #   Check that -canvas, -from, -to, -fromcoords and -tocoords are all not
    #   empty.
    #
    method _checkLegalOptions {} {
        $self _checkNonBlank -canvas
        $self _checkNonBlank -from
        $self _checkNonBlank -to
        $self _checkNonBlank -fromcoords
        $self _checkNonBlank -tocoords
    }
    ##
    # _drawConnector
    #   Initial draw of the connector.  The connector is drawn between fromCoords
    #   and toCoords with the arrow defined as in our -arrow option.
    #
    method _drawConnector {} {
        set c    $options(-canvas)
        set from $options(-fromcoords)
        set to   $options(-tocoords)
        
        set id [$c create line [concat $from $to] -arrow $options(-arrow)]
    }
    ##
    # _startMove
    #   Called when the left button drops.  The x/y coordinates of the mouse are
    #   saved so that if/when it moves we can compute the correct delta for the
    #   coordinates of the moved end of the line.
    #
    # @param x  - X mouse coordinate.
    # @param y  - Y mouse coordinate.
    #
    method _startMove {x y} {
        set priorX $x
        set priorY $y
    }
    ##
    # _moveCoords
    #   Using priorX/Y and x/y compute a new set of coordinates and
    #   -  Store them in priorX/Y
    #   -  Return them as [list x y].
    #
    # @param x - Current mouse x coord.
    # @param y - Current mouse y coord.
    # @param oldList - List of prior coordinates that must be modified.
    # @return newCoords - New coordinate values.
    #
    method _moveCoords {x y oldList} {
        set dx [expr {$x - $priorX}]
        set dy [expr {$y - $priorY}]
        
        set priorX $x
        set priorY $y
        
        #  Apply dx/dy to oldList:
        
        set ox [lindex $oldList 0]
        set oy [lindex $oldList 1]
        incr ox $dx
        incr oy $dy
        
        return [list $ox $oy]
    }
    ##
    # _updateCoords
    #   Update the coordinates of the line on the canvas.
    #
    method _updateCoords {} {
        set c $options(-canvas)
        set f $options(-fromcoords)
        set t $options(-tocoords)
        
        $c coords $id [concat $f $t]
    }
    
    ##
    # _moveFrom
    #    Move the from point of the line from where it is to the position whose
    #    deltas are determined by the current pointer position and priorX/priorY.
    #
    # @param x  - Current pointer x position.
    # @param y  - Current pointer y position.
    #
    method _moveFrom {x y} {

        set newCoords [$self _moveCoords $x $y $options(-fromcoords)]
        set options(-fromcoords) $newCoords
        $self _updateCoords
    }
    ##
    # _moveTo
    #   Same as _moveFrom but the to coords are modified.
    #
    method _moveTo {x y} {
        set newCoords [$self _moveCoords $x $y $options(-tocoords)]
        set options(-tocoords) $newCoords
        $self _updateCoords
    }
    
    ##
    # _changeArrow
    #   Change the location of the arrow.
    #
    # @param optname -option name holding the arrow position(s).
    # @param value   - new value for that option.
    #
    method _changeArrow {optname value} {
        $options(-canvas) itemconfigure $id -arrow $value
        set options($optname) $value
    }
    #---------------------------------------------------------------------------
    # Public methods.
    #
}