#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#    Jeromy Tompkins
#	   NSCL
#	   Michigan State University
#	   East Lansing, MI 48824-1321

package provide Utils 1.0

namespace eval Utils {

  ## @brief Test that a value falls with a certain range
  #
  # The range specified is inclusive such that the condition to be tested is
  # low <= val <= high.
  #
  # @param low  lower bound of range 
  # @param high upper bound of range 
  # @param val  value to test 
  #
  # @returns boolean indicating whether value is within the defined range
  proc isInRange {low high val} {
    return [expr {($val >= $low) && ($val <= $high)}]
  }

  ## @brief Check that all of the elements in a list fall within a range
  #
  # This iterates through the list and checks each element for the following
  # condition: low <= element <= high. The algorithm begins at the beginning of
  # the list and keeps checking until either an element is identified that does
  # not satisfy the condition or the end of the list is reached.
  #
  # @param low  lower bound
  # @param high upper bound
  # @param list list of values
  #
  # @returns boolean 
  # @retval 0 - at least one element in list is outside of range
  # @retval 1 - all elements fall within range
  proc listElementsInRange {low high list} {

  # this is innocent until proven guilty
    set result 1 

    # if an element is out of range, flag it and stop looking.
    foreach element $list {
      if {($element<$low) || ($element>$high)} {
        set result 0
        break
      }
    }

    return $result
  }

  ##
  #
  #
  proc sequence {start n {inc 1}} {
    set res [list]
    for {set i 0} {$i<$n} {incr i} {
      lappend res [expr $start+$i*$inc]
    }
    return $res
  }
}
