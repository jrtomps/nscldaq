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
# @file scalermain.tcl
# @brief Scaler program main.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide scalermain 1.0


if {[info command __package_orig] eq "__package_orig"} {

    # pkg_mkIndex is running us so exit before some of our esoteric stuff fails.
    # Making this package not index properly.
    
    return
}

set here [file dirname [info script]]
set libdir [file normalize [file join $here ..]]
lappend auto_path $libdir


package require Thread
package require Tk
package require scalerconfig
package require header
package require scalerReport
package require Plotchart::xyplotContainer
package require scaleControl
package require zoomPrompt


## Ensure the user supplied a configuration file:

if {[llength $argv] != 1} {
    puts stderr "You need to supply a single parameter: the configuration script filename."
    exit -1
}

##
# Globals:

set notebook "";                  # Widget containing the tabbed notebook.
set header   "";                  # Header widget.

set startTime 0;                  # when the run started:
set duration  0;                  # Number of seconds in the run.
set stripcharts "";               # xyplotContainer with the stripchart.
set alarmcontrol 1;               # start with alarms on.

#  Y scaling variables
#
#  initialYrange is the basis for the scale multiplier (remember magnification
#  is inverse to scale so a magnification of 3x means that ymax = ymin + (initialYRange/3)).

set autoY 1;                      # If true autoscaling y axis.
set initialYrange  "";            # Range of y values when autoscale turned off.


#-------------------------------------------------------------------------
# Provide access to the globals for extensions:


##
# getStartTime
# @return [clock seconds] for when the run started.
#
proc getStartTime {} {
    return $::startTime
}
##
# get ElapsedTime
#
# @return - seconds of elapsed run time.
#
proc getElapsedTime {} {
    return $::duration
}
##
# getTitle
#   @return most recent title string.
#
proc getTitle {} {
    set h [getHeader]
    
    return [$h cget -title]
}
##
# getRunNumber
#
# @return most recent run number.
#
proc getRunNumber {} {
    set h [getHeader]
    
    return [$h cget -run]
}
##
# getState
#
# @return  most recent run state string.
#
proc getState {} {
    set h [getHeader]
    return [$h cget -state]
}

#---------------------------------------------------------------------------
# Strip chart y axis scale handling:
#

proc yScaleChanged {newValue} {
    
    #  Two special values are Auto and Custom...
    
    if {$newValue eq "Auto"} {
        #
        #  If not auto before, reset the scale and set the auto flag:
        
        if {! $::autoY} {
            set xmin [$::stripcharts cget -xmin]
            $::stripcharts configure -ymax [expr {$xmin + $::initialYRange}]
            set ::autoY 1    
        }
            
        
    
    } else {
        if {$newValue eq "Custom..."} {
            #  Must prompt.
           
           set zoom [getNewZoom]
           if {$zoom eq ""} return;                 # Cancel or invalid zoom...
            
        } else {
            # Decode from scale value  it's of the form nnx where nn is a zoom
            # factor.
            
            set zoom [string range $newValue 0 end-1]
        }
        #  If autoY is set, we need to record the range so that our expansions
        #  relate to that -- and we need to turn off autoY:
        
        set ymin [$::stripcharts cget -ymin]
        if {$::autoY} {
            set ymax [$::stripcharts cget -ymax]
            
            set ::initialYRange [expr {$ymax - $ymin}]
            set ::autoY 0
        }

        # Now determine the new scale range, from that ymax and set it:
        
        set newRange [expr {$::initialYRange / $zoom}]
        $::stripcharts configure -ymax [expr {$ymin + $newRange}]
    }
}
##
# yminChanged
#    Called when the user wants a non zero value for the strip chart minimum y.
#
# @param newValue - the new value of the strip chart ymin.k
proc yMinChanged {newValue} {
    set ymin [$::stripcharts cget -ymin]
    set ymax [$::stripcharts cget -ymax]
    set dy   [expr {$ymax - $ymin}]
    
    $::stripcharts configure -ymin $newValue -ymax [expr {$newValue + $dy}]
}
#------------------------------------------------------------------------------
# strip chart x axis scaling.
#
##
# setXaxis
#
#   Set new values for the x axis limits
#
# @param min - xmin
# @param zoom - Zoom factor.
#
proc setXaxis {min zoom} {
        # Figure out the new range:
        
        set newRange  [expr {$::scalerconfig::stripChartOptions(-timeaxis)/$zoom}]
        $::stripcharts configure -xmin $min -xmax [expr {$min + $newRange}]
            
}

##
# xScaleChanged
#
#   Called when the zoom settings for the xscale changed.
#   Determine the original range (that's a config parameter)
#   and set a new xmin/xmax range basd on the scaled range.
#
proc xScaleChanged {min value} {
    
    # Auto just sets the xmin -0 and xmax to the initial range.  This will
    #  slide next time there's an update:
    
    if {$value eq "Auto"} {
        $::stripcharts configure -xmin 0 -xmax $::scalerconfig::stripChartOptions(-timeaxis)
        
    } else {
        if {$value eq "Custom..."} {
            
            #Need to prompt for a custom zoom range
            
            set zoom  [getNewZoom]
            if {$zoom eq ""} return;              # Zoom not accepted.
        } else {
            set zoom [string range $value  0 end-1]
            
            
        }
        setXaxis $min $zoom
        
    }
}
##
# xMinChanged
#
#   Called when the xmin changed
#
# @param min - new minimum value
#
proc xMinChanged {min} {
    
    # Figure out the current extent:
    
    set xmin [$::stripcharts cget -xmin]
    set xmax [$::stripcharts cget -xmax]
    set dx   [expr {$xmax - $xmin}]
    
    $::stripcharts configure -xmin $xmin -xmax [expr {$xmin + $dx}]
}
    


#---------------------------------------------------------------------------
# Internal private procs
#

##
# Create the thread that will read data from the ring and post it back to us:
#
proc startAcqThread {ringUrl} {
    set acqThread [thread::create -joinable]
    if {[thread::send $acqThread [list lappend auto_path $::libdir] result]} {
        puts "Could not extend thread's auto-path"
        exit -1
    }
    if {[thread::send $acqThread [list package require TclRingBuffer] result]} {
        puts "Could not load RingBuffer package in acqthread: $result"
        exit -1
    }
    
    if {[thread::send $acqThread [list ring attach $ringUrl] result]} {
        puts "Could not attach to scaler ring buffer in acqthread $result"
        exit -1
    }
    
    #  The main loop will forward data to our handleData item.
    
    set myThread [thread::id]
    set getItems "proc getItems {tid uri} { 
        while 1 {                                             
            set ringItem \[ring get \$uri {1 2 20}]             
            thread::send \$tid \[list handleData \$ringItem]     
        }                                                     
    }                                                         
    getItems $myThread $ringUrl
    "
    thread::send -async $acqThread $getItems

    
    return $acqThread
}
#------------------------------------------------------------------------------------------------
#  Scaler display table:

##
#  updatePages
#    Updates all pages on the display.
#
proc updatePages {} {

    set tabNames [::scalerconfig::pages list]
    foreach tab $tabNames {
        set widget [::scalerconfig::pages get $tab]
        set bkg [$widget update]
	
	# Set the tab background accordingly.

	set tabidx [.notebook index $widget]

	if {$bkg eq "ok"} {
	    .notebook tab $tabidx -compound text
	} elseif {$bkg eq "low"} {
	    .notebook tab $tabidx -image GreenBrick -compound center
	} elseif {$bkg eq "high"} {
	    .notebook tab $tabidx -image RedBrick -compound center
	} else {
	    .notebook tab $tabidx -image AmberBrick -compound center
	}
    }

    update idletasks
    
}

#----------------------------------------------------------------------------
#  Strip charts


##
# clearStripcharts
#    Clear all the data from the stripcharts and prepare the plot for
#    new data with auto-y scale.
#
#
proc clearStripcharts {} {
    # Clear the plot:
    
    if {$::stripcharts ne ""} {
	set seriesNames [$::stripcharts getSeries]
	foreach series $seriesNames {
	    $::stripcharts clearSeries $series
            $::stripcharts setSeriesPointLimit $series 20000
	}


	#  Reset the ymax to 1 so autoscale will start up again.:
	
        if {$::autoY} {
            $::stripcharts configure -ymax 1 
        }
        #  Reset the X axis to the current zoom but xmin = 0.
        
        set xmin [$::stripcharts cget -xmin]
        set xmax [$::stripcharts cget -xmax]
        set dx   [expr {$xmax - $xmin}]

	# Create empty series...to re-establish colors.
	#
	foreach series [_getStripItems] color {black red green blue goldenrod purple cyan yellow orange brown} {
	    if {$series eq ""} {
		break
	    }
	    $series clear;                          # Invalidate the time.
	    $::stripcharts series [$series name] [list] [list] $color
	}
        #  Reset the zooms:
        
        

    }
}
##
# Writes the stripchart plot to a postscript file:
#
# @param filename - name of the output file.
#
proc saveStripcharts   {filename} {
    if {$::stripcharts ne ""} {
	set plot [$::stripcharts cget -plotid]
	$plot saveplot $filename 
    }
}
##
# updateStripcharts
#   For each series, that has a new time a new point is drawn for that series.
#   In autoscale; y value of that point is larger than the current -ymax, -ymax is
#   changed to be 10% larger than the requested y value.
#
proc updateStripcharts {} {
    if {$::stripcharts ne ""} {
	
	set ymax -1
	foreach item [_getStripItems] {
	    if {[$item hasUpdated]} {
		set name [$item name]
		set y [$item rate]
		set t [$item time]
		set ymax [expr {max($ymax, $y)}]
		
		$::stripcharts addSeriesPoint $name $t $y
	    }
	}
	
	# If needed update the -ymax to autoscale that axis.
	
	
	if {$::autoY} {
            if {$ymax > [$::stripcharts cget -ymax]} {
                set ymax [expr {$ymax * 1.1}]
                $::stripcharts configure -ymax $ymax
            }
        }
    }
}

#--------------------------------------------------------------------------
# Data handling

##
# scaler
#   Called when a scaler item comes in.
#   * Figure out if there is a data source.
#   * For each scaler figure out the identifier
#   * If there is a command corresponding to the identifier,
#     update the scaler.
#   * Update all display widgets.
# @param item - The scaler item dict.
#
proc scaler item {
    # puts $item
    #
    #  Figure out if there's a source id that needs to appended to the channel
    #  number:
    
    if {[dict exists $item bodyheader]} {
        set sourceId .[dict get $item bodyheader source]
    } else {
        set sourceId "";                   # No source id.
    }
    # Figure out the interval over which the scalers accumulated
    # Note the divisor changes whatever timebase used for the start/end
    # values to floating point seconds.
    
    set end [dict get $item end]
    set start [dict get $item start]
    set dt [expr {double($end - $start)/[dict get $item divisor]}]
    
    # Iterate over the channels invoking update methods for those channels
    # that have been defined for us:
    
    set channel 0
    foreach counter [dict get $item scalers] {
        if {[info command ::channel_$channel$sourceId] ne ""} {
            ::channel_$channel$sourceId update $counter $dt
        }
        
        incr channel
    }
    
    updatePages
    
    updateStripcharts
    
    # The state is now known to be active (if it was not known before):
    
    set h [getHeader]
    $h configure -state Active
    
    # If the end time is longer than duration, use it:
    
    set elapsed [expr {$end/[dict get $item divisor]}]
    if {$elapsed > $::duration} {
        set ::duration $elapsed
        $h configure -elapsed $elapsed
    }
    # Set the dt in seconds for the source, if there is no  body
    # header to supply  an sid, just make a blank source id:
    
    set sid ""
    if {[dict exists $item bodyheader]} {
        set sid [dict get $item bodyheader source]
    }
    $h update $sid $dt
    
    # If the user has extended us with a UserUpdate call that:
    
    if {[info commands ::UserUpdate] ne ""} {
        ::UserUpdate
    }
}
##
# beginRun
#   Handles begin run items:
#   *   All counters are reset.
#   *   The header run number and title are  reset.
#   *   The state is set to Active.
#   *   All pages are refreshed.
#
# @param item - The dict that contains the begin run item.
#
proc beginRun {item} {
    
    set ::startTime [dict get $item realtime]
    set ::duration  0
    
    foreach counter [::scalerconfig::channelMap list] {
        [::scalerconfig::channelMap get $counter] clear
        
    }
    updatePages
    
    set h [getHeader]
    $h configure -title [dict get $item title] -run [dict get $item run] \
        -state Active
    $h clear;                   # Clear the dt's for each data source.
    
    clearStripcharts;           # If there are stripcharts clear them.
    
    #  If the user has plugged into use with a UserBeginRun call it:
    
    if {[info commands ::UserBeginRun] ne ""} {
        ::UserBeginRun
    }
}
##
# endRun
#   state -> inactive.
proc endRun   {item} {
    
    [getHeader] configure -state Inactive
    
    # Only make reports if the run start time is available:
    
    if {$::startTime > 0} {
        
        
        # Human readable:
        
        set run [dict get $item run]
        set filename [format run%04d.report $run]
        set fd [open $filename w]
        humanReport $fd $::startTime $item ::scalerconfig::channelMap
        close $fd
        
        #  Computer readable:
        
        set filename [format run%04d.csv $run]
        set fd [open $filename w]
        computerReport $fd $::startTime $item ::scalerconfig::channelMap
        close $fd
        
        #  Stripchart postscript.
        
        set filename [format run%04d-stripchart.ps $run]
        saveStripcharts $filename
    }
    #
    #  If the user has plugged into us with a UserEndRun proc, call it:
    #
    if {[info commands ::UserEndRun] ne ""} {
        ::UserEndRun    
    }
}

##
# handleData
#   Called with a dict that contains the ring item when a new ring item arrives.
#
# @param item - the new ring item.
#
proc handleData item {
    #puts $item
    # Dispatch based on the type of event:
    
    set type [dict get $item type]
    switch $type {
        "Begin Run" {beginRun $item}
        "End Run"   {endRun   $item}
        "Scaler"    {scaler   $item}
    }
}

# Procs to help the configuration file interact with the GUI:

##
# getNotebook
# @return widget path of the notebook.
#
proc getNotebook {} {
    return $::notebook
}
##
# Add a new widget to the notebook.
#
# @param widget - path to the widget - must be a child of the notebook.
# @param tabname - Text to put in the tab.
#
proc addPage {widget tabname} {
    pack $widget -fill both -expand 1;    # Just stack them in the toplevel for now.
    [getNotebook] add $widget -text $tabname
}

##
# getHeader
#   Return the header widget.
#
proc getHeader {} {
    return $::header
}

##
#  enableDisableAlarms
#    Called when the toggle button that controls the alarm enables changes
#
# @param widget - Path to the widget that enables/disables the controls.
#
proc enableDisableAlarms widget {
    set state $::alarmcontrol
    set tabNames [::scalerconfig::pages list]
    foreach tab $tabNames {
        set wid [::scalerconfig::pages get $tab]
        $wid alarms $state
    }
}

##
# ResizeStripchart
#   Resize the stripchart canvas and the stripchart widget to the new width
#   of the window.  This is done by:
#   *  Getting the geometry  of . and decoding it.
#   *  Whacking a few pixels off the width.
#   *  resizing the stripchart (which resizes the canvas) to the current
#      canvas height and computed width.
#
# @param charts - the stripchart container object.
#
proc ResizeStripchart charts {
    #
    #  The geometry is of the form widthxheight+xoffset+yoffset ..
    #
    set geometry [wm geometry .]
    set wh       [split [lindex [split $geometry +] 0] x]
    set width    [lindex $wh 0]
    
    set canvasWidth [expr {$width - 10}]
    set canvasHeight [[$charts cget -canvas] cget -height]
    
    $charts resize $canvasWidth $canvasHeight
}

##
# setupGui
#   Set up the top level gui stuff.
#
proc setupGui {} {
    set ::header [header .header -title ????? -run ???? -elapsed 0]
    pack .header -fill x -expand 1
    set ::notebook [ttk::notebook .notebook]
    pack .notebook -fill both -expand 1
    ttk::frame .alarmcontrol
    ttk::checkbutton .alarmcontrol.enable -text {Enable Alarms} -variable alarmcontrol \
        -command [list enableDisableAlarms .alarmcontrol.enable]
    grid .alarmcontrol.enable -sticky w -row 0 -column 0
    
   
    
    pack .alarmcontrol -fill x -expand 1
}



##
# setupStripchart
#
#  Create a canvas at the bottom of the display and put a stripchart widget
#  into it.  Create a series for each item in the list.
#
# @param charts - List of strip charts to create
#
proc setupStripchart charts {
    
    # Axis controls go in the alarm strip for brevity:
    
    ttk::labelframe .alarmcontrol.y -text {Y axis}
    ScaleControl    .alarmcontrol.y.s -menulist [list 1x 2x 4x 8x 16x 32x Custom... Auto]
    .alarmcontrol.y.s configure -zoomrange [list 0 5]
    .alarmcontrol.y.s configure -current Auto
    .alarmcontrol.y.s configure -command [list yScaleChanged %S] \
        -mincommand [list yMinChanged %M]
    
    ttk::labelframe .alarmcontrol.x -text {X axis} 
    ScaleControl    .alarmcontrol.x.s -menulist [list 1x 2x 4x 8x 16x 32x Custom... Auto]
    .alarmcontrol.x.s configure -zoomrange [list 0 5]
    .alarmcontrol.x.s configure -current Auto -enablemin false
    .alarmcontrol.x.s configure -command [list xScaleChanged %M %S] \
        -mincommand [list xMinChanged %M]
    
    
    pack .alarmcontrol.y.s -fill both -expand 1
    pack .alarmcontrol.x.s -fill both -expand 1
    grid .alarmcontrol.y  -sticky nsew -row 0 -column 1
    grid .alarmcontrol.x  -sticky nswe -row 0 -column 2
  
    # The strip charts themselves:
    
    canvas .stripcharts
    pack .stripcharts -fill x -expand 1
    
    # Ensure the canvas size has been computed by the packer:
    
    update idletasks
    update idletasks
    
    # The ranges given should ensure that the y will auto-scale once points
    # start arriving.
    
    set ::stripcharts [Plotchart::xyplotContainer %AUTO% \
        -xmin 0 -xmax  $::scalerconfig::stripChartOptions(-timeaxis) \
        -ymin 0 -ymax 1 -plottype ::Plotchart::createStripchart \
        -canvas .stripcharts  -xtitle {Run time (seconds)} -ytitle Rate            \
    ]
    # Create empty series.
    #
    foreach series $charts color {black red green blue goldenrod purple cyan yellow orange brown} {
        if {$series eq ""} {
            break
        }
        $::stripcharts series [$series name] [list] [list] $color
        $series clear;                          # Invalidate the time.
    }

    
    #  Resize to the size of the canvas:
    
    $::stripcharts resize [.stripcharts cget -width] [.stripcharts cget -height]
    
    ## TODO: Add resize handler here.
    
    bind .stripcharts <Configure> [list ResizeStripchart $::stripcharts]
    
    
}

#-----------------------------------------------------------------------------
# Main script entry point.

# Start the acquisition thread it will post an event for handleData when items
# of interest are seen in the ring.


if {[array names env SCALER_RING] eq "SCALER_RING"} {
    set uri $env(SCALER_RING)
} else {
    set uri tcp://localhost/$tcl_platform(user)
}
#
#  We support lists of rings in SCALER_RING:
#
foreach ring $uri {
    set acqThread [startAcqThread $ring]
}

# Set up the base graphical user interface:


setupGui


#
# Process the scaler configuration file

set configFile [lindex $argv 0]

source $configFile

# If there's at least one stripchart, add the plot to the display.

set stripItems [_getStripItems]
if {[llength $stripItems] > 0} {
    setupStripchart $stripItems
}

# Set page alarm enables:

enableDisableAlarms .alarmcontrol.enable

