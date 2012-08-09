

proc getByte {bytelist item} {
	set byte [lindex $bytelist $item]
	if {$byte < 0} {
		set byte [expr $byte+256]
	}
	return $byte
}

proc SetupGUI {} {
    pack .conttitle.title -side top

    pack .controls.exit -side left
    pack .controls.fifo -side left

    pack .seltitle.title -side top

    pack .selector.seltitX -side bottom
    pack .selector.xlmtype -side bottom
    pack .selector.seltit8 -side bottom

    pack .selector.cratenum -side bottom
    pack .selector.crtit -side bottom
    pack .selector.slotnum -side bottom

    pack .conttitle -side top
    pack .controls -side top
    pack .seltitle -side top
    pack .selector -side top
}

proc LeaveConfig {} {
    pack forget .conttitle.title
    pack forget .controls.exit
    pack forget .controls.fifo
    pack forget .seltitle.title
    pack forget .selector.slotnum

    pack forget .conttitle
    pack forget .controls
    pack forget .seltitle
    pack forget .selector

}
