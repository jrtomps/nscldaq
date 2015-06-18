/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   tracker.cpp
# @brief  TransitionCallback that outputs participant state transitions.
# @author <fox@nscl.msu.edu>
*/

#include "tracker.h"
#include <string>
#include "CStateManager.h"

/**
 * tracker
 *    Registerd as the state transition callback in the case
 *    when --verbose is specified.  This just track the
 *    state changes amongst the programs.
 *
 *  @param mgr     - The state manager object.
 *  @param program - Program undergoing transition.
 *  @param state   - New state
 *  @param cd      - callback data --not used.
 */
void
tracker(CStateManager& sm, std::string program, std::string state, void* cd)
{
    std::cout << program << " -> " << state << std::endl;
}

