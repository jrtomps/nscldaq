set prefix /scratch/fox/daq/8.1test;    # Change this if --prefix changes.

set libdir [file join $prefix TclLibs]
lappend auto_path $libdir

set channel B159F-C;                   # Change this for different bcm.

source meter.tcl
source bcmMeter.tcl

::controlwidget::bcmMeter .bcm  -channel $channel

pack .bcm

