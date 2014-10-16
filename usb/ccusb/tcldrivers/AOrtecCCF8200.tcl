#===================================================================
# class AOrtecCCF8200
#===================================================================
#
package provide ortecccf8200 11.0

package require Itcl
package require Utils
package require CCUSBDriverSupport



itcl::class AOrtecCCF8200 {

	protected variable device
	private variable node

	constructor {de no} {
		set device $de
		set node $no
	}
	
	destructor {}
	
	public method GetVariable {v} {set $v}

  ## @brief 
	public method WriteThresholds {thres}
	public method WriteWidths {width}



  public method WriteThreshold {ch thresh} 
  public method WriteAWidth {width} 
  public method WriteBWidth {width} 
  public method WriteInhibitMask {mask} 

  public method ReadThreshold {ch}
  public method ReadAWidth {}
  public method ReadBWidth {}
  public method ReadInhibitMask {}

  public method Clear {}
  public method GenerateTest {}

  public method sWriteThreshold {stack ch thresh} 
  public method sWriteAWidth {stack width} 
  public method sWriteBWidth {stack width} 
  public method sWriteInhibitMask {stack mask} 

  public method sReadThreshold {stack ch} 
  public method sReadAWidth {stack} 
  public method sReadBWidth {stack} 
  public method sReadInhibitMask {stack} 

  public method sClear {stack} 
  public method sGenerateTest {stack} 


  public method Execute {grouping script}
}

###############################################################################
#
itcl::body AOrtecCCF8200::WriteThreshold {ch thresh} { 

  if {[catch {Execute 1 [list sWriteThreshold $ch $thresh]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::WriteThreshold]
    return -code error [join $err " "]
  }

  return $msg
}

itcl::body AOrtecCCF8200::WriteAWidth {width} { 

  if {[catch {Execute 1 [list sWriteAWidth $width]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::WriteAWidth]
    return -code error [join $err " "]
  }

  return $msg
}

itcl::body AOrtecCCF8200::WriteBWidth {width} { 

  if {[catch {Execute 1 [list sWriteBWidth $width]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::WriteBWidth]
    return -code error [join $err " "]
  }

  return $msg
}

itcl::body AOrtecCCF8200::WriteInhibitMask {mask} { 

  if {[catch {Execute 1 [list sWriteInhibitMask $mask]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::WriteInhibitMask]
    return -code error [join $err " "]
  }

  return $msg
}

itcl::body AOrtecCCF8200::ReadThreshold {ch} { 

  if {[catch {Execute 2 [list sReadThreshold $ch]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::ReadThreshold]
    return -code error [join $err " "]
  }

  return $msg
}

itcl::body AOrtecCCF8200::ReadAWidth {} { 

  if {[catch {Execute 2 [list sReadAWidth]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::ReadAWidth]
    return -code error [join $err " "]
  }

  return $msg
}

itcl::body AOrtecCCF8200::ReadBWidth {} { 

  if {[catch {Execute 2 [list sReadBWidth]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::ReadBWidth]
    return -code error [join $err " "]
  }

  return $msg
}


itcl::body AOrtecCCF8200::ReadInhibitMask {} { 

  if {[catch {Execute 2 [list sReadInhibitMask]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::ReadInhibitMask]
    return -code error [join $err " "]
  }

  return $msg
}

itcl::body AOrtecCCF8200::Clear {} { 

  if {[catch {Execute 1 [list sClear]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::Clear]
    return -code error [join $err " "]
  }

  return $msg
}

itcl::body AOrtecCCF8200::GenerateTest {} { 

  if {[catch {Execute 1 [list sGenerateTest]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::GenerateTest]
    return -code error [join $err " "]
  }

  return $msg
}
#-----------------------------------------------------------------------------#


itcl::body AOrtecCCF8200::sWriteThreshold {stack ch thresh} {
  if {![Utils::isInRange 0 7 $ch]} {
    set msg {AOrtecCCF8200::sWriteThreshold Channel argument out of bounds. }
    append msg {Must be in range [0,7].}
    return -code error $msg
  }

  if {![Utils::isInRange 0 255 $thresh]} {
    set msg {AOrtecCCF8200::sWriteThreshold Threshold argument out of bounds. }
    append msg {Must be in range [0,255].}
    return -code error $msg
  }

  $stack addWrite24 $node $ch 16 $thresh
}


itcl::body AOrtecCCF8200::sWriteAWidth {stack width} {
  if {![Utils::isInRange 0 255 $width]} {
    set msg {AOrtecCCF8200::sWriteAWidth Width argument out of bounds. }
    append msg {Must be in range [0,255].}
    return -code error $msg
  }

  $stack addWrite24 $node 0 17 $width
}


itcl::body AOrtecCCF8200::sWriteBWidth {stack width} {
  if {![Utils::isInRange 0 255 $width]} {
    set msg {AOrtecCCF8200::sWriteBWidth Width argument out of bounds. }
    append msg {Must be in range [0,255].}
    return -code error $msg
  }

  $stack addWrite24 $node 1 17 $width
}


itcl::body AOrtecCCF8200::sWriteInhibitMask {stack mask} {
  if {![Utils::isInRange 0 0xff $mask]} {
    set msg {AOrtecCCF8200::sWriteInhibitMask Mask sets bits other than bits 0-7. }
    return -code error $msg
  }

  $stack addWrite24 $node 2 17 $mask
}


itcl::body AOrtecCCF8200::sReadThreshold {stack ch} {
  if {![Utils::isInRange 0 7 $ch]} {
    set msg {AOrtecCCF8200::sReadThreshold Channel argument out of bounds. }
    append msg {Must be in range [0,7].}
    return -code error $msg
  }

  $stack addRead24 $node $ch 0 0 ;# no lam wait
}



itcl::body AOrtecCCF8200::sReadAWidth {stack} {
  $stack addRead24 $node 0 1 0 ;# no lam wait
}


itcl::body AOrtecCCF8200::sReadBWidth {stack} {
  $stack addRead24 $node 1 1 0 ;# no lam wait
}


itcl::body AOrtecCCF8200::sReadInhibitMask {stack} {
  $stack addRead24 $node 2 1 0 ;# no lam wait
}


itcl::body AOrtecCCF8200::sClear {stack} {
  $stack addControl $node 0 9 
}

itcl::body AOrtecCCF8200::sGenerateTest {stack} {
  $stack addControl $node 0 25 
}



#-----------------------------------------------------------------------------#

itcl::body AOrtecCCF8200::WriteThresholds {thres} {
  if {[llength $thres] != 8} {
    set msg "AOrtecCCF8200::WriteThresholds Fewer than 8 threshold values "
    append msg "provided."
    return -code error $msg
  }

	for {set i 0} {$i < [llength $thres]} {incr i} {

    set thr_i [lindex $thres $i] 
		WriteThreshold $i [lindex $thr_i $i]
		set rdback [ReadThreshold $i]
    set decodedRdBack [::CCUSBDriverSupport::lower24Bits $rdback]

    if {$decodedRdBack!=$thr_i} {
      set msg "AOrtecCCF8200::WriteThresholds Failed to read back value "
      append msg "written for channel $i. Wrote $thr_i, Read $decodedRdBack"
			return -code error $msg
		}

	}
}



itcl::body AOrtecCCF8200::WriteWidths {width} {
	WriteAWidth [lindex $width 0]
	WriteBWidth [lindex $width 1]
}

# A utility to facility single-shot operation evaluation
#
# Given a list whose first element is a proc name and subsequent elements are
# arguments (i.e. procname arg0 arg1 arg2 ...) , this creates a stack
#
#  procname stack arg0 arg1 arg2 ...
#  
#
itcl::body AOrtecCCF8200::Execute {grouping script} {


# create a new readout list
  set rdoList [cccusbreadoutlist::CCCUSBReadoutList %AUTO%]

  # extract the proc we want to use to manipulate the stack
  set cmd [lindex $script 0]

  # if there are arguments provided, use them. otherwise, dont.
  if {[llength $script]>1} { 
    $cmd $rdoList {*}[lreplace $script 0 0] 
  } else {
    $cmd $rdoList 
  }

  # At this point the list will contain some commands added by the cmd
  # command

  # execute the list
  set data [$device executeList $rdoList [expr 4<<20]] 

  # the returned data is actually a std::vector<uin16_t> wrapped by swig. 
  # Convert this into a list of 32-bit integers and return it as a tcl list
  return [::CCUSBDriverSupport::shortsListToTclList data $grouping]

}

