#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#      NSCL DAQ Development Group
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



# @file checkdriver.tcl
#   Provides common base code for tcl drivers for the CC-USB (probably just fine for the CCUSB as
#   well.  This includes
#   - parameter type/range checking.
#   - object conversion (turn swig object id into a usable object).
#

package provide CCUSBDriverSupport 1.0
package require DriverSupport 
package require cccusb 
package require cccusbreadoutlist


# Establish the namespace in which the procs will live.
#
namespace eval ::CCUSBDriverSupport {

}


##
# convertCCUSB
#
# Convert a CCUSB driver id passed into the Tcl code from swig wrappers
# to an object of the same name
#
# @param name - CCUSB object name passed in by swig.
#
# @return string - Name of object useable by Tcl code.
#                  in general this will be the same as the input driver name.
#
proc ::CCUSBDriverSupport::convertCCUSB name {
    cccusb::CCCUSB -this $name
    return $name
}

##
# convertCCUSBReadoutlist
#
#  Convert a CCUSB readout list id passed into the Tcl code from swig wrappers
#  to an object of the same name.
# 
# @param name - CCUSB object name passed in by swig.
#
# @return string -name of an object usable by Tcl code. 
#                 in general this will be $name.
#
proc ::CCUSBDriverSupport::convertCCUSBReadoutList name {
    
    cccusbreadoutlist::CCCUSBReadoutList -this $name
    return $name

}

proc ::CCUSBDriverSupport::convertToReadoutList {atcllist} { 

  set len [llength $atcllist]
  set vecptr [cccusbreadoutlist::vecuint32_create $len ]

  for {set i 0} {$i < $len} {incr i} {
    cccusbreadoutlist::vecuint32_set $vecptr $i [lindex $atcllist $i] 
  }
  
  set rdolist [cccusbreadoutlist::CCCUSBReadoutList rdolist \
                [cccusbreadoutlist::vecuint32_ptr2ref $vecptr]]
  return $rdolist

}


proc ::CCUSBDriverSupport::shortsListToTclList {data_ {grouping 2}} {
  upvar $data_ data

  set pkg cccusb
  
  set nshorts [${pkg}::uint16_vector_size $data]
  if {($nshorts%$grouping) != 0} {
    error "CCUSBDriverSupport::shortsListToTclList size of shorts list must be divisible by 2"
  }
  
  set shift 16
  set intList [list] 
  for {set short 0} {$short < $nshorts} {incr short $grouping} {
    set int 0
    for {set unit 0} {$unit < $grouping} {incr unit} {
      set name short$unit
      set short$unit [${pkg}::uint16_vector_get $data [expr $short+$unit]]
      set int [expr ($int | ($$name<<($shift*$unit)))]
    }
    
    lappend intList $int
  }

  return $intList

}

