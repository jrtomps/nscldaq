#===================================================================
##
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2015.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#      NSCL DAQ Development Group 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#
# @file   ACrdcXLM72.tcl
# @author Daniel Bazin and Jeromy Tompkins
# @note   This is a modified version of the original AGD16XLM72 class
#         that was written by Daniel Bazin. It has been updated to use
#         the cvmusb::CVMUSB and cvmusbreadoutlist::CVMUSBReadout methods
#         and to have a format that is more in line with other NSCLDAQ 
#         code.

package provide gd16xlm72 1.0

package require Itcl
package require xlm72

itcl::class AGD16XLM72 {
	inherit AXLM72

	constructor {de sl} {
		AXLM72::constructor $de $sl
	} {}

	public method WriteDelayWidth {ch de wi}
	public method ReadDelayWidth {ch}

	public method WriteBypass {by} 
  public method ReadBypass {}

	public method WriteInspect {in}
	public method ReadInspect {}

	public method Init {filename aname}
}

itcl::body AGD16XLM72::WriteDelayWidth {ch de wi} {
  set offset [expr $ch*4] 
  return [Write fpga $offset [expr $de+($wi<<8)]]
}

itcl::body AGD16XLM72::ReadDelayWidth {ch} {
  set offset [expr $ch*4] 

  set retValue [Read fpga $offset]
  puts [format %x $retValue]

  # parse the results
  set delay [ expr 0xff & $retValue ]
  set width [ expr ($retValue>>8) & 0xff ]

  return [list $delay $width]
}


itcl::body AGD16XLM72::WriteBypass {by} {
  return [Write fpga 68 $by]
}

itcl::body AGD16XLM72::ReadBypass {} {
  return [Read fpga 68]
}

itcl::body AGD16XLM72::WriteInspect {in} {
  return [Write fpga 72 $in]
}

itcl::body AGD16XLM72::ReadInspect {} {
  return [Read fpga 72]
}

itcl::body AGD16XLM72::Init {filename aname} {
	source $filename

	AccessBus 0x10000
	for {set i 1} {$i <= 16} {incr i} {
    set delay [lindex [array get $aname delay$i] 1] 
    set width [lindex [array get $aname width$i] 1]
		WriteDelayWidth $i $delay $width
	}
	WriteBypass [lindex [array get $aname bypass] 1]
	WriteInspect [lindex [array get $aname inspect] 1]
	ReleaseBus
}

