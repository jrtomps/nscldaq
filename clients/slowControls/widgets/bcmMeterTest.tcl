set prefix /scratch/fox/daq/8.1test;    # Change this if --prefix changes.

set libdir [file join $prefix TclLibs]
lappend auto_path $libdir

source meter.tcl
source bcmMeter.tcl

::controlwidget::bcmMeter .bcm  -channel Z026L-C
::controlwidget::bcmMeter .bcm2 -channel Z026R-C
::controlwidget::bcmMeter .bcm3 -channel B159F-C

pack .bcm .bcm2 .bcm3 -side left
