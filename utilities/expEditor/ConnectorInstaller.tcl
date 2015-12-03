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
# @file ConnectorInstaller.tcl
# @brief Install a connector.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide connectorInstaller 1.0
package require connector
package require Tk

##
# @class ConnectorInstaller
#    installer for connectors.  This guy is a bit unusual, rather than just
#    cloning and placing an initial copy of the toolbar entity, a connector must
#    be formed by picking two 'eligible' objects.
#
#  TODO:  Define eligible in a way that only allows programs <-> rings.
#
#   Once both objects are chosen, the the connector is drawn between them.
#
#  OPTIONS
#    -installcmd - script invoked when a connector is installed. Substitutions:
#            %C - ordered list of connected objects.
#            %W - Widget he connector is installed on.
#            %O - Object that was installed
#
#  METHODS:
#     *  install   - install a new connector.
#     *  uninstall - Remove a connector from the system.
#
snit::type ConnectorInstaller {
    option -installcmd [list]
    variable item1 ""
    variable item2 ""
    
    # connectors is a list of connector definitions.  Each connecto is defined
    # by a dict consisting of the following keys:
    #
    #  *  object   - The connector object.
    #  *  from     - The from object.
    #  * to        - The to object.
    #  * canvas    -  The canvas on which the connector is drawn. 
    
    variable currentConnectors [list]
    
    #---------------------------------------------------------------------------
    # Private methods
    #
    
    ##
    # _connectionPoints
    #   Given an image object and its center point produce a set of 8 coordinate
    #   pairs that represent the legal attachment points of that image.
    #   The legal attachment points are the  midpoints of the
    #   Bounding rectangle.
    #
    # @param c      - Canvas on which the items live.
    # @param center - x,y pair of the image midpoint.
    # @param id     - Id of the object on the canvas.
    # @return [list] containing 8 pairs of coordinates that specify the end points.
    #
    method _connectionPoints {c center id} {
        #
        # Get the image name and its height/width.
        #
        set img [$c itemcget $id -image]
        set w   [image width $img]
        set w   [expr {$w/2}]
        set h   [image height $img]
        set h   [expr {$h/2}]
        
        # Now get the coordinate limits:
        
        set xc [lindex $center 0]
        set yc [lindex $center 1]
        
        set l [expr {$xc - $w}]
        set r [expr {$xc + $w}]
        set t [expr {$yc - $h}];     # Y coordinates are 'backwards'.
        set b [expr {$yc + $h}]
        
        foreach x [list $l  $r] {
            lappend result [list $x $yc]
        }
        
        foreach y [list $t  $b] {
            lappend result [list $xc $y]
        }
        return $result
    }
    
    ##
    # _tagAllItems
    #   tags eligible connectable items with 'connectable'
    #   TODO: For now all image items are tagged.
    #
    #  @param c - canvas on which we're working.
    #
    method _tagAllItems c {
        set items [$c find all]
        foreach item $items {
            if {[$c type $item] eq "image"} {
                $c addtag  connectable withtag $item
            }
        }
    }
    ##
    # _computeConnectionCoords
    #
    #    Compute the pair of closest attachment points between item1 and item2.
    #    - The coordinates of items are assumed to be at their center points.
    #    - We compute coordinates of connection points which are defined to be
    #      the corners and the midpoints of the bounding box.
    #    - We figure out which pair of connectors are closest and return their
    #      coordinates.
    #
    # @param c - The canvas.
    # @return list - two element list containing two coordinate pairs for
    #          the connection point on item1 and item2 respectively.
    #
    method _computeConnectionCoords c {
        # First lets get the coordinates of the items.
        
        set c1 [$c coords $item1]
        set c2 [$c coords $item2]
        
        #  Now let's turn that into a list of connection points
        
        set cpts1 [$self _connectionPoints $c $c1 $item1]
        set cpts2 [$self _connectionPoints $c $c2 $item2]

        #  Pair each connection point in cpts1 with the closest connection point
        #  in cpts2:
        
        set pairs [list]
        foreach pt $cpts1 {
            lappend pairs [list $pt [_findClosest $pt $cpts2]]
        }
        # Now find the pair with the minimum distance:
        
        return [_minimumDistancePair $pairs]
        
    }
    ##
    # _dispatch
    #   Dispatch scripts.
    #
    # @param optname - name of option holding the script.
    # @param submap  - Substitution map.
    #
    method _dispatch {optname submap} {
        set script $options($optname)
        if {$script ne ""} {
            set script [string map $submap $script]
            uplevel #0 $script
        }
    }
    ##
    # _connect
    #   item1 and item2 will be connected.
    #
    # @param c - the canvas on which the arrow line is being drawn.
    #
    method _connect {c} {
        set connections [$self _computeConnectionCoords $c]
        set from [lindex $connections 0]
        set to   [lindex $connections 1]
        
        
        
        set item [connector %AUTO% \
            -from $item1 -to $item2 -fromcoords $from -tocoords $to \
            -arrow last -canvas $c]
        
        lappend currentConnectors [dict create object $item from $item1 to $item2 canvas $c]
        
        $self _dispatch -installcmd "%W $c %C [list $item1 $item2] %O $item"
        
        #  Now we have no items:
        #
        set item1 ""
        set item2 ""
    }
    ##
    # _removeTags
    #    Remove the connectable tag from all that hold it:
    #
    # @param c - Canvas on which we are removing tagging.
    #
    method _removeTags c {
        $c dtag connectable connectable
    }
    ##
    # _removeBindings
    #   Unset the bindings associated with the connectable tag.
    #
    # @param c - the canvas.
    
    method _removeBindings c {
        $c bind connectable <Button-1> ""
        $c bind connectable <Button-3> ""
    }
    ##
    #  _select
    #    Choose an item:
    #    -  If item1 is "" that't the item chosen.
    #    -  otherwise item2 is chosen only if the two differ.
    #    - If the same item is chosen twice an error is reported.
    #
    # @param c   - canvas being picked in.
    # @param x,y - coordinates of the pointer.
    #
    method _select {c x y} {
        set item [$c find closest $x $y]
        if  {$item1 eq ""} {
            set item1 $item
        } else {
            if {$item eq $item1} {
                tk_messageBox -title "Bad connection" -icon error -type ok \
                    -message {You cannot connect an item to itself}
            } else {
                set item2 $item
                $self _removeTags $c
                $self _removeBindings $c
                $self _connect $c
               
            }
        }
    }
    ##
    # _deselect
    #   Unselect item 1.
    #     Unless we want to require the user point at the item being deselected
    #     don't actually need the parameters.
    #
    method _deselect {w x y} {
        set item1 "";               # All that's really needed.
    }
    
    ##
    # _makeBindings
    #    Add the bindings required to select connected objects.
    #
    # @param c - the canvas.
    #
    method _makeBindings c {
        $c bind connectable <Button-1> [mymethod _select %W %x %y]
        $c bind connectable <Button-3> [mymethod _deselect %W %x %y]
    }
    #---------------------------------------------------------------------------
    # private procs (static methods).
    #

    ##
    # _minimumDistancePair
    #
    #   Return the pair of point from a list of point pairs that are closest
    #   together.
    # @param pairs - list of point pairs.
    # @return list - of two points that are closest.
    #
    proc _minimumDistancePair pairs {
        set result [list]
        set min    ""
        foreach pair $pairs {
            set f [lindex $pair 0]
            set l [lindex $pair 1]
            
            set x1 [lindex $f 0]
            set y1 [lindex $f 1]
            set x2 [lindex $l 0]
            set y2 [lindex $l 1]
            
            # We're going to compare the square of the distances
            
            set d [expr {($x2-$x1)*($x2-$x1) + ($y2-$y1)*($y2-$y1)}]
            if {($min == "") || ($d < $min)} {
                set min $d
                set result $pair
            }
        }
        return $result
    }
    
    ##
    # _findClosest
    #   Given a point and a series of candidate points, return the candidate
    #   point that is closest to the point.  We're going to do this a bit
    #   weirdly but in a way that provides maximum code re-use
    #
    # @param initial - initial coordinates.
    # @param pairs   - pairs to check for.
    # @return list   - the x/y coordinate of the closest point.
    proc _findClosest {initial pairs} {
        #
        # Create pairs suitable for use in _minimuDistancePair:
        #
        foreach pair $pairs {
            lappend minpair [list $initial $pair]
        }
        
        return [lindex [_minimumDistancePair $minpair] 1]
    }
    ##
    # _findConnectionFromOrTo
    #    Locate the first connection that either originates or ends in the
    #    specified canvas/id.
    #
    # @param id   - Id of one of the terminations.
    # @param c    - Canvas
    #
    method _findConnectionFromOrTo {id c} {
        for {set i 0} {$i < [llength $currentConnectors]} {incr i} {
            set connection [lindex $currentConnectors $i]
            set ca [dict get $connection canvas]
            set f  [dict get $connection from]
            set t  [dict get $connection to]
            if {($c == $ca) && (($f == $id) || ($t == $id))} {
                return $i
            }
        }
        return -1
    }
    #---------------------------------------------------------------------------
    # Public methods.
    
    
    ##
    # install
    #    Invoked to actually install a new connector. We tag all canvas items
    #    with connectable (TODO: All eligible canvas items).  Bind
    #    <Button-1> to _addObject which will do the rest.
    #
    # @param object in the tool bar being installed (ths is not useful just
    #        a connector icon).
    # @param from - Information about the from object. This is also useless.
    # @param to   - Canvas in which the connector is being generated.
    method install {object from to} {
        $self _tagAllItems $to
        $self _makeBindings $to
    }
    ##
    # uninstall
    #    Call when a connected item is being deleted to destroy all connectors
    #    that originate or terminate in the object.
    #
    # @param from   - canvas id of one of the objects.
    # @param c      - canvas.
    #
    method uninstall {from c} {
        set id [$self _findConnectionFromOrTo $from $c]
        while {$id != -1} {
            set connector [lindex $currentConnectors $id]
            set cobj [dict get $connector object]
            $cobj destroy
            set currentConnectors [lreplace $currentConnectors $id $id]
            set id [$self _findConnectionFromOrTo $from $c]
        }
        return true
    }
}