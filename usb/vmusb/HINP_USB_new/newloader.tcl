package require Vme

#puts "In which slot is the XLM72 located? "
#gets stdin slot
#set slot [expr $slot<<27]

#vme create XLM -device /dev/vme32d32 $slot 0xa00000

proc LoadFPGA {filename XLMslot} {
    set slot [expr $XLMslot<<27]

    vme create XLM -device /dev/vme32d32 $slot 0xa00000

	set file [open $filename r]
	fconfigure $file -translation lf
	set str [read $file]
	set length [string length $str]
	puts ".bit file is $length bytes long"
	binary scan $str c$length bytes
	for {set i 0} {$i < $length} {incr i} {
		if {[getByte $bytes $i] == 255} {
			puts "found first 0xff at byte $i"
			break
		}
	}
	set first [expr $i]
# Request busses A,B,X
	XLM set -l 0x800000 0x10003
# Check that they have been granted
	set check [XLM get -l 0x800000]
	if {$check != 0x10003} {
		puts "Request of busses A, B, and X was NOT granted! ([format %x $check])"
		close $file
	        vme delete XLM
		return
	}
	for {set i $first} {$i < $length} {incr i} {
		set byte [getByte $bytes $i]
		set blow [expr $byte & 0xf]
		set bhigh [expr ($byte &0xf0)/16]
		set llow [expr 4*($blow&0x7) + 128*($blow&0x8)]
		set lhigh [expr 4*($bhigh&0x7) + 128*($bhigh&0x8)]
		set long [expr $llow + 65536*$lhigh]
		set addr [expr ($i-$first)*4]
#		puts "loading byte [format %hx $byte] mapped as long [format %x $long] in addr [format %x $addr]"
		XLM set -l $addr $long
		set check [XLM get -l $addr]
		if {$check != $long} {
			puts "Invalid write to SRAM"
			puts "long=[format %x $long], check=[format %x $check] at [format %x $addr]"
			close $file
		        vme delete XLM
			return
		}
	}
	puts "Configuration $filename written into SRAM A"
	close $file
# release busses
    XLM set -l 0x800000 0
# Set FPGA boot source to SRAM A
    after 100
	XLM set -l 0x800008 0x10000
	set check [XLM get -l 0x800008]
	if {$check == 0x10000} {
		puts "FPGA boot source set to SRAM A"
	} else {
		puts "FPGA boot source incorrectly set: [format %x $check]"
# 	        vme delete XLM
# 		return
	}
# Boot FPGA
	XLM set -l 0x800004 0x1
	after 1000
	XLM set -l 0x800004 0x0
	puts "FPGA booted!"
	after 10000
# Release busses A, B, X
	XLM set -l 0x800000 0x0
	set check [XLM get -l 0x800000]
	if {$check != 0x0} {
		puts "Busses A, B and X were NOT released! ([format %x $check])"
	} 

    vme delete XLM

}

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
