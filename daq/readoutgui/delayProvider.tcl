#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#      NSCL Data Acquisition Group 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

##
# @file delayProvider.tcl
# @brief A mechanism for inserting a delay between begins received by 
#        data providers. 
# @author Jeromy Tompkins 
#

package provide Delay_Provider 1.0

# Establish the namespace in which the methods and state live and
# define the state variables.
#

namespace eval ::Delay {
  variable delayTime 0    ;#< The amount to delay
}

#------------------------------------------------------------------------------
#  Public interface of the data source provider.

##
# parameters
#    Returns the set of parameters that are required to create the source.
#    For now these are:
#    - delay  - The time to delay in milliseconds 
#
proc ::Delay::parameters {} {
    return [dict create delay [list {milliseconds to delay}]]
}

##
# start
#   Do nothing.
#
# @param params - Dict containing the parameterization of the source.
#
#
proc ::Delay::start params {
  variable delayTime 0
  set delayTime [dict get $params delay]
}
##
# check
#   Check the status of the connection to the source.
#   Do nothing
#
# @param id - Source id of the provider instance.
# @return always true 
#
proc ::Delay::check id {
  return 1
}
##
# stop
#   Stop the data source.  We don't do anything.
#
# @param id - Id of the source to close.
#
proc ::Delay::stop id {
}
##
# begin
#   Wait for the specified amount of time in the delay parameter
#
# @param id - Source id.
# @param run  - Run number desired.
# @param title - Title desired.
#
proc ::Delay::begin {id run title} {
  variable delayTime
  after $delayTime
}

##
# end
#   Do nothing.
#
# @param id - Id of the source to end.
#
proc ::Delay::end id {
}
##
# capabilities
#   Returns a dict describing the capabilities of the source:
#   - canpause - false
#   - runsHaevTitles - true
#   - runsHaveNumbers - true
#
# @return dict - as described above.
#
proc ::Delay::capabilities {} {
    return [dict create \
        canPause        true    \
        runsHaveTitles  true    \
        runsHaveNumbers true    \
    ]
}
