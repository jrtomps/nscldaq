

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

}
