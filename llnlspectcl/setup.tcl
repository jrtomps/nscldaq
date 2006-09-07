#
#   Setup the code.

package require configFile




configClear
configRead ~/config/daqconfig.tcl



set paramnum 100

#  Process the adcs..
#
foreach adc [array names adcConfiguration] {

    #  Make the parameters, 1d/2d spectra for this adc
    set basename1 $adc.1.
    set basename2 $adc.2.

    set slot      $adcConfiguration($adc)
    
    for {set i 0} {$i < 16} {incr i} {
	set channel [format %02d $i]


	set name1 $basename1$channel
	set name2 $basename2$channel

	parameter $name1 [incr paramnum]
	parameter $name2 [incr paramnum]

	spectrum $name1 1 $name1 {{0 4095 4096}}
	spectrum $name2 1 $name2 {{0 4095 4096}}
	spectrum $adc.1-vs-2.$channel 2 [list $name1 $name2]  {{0 4095 512} {0 4095 515}}

	# Add the parameter mapping for the decoder.

	paramMap $slot $i $name1
	paramMap $slot [expr $i + 16] $name2

    }
}
sbind -all
.gui.b update
