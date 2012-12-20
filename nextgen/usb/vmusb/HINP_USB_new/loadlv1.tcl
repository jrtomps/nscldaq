proc SetupGUI {} {
    global slot

    pack .controls.exit -side left
    pack .controls.fifo -side left
    pack .selector.slotnum -side bottom

    pack .controls -side top
    pack .selector -side top
}

proc LeaveConfig {} {
    pack forget .controls.exit
    pack forget .controls.fifo
    pack forget .selector.slotnum

    pack forget .controls
    pack forget .selector

    vme delete XLM
    #exit
}

proc positionWindow w {
    wm geometry $w +100+100
}

proc configXLM {filename} {
    LoadFPGA $filename
}

#SetupGUI
#source loader.tcl
