#
#   Setup the readout software.

# Common thresholds for now:

set thresholds 0
for {set i 0} {$i < 32} {incr i} {
    lappend thrlist $thresholds
}

module adc caenv785 slot 9  
adc config threshold $thrlist multievent true
adc config fastclearwindow 100
readout add adc

module tdc caenv775 slot 10
tdc  config threshold $thrlist multievent true
tdc  config  keepoverflow true
tdc  config commonstart false
tdc  config range 600      ;# ns. 

readout add tdc





# Report on the configuration:

proc DumpConfig {modules} {
    set layout   "%-10s %-10s %-15s %s"
    puts [format $layout Module Type Parameter Value]
    foreach module $modules {
	set name [lindex $module 0]
	puts [format $layout $name \
		             [lindex $module 1] \
                             "" "" ]
	set config [$name cget]
	foreach param $config {
	    puts [format $layout "" "" \
		    [lindex $param 0]  \
		    [lindex $param 1] ]
	}
    }
}


proc ReadOrder {list} {
    set layout "%s             %s"
    puts [format $layout Module Type]
    foreach module $list {
	puts [format $layout \
		[lindex $module 0] \
	        [lindex $module 1] \
             ]
    }
}

puts "Modules and their configurations:"
DumpConfig [module -list]

puts "Module readout order: "
ReadOrder [readout list]











