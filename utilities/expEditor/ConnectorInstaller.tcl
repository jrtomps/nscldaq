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
    
    #  These are the from (item1) and to (item2) objects being connecte.
    
    
    variable item1 ""
    variable item2 ""
    
    #  The following variables are used to manage the rubber band arrow that's
    #  drawn during connector creation.
    #  *  tempArrowId  is the canvas Id of the item.
    #  *  tempArrowStartX, tempArrowStartY are the starting coordinates of the arrow (the end point
    #    tracks the pointer position via a <Motion> event binding).
    variable tempArrowId ""
    variable tempArrowStartX
    variable tempArrowStartY
    
    # connectors is a list of connector definitions.  Each connector is defined
    # by a dict consisting of the following keys:
    #
    #  *  object   - The connector object.
    #  *  from     - The from object.
    #  * to        - The to object.
    #  * canvas    -  The canvas on which the connector is drawn.
    
    
    variable currentConnectors [list]
    
    ##
    # currentObjects is the set of objects known to the  installer if it's been
    # hooked into the corresponding object installer object for objects it can
    # connect.
    #  This is an array indexed by cavas id containing a list of  dicts containing:
    #
    #   -  object  - the object ensemble command.
    #   -  canvas  - canvas on which this object is drawn.
    #   -  id      - Id of object on canvas (avoids rev. lookups).
    #
    # List because in theory we can be managing the installation for more than one
    # canvas and two distinct objects could have the same canvas id but be on
    # different canvases.
    #
    
    variable currentObjects -array [list]
    
    #---------------------------------------------------------------------------
    # Private methods
    #
    
    method _moveRubberBandArrow {c x y} {

        $c coords $tempArrowId $tempArrowStartX $tempArrowStartY $x $y
    }
    
    ##
    # _createRubberBandArrow 
    #   Create the rubber band arrow that will track the creation of the
    #   connector.
    #
    # @param c - the canvas on which we are drawing.
    # @param x - the starting x coordinate of the arrow (end point tracks the pointer)
    # @param y - the starting y coordinate of the arrow (end point tracksthe pointer).
    #
    #  @note - for convenience, the initial end point of the line is x+1, y+1
    #          the motion event will soon correct this.
    #
    method _createRubberBandArrow {c x y} {
        set tempArrowStartX $x
        set tempArrowStartY $y
        set tempArrowId [$c create line $x $y [incr x] [incr y] -dash - -arrow last]
        bind $c <Motion> [list $c coords $tempArrowId $tempArrowStartX $tempArrowStartY %x %y]
        $c bind $tempArrowId <Button-1> [mymethod _select $c %x %y]
    }

    ##
    # _findObject
    #     Given a list of object info dicts (an element of currentObjects), find
    #     and return the object that is on the specified canvas.
    #
    # @param dList - list of dicts.
    # @param c     - canvas to match.
    # @return command - base command of an object command ensemble.
    #
    method _findObject {dList c} {
        foreach d $dList {
            if {[dict get $d canvas] == $c} {
                return [dict get $d object]
            }
        }
        #   Empty object if not found.
        
        return ""
    }
    
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
    #  @param c   - canvas on which we're working.
    #  @param dir - direction of the connection (from to)
    #
    method _tagAllItems {c dir} {
        
        # Iterate through the objects - for each id if it has objects on the
        # canvas, ask that object if it can be connected in the requested direction.
        # If so, add the connectable tag to the object's canvas rendition:
        
        foreach id [array names currentObjects] {
            set o [$self _findObject $currentObjects($id) $c]
            if {[$o isConnectable $dir]} {
                $c addtag connectable withtag $id
            }
        }   
    }
 

    ##
    # _makeBindings
    #    Add the bindings required to select connected objects.
    #
    # @param c - the canvas.
    #
    method _makeBindings c {
        $c bind connectable <Button-1> [mymethod _select %W %x %y]
        #  Arrange for the escape key to abor the process of creating the connection.
        
        focus $c
        bind $c <KeyPress-Escape> [mymethod _abortConnection $c]
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
        
        # find the objects themselves and let them know they were connected.
        # This is done first so that if an error is thrown, we can _abortConnection.
        #
        
        set status [catch {
            set fromDicts $currentObjects($item1)
            set toDicts   $currentObjects($item2)
            
            set fromObj [$self _findObject $fromDicts $c]
            set toObj   [$self _findObject $toDicts   $c]
            
            $fromObj connect from $toObj
            $toObj   connect to   $fromObj
            
        } msg]
        if {$status} {
            $self _abortConnection $c
            tk_messageBox -type ok -icon error -title "not allowed" \
                -message "Connection is rejected by one of the objects as illegal from/to type: $msg $::errorInfo"
            return
        }
        
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
        $c dtag connectable
    }
    ##
    # _removeBindings
    #   Unset the bindings associated with the connectable tag.
    #
    # @param c - the canvas.
    
    method _removeBindings c {
        $c bind connectable <Button-1> ""
        bind $c <KeyPress-Escape> ""
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
        
        if  {$item1 eq ""} {
            set item [$c find closest $x $y]
            set item1 $item
            
            $self _createRubberBandArrow $c $x $y
            
            # Bind to the objects that can be destinations:
            
            $self _removeBindings $c
            $self _removeTags $c
            
            $self _tagAllItems $c to
            $self _makeBindings $c
            
        } else {
            set item [$c find closest $x $y 5 $tempArrowId]
            if {$item eq $item1} {
                tk_messageBox -title "Bad connection" -icon error -type ok \
                    -message {You cannot connect an item to itself}
            } else {
                #
                #  With the rubber band arrow requiring a <button-1> binding
                #  on the _canvas_ we need to see if the item is connectable.
                #  If not ignore the click:
                
                if {[array names currentObjects $item] eq ""} {
                    return
                }
                set o        [$self _findObject $currentObjects($item) $c]
                if {($o eq "") || ![$o isConnectable to]} {
                    return
                }
                
                # Connection can proceed
                
                set item2 $item
                $self _removeTags $c
                $self _removeBindings $c
                $self _connect $c
               
                $c delete $tempArrowId;               # Destroy rubberband arrow.
                set tempArrowId ""
                bind $c <Motion> ""
            }
        }
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
    ##
    # _abortConnection
    #    Called to stop doing a connection.
    #    - Bindings on the canvas are turned off.
    #    - item1, item2 are cleared.
    #
    # @param c - the canvas on which the connector is being drawn.
    #
    method _abortConnection c {
        $self _removeBindings $c
        $self _removeTags     $c
        
        if {$tempArrowId ne ""} {
            $c delete $tempArrowId
        }
        
        set item1 ""
        set item2 ""
        set tempArrowid ""
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
    # _notifyDisconnect
    #    Invoke the disconnect method of the other end of a connection when
    #    one end is being destroyedl
    #
    # @param object  - the object being destroyed.  We don't bother calling its
    #                  disconnect method.
    # @param connection - The connection dict.
    # @param c          - Canvas.
    #
    method _notifyDisconnect {object connection c} {
        
        #  Get the from/to ids and look up the objects they represent:
        
        set from [dict get $connection from]
        set to   [dict get $connection to]
        
        set fromObj [$self _findObject $currentObjects($from) $c]
        set toObj   [$self _findObject $currentObjects($to) $c]
        
        if {$fromObj ne $object} {
            $fromObj disconnect $object
        }
        if {$toObj ne $object} {
            $toObj disconnect $object
        }
    }
    ##
    # _propertyChanged
    #    Callback invoked by an object property that has changed.
    #
    # @param desc - Dict describing the object with the keys:
    #               * object - object ensemble command.
    #               * canvas - canvas on which the object's graphical rep is drawn.
    #               * id     - Canvas id of the object's graphical representation.
    #
    method _propertyChanged desc {
        set objid [dict get $desc id]
        set c     [dict get $desc canvas]
        set o     [dict get $desc object]
        #  Use currentConnectors to locate the objects we are connected to:
        
        set connectedList [list]
        foreach connection $currentConnectors {
            if {[dict get $connection from] == $objid} {
                lappend connectedList [dict get $connection to]
            }
            if {[dict get $connection to] == $objid} {
                lappend connectedList [dict get $connection from]
            }
        }
        #  Invoke the connectionPropertyChanged method on each of the connected
        #  objects.
        
        foreach id $connectedList {
            set co [$self _findObject $currentObjects($id) $c]
            $co connectionPropertyChanged $o
        }
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
        $self _tagAllItems $to from
        $self _makeBindings $to
        

        
    }
    ##
    # uninstall
    #    Call when a connected item is being deleted to destroy all connectors
    #    that originate or terminate in the object.
    #
    # @param object - base command of deleted object's command ensemble.
    # @param from   - canvas id of one of the objects.
    # @param c      - canvas.
    # @param object
    method uninstall {object from c} {

        # Destroy any connectors to/from the defuct object.
    
        set id [$self _findConnectionFromOrTo $from $c]
        while {$id != -1} {
            set connector [lindex $currentConnectors $id]
            $self _notifyDisconnect $object $connector $c
            set cobj [dict get $connector object]
            $cobj destroy
            set currentConnectors [lreplace $currentConnectors $id $id]            
            set id [$self _findConnectionFromOrTo $from $c]
            
        }
        
        
        # Destroy  our record of the object each canvas really is only allowed
        # to have one guy with any one id:
        
        if {[array names currentObjects $from] ne ""}  {
            set obDicts $currentObjects($from)
            for {set i 0} {$i < [llength $obDicts]} {incr i} {
                set oDict [lindex $obDicts $i]
                set obj   [dict get $oDict object]
                set canv  [dict get $oDict canvas]
                
                # If this object matches, remove it from the list and update
                # currentObjects - then break out of the loop.  Really object names
                # should be a sufficient test.
                
                if {($object eq $obj) && ($c == $canv)} {
                    set obDicts [lreplace $obDicts $i $i]
                    if {[llength $obDicts] > 0} {
                        set currentObjects($from) $obDicts
                    } else {
                        array unset currentObjects $from
                    }
                    break
                }
            }
        }
        return true
    }
    ##
    # newObject %O %I %W
    #
    #    Call this when a new object has been created/installed.   This maintains
    #    the set of objects that might be connectable on the canvas.
    #
    # @param obj    - object's command ensemble command.
    # @param id     - object's visual representation canvas id.
    # @param c      - canvas on which the object was installed.
    #
    method newObject {obj id c} {
        set objDesc [dict create object $obj canvas $c id $id]
        lappend currentObjects($id) $objDesc
        
        # Hook into the property change callback:
        
        $obj configure -changecmd [mymethod _propertyChanged $objDesc]
    }
}