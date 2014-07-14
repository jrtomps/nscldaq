#===================================================================
# class AOrtecCCF8000
#===================================================================

package provide ortecccf8000 1.0

package require cccusb

itcl::class AOrtecCCF8000 {
	protected variable device
	private variable node

  ##
  # Constructor
  # 
  # \param de a CAMAC USB controller (such as cccusb::CCCUSB)
  # \param no the slot in which the device resides
	constructor {de no} {
		set device $de
		set node $no
	}
	
	destructor {}
	
	public method GetVariable {v} {set $v}
	public method WriteThresholds {thres}
	public method WriteWidths {width}
}

##
# Write thresholds
#
# Writes a list of thresholds to the channels. This
# does a symmetric operation for each channel and
# thereby ensures that the same thing written is read back.
#
# \param thres the threshold value
#
itcl::body AOrtecCCF8000::WriteThresholds {thres} {
	for {set i 0} {$i < [llength $thres]} {incr i} {
    
		$device simpleWrite24 $node $i 16 [lindex $thres $i]

		set check [$device simpleRead24 $node $i 0]
		if {$check != [lindex $thres $i]} {
			puts "Failed to set threshold in CCF8000: $check != [lindex $thres $i]"
		}
	}
}


##
# Write widths
#
itcl::body AOrtecCCF8000::WriteWidths {width} {
	$device simpleWrite24 $node 0 17 [lindex $width 0]
	$device simpleWrite24 $node 1 17 [lindex $width 1]
}
