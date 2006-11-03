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
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#

#  Implements a 'meter' megawidget.  A meter is a
#  box with a needle that goes up and down between
#  two possible limits.
#
# This is drawn in a canvas as follows:
#    +-------+
#    |       |
#    |  <----|
#    | ...
#    +-------+
#
#
# OPTIONS:
#    -from        - Value represented by the lower limit of the meter.  (dynamic)
#    -to          - Value represented by the upper limit of the meter.  (dynamic)
#    -height      - Height of the meter.                                (static)
#    -width       - Width of the meter.                                 (static)
#    -variable    - Variable the meter will track.                      (dynamic)
#    -majorticks  - Interval between major (labelled) ticks.            (dynamic)
#    -minorticks  - Number of minor ticks drawn between major ticks.    (dynamic)
#
# Methods:
#    set value    - Set the meter to a specific value (if -variable is defined it is modified).
#    get          - Returns the current value of the meter.

package provide meter 1.0
package require Tk
package require snit
package require bindDown

namespace eval controlwidget {
    namespace export meter
}
snit::widget controlwidget::meter {
    option -from        -1.0
    option -to          1.0
    option -height     {2i}
    option -width      {1.5i}
    option -variable   {}
    option -majorticks 1.0
    option -minorticks 4

    variable needleId       {}
    variable topY           {}
    variable bottomY        {}
    variable valueRange     {}
    variable needleLeft     {}
    variable meterLeft      {}

    variable tickIds        {}
    variable lastValue       0

    # Construct the widget:

    constructor args {
        $self configurelist $args

        set valueRange [expr 1.0*($options(-to) - $options(-from))]

        # Create the canvas and draw the meter into the canvas.
        # The needle is drawn at 1/2 of the rectangle height.
        # 3/4 width.
        # We'll store the resulting size back in the options asn
        # pixels since their much easier to work with:

        canvas $win.c   \
            -width $options(-width)   \
            -height $options(-height) \
            -background white


        set options(-height) [$win.c cget -height]
        set options(-width)  [$win.c cget -width]

        # In order to support label we need to create a left margin
        # the margin will be 8chars (80pt) wide
        # and a top/bottom margin of 5pt.. the assumption is that the labels
        # will be drawn in 10pt font.

        set leftmargin [$win.c canvasx 45p]
        set vmargin    [$win.c canvasy 5p]

        # Compute the coordinates of the rectangle and the top/bottom limits
        # (for scaling the arrow position).

        set meterLeft  $leftmargin
        set topY       $vmargin
        set meterRight $options(-width)
        set bottomY    [expr $options(-height) - $vmargin]


        # draw the frame of the meter as a rectangle:

        $win.c create rectangle $meterLeft $topY $meterRight $bottomY

        # figure out how to put the needle in the middle of the
        # height of the meter allowing 1/4 of the meter for ticks.
        #

        set needleWidth   [expr 3*($meterRight - $meterLeft)/4]
        set needleHeight  [$self computeHeight   \
                             [expr ($options(-to) + $options(-from))/2]]
        set needleLeft   [expr $options(-width) - $needleWidth]

        set needleId [$win.c create line $needleLeft $needleHeight      \
                                         $options(-width) $needleHeight \
                                        -arrow first]

        grid $win.c -sticky nsew


        $self drawTicks

        if {$options(-variable) ne ""} {
           trace add variable ::$options(-variable) write [mymethod variableChanged]
            $self needleTo [set ::$options(-variable)]
        }
	bindDown $win $win

    }

    #-------------------------------------------------------------------------------
    # public methods
    #

    # Set a new value for the meter... this moves the pointer to a new value.
    # if a variable is tracing the meter, it is changed
    #
    method set newValue {
        if {$options(-variable) ne ""} {
            set ::$options(-variable) $newValue;      # This updates meter too.
        } else {
            $self needleTo $newValue
        }
    }

    # Get the last meter value.
    #
    method get {} {
        return $lastValue
    }

    #-------------------------------------------------------------------------------
    # 'private' methods.

    # trace on -variable being modified.

    method variableChanged {name1 name2 op} {

        $self needleTo [set ::$options(-variable)]
    }

    # Set a new position for the needle:

    method needleTo newCoords {
        set lastValue $newCoords

        set height [$self computeHeight $newCoords]
        $win.c coords $needleId $needleLeft $height $options(-width) $height
    }

    # Compute the correct height of the needle given
    # A new coordinate value for it in needle units:

    method  computeHeight needleCoords {

        #
        # Peg the needle to the limits:
        #
        if {$needleCoords > $options(-to)}  {
            return $topY
        }
        if {$needleCoords < $options(-from)} {
            return $bottomY
        }
        set pixelRange [expr 1.0*($bottomY - $topY)]

        # Transform the coordinates:

        set height [expr ($needleCoords - $options(-from))*$pixelRange/$valueRange]
        return [expr $bottomY - $height]
    }

    # Draw the tick marks on the meter face.  The major ticks are
    # labelled, while the minor ticks are just some length.
    # Major ticks extend from the meter left edge to 1/5 the width of the meter
    # while minor ticks extend from the meter left edge to 1/10 the width of the meter.
    # Tick labels are drawn at x coordinate 0.
    #
    method drawTicks {} {
        set first $options(-from)
        set last  $options(-to)
        set major $options(-majorticks)


        # minor ticks are just given in terms of the # ticks between majors so:

        set minor [expr 1.0*$major/($options(-minorticks)+1)]

        # Figure out the right most coordinates of the tick lines.

        set majorlength [expr ($options(-width) - $meterLeft)/5]
        set minorlength [expr  $majorlength/2]
        set majorRight  [expr $meterLeft + $majorlength]
        set minorRight  [expr $meterLeft + $minorlength]

       # the for loop is done the way it is in order to reduce
       # the cumulative roundoff error from repetitive summing.
       #


        set majorIndex 0
        for {set m $first} {$m <= $last} {set m [expr $first + $majorIndex*$major]} {
            # Draw a major tick label and the tick mark itself
            # major ticks are formatted in engineering notation (%.1e).

            set label [format %.1e $m]
            set height [$self computeHeight $m]
            lappend tickIds [$win.c create text  $meterLeft $height -text $label -anchor e]
            lappend tickIds [$win.c create line  $meterLeft $height $majorRight $height]
            for {set i 1} {$i <=  $options(-minorticks)} {incr i} {
                set minorH [expr $m + 1.0*$i*$minor]
                set minorH [$self computeHeight $minorH]
                lappend tickIds [$win.c create line $meterLeft $minorH $minorRight $minorH]
            }
            incr majorIndex
        }
    }
    #
    #  Erase the Tick ids from the meter:
    #
    method eraseTicks {} {
        foreach id $tickIds {
            $win.c delete $id
        }
    }

    #------------------------ Configuration handlers for dynamic options  ----
    #    -from        - Value represented by the lower limit of the meter.  (dynamic)
    #    -to          - Value represented by the upper limit of the meter.  (dynamic)
    #    -variable    - Variable the meter will track.                      (dynamic)
    #    -majorticks  - Interval between major (labelled) ticks.            (dynamic)
    #    -minorticks  - Number of minor ticks drawn between major ticks.    (dynamic)


    # Handle configure -from
    # Need to set the stuff needed to scale the meter positions and reset the meter position.
    # Need to redraw ticks as well.
    #
    onconfigure -from value {
        set options(-from) $value
        if {![winfo exists $win.c]} return;     # Still constructing.
        $self eraseTicks
        set   valueRange [expr $options(-to) - $value]
        $self drawTicks

        $self needleTo $lastValue
    }
    # Handle configure -to
    # As for -from but -to is modified.
    #
    onconfigure -to value {
        set options(-to) $value
        if {![winfo exists $win.c]} return;     # Still constructing.
        $self eraseTicks
        set valueRange [expr $value - $options(-from)]
        $self drawTicks
        $self needleTo $lastValue
    }

    # Handle a change in major ticks.. we just need to set the option and redraw the ticks.
    #
    onconfigure -majorticks value {
        set options(-majorticks) $value
        if {![winfo exists $win.c]} return;     # Still constructing.
        $self eraseTicks
        $self drawTicks
    }
    # same but for minor ticks...
    #
    onconfigure -minorticks value {
        set options(-minorticks) $value
        if {![winfo exists $win.c]} return;     # Still constructing.
        $self eraseTicks
        $self drawTicks
    }
    #  Configure the variable for the meter.
    #  Any prior variable must have its trace removed.
    #  The new variable gets a trace established and the meter position
    #  is updated from it.
    #  Note that if the new variable is "" then the meter will have
    #  no variable associated with it.

    onconfigure -variable name {

        # Could be still constructing in which case
        # $win.c does not exist:

        if {![winfo exists $win.c]} {
            set options(-variable) $name
            return;
        }

        # Remove any old traces


        if {$options(-variable) ne ""} {
            trace remove variable ::$options(-variable) write [mymethod variableChanged]
        }

        # Set new trace if appropriate and update value.

        set options(-variable) $name
        if {$options(-variable) ne ""} {
            trace add variable ::$options(-variable) write [mymethod variableChanged]
            $self needleTo [set ::$options(-variable)]
        }

    }
}

