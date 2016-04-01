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
# @file   transition.cpp
# @brief  Common utility to perform a state transition.
# @author <fox@nscl.msu.edu>
*/

#include "transition.h"
#include <CStateManager.h>
#include "tracker.h"
#include <iostream>
#include <stdexcept>

/**
 *  transition
 *     Perform a state transition:
 *
 *    @param sm - state manager api object.
 *    @param pState - name of the desired state.
 *    @param verbose - true if verbose messages go to  stdout/err
*/
void transition(CStateManager& sm, const char* pState, bool verbose)
{
    sm.setGlobalState(pState);
    CStateManager::TransitionCallback cb(0);
    
    if (verbose) {
        cb = tracker;
    }
    try {
        sm.waitTransition(cb, nullptr);
        if (sm.getGlobalState() == "NotReady") {
            std::cerr << "Faled prebegin operation state got changed to NotReady\n";
            std::exit(EXIT_FAILURE);
        }
    }
    catch(std::runtime_error& err) {
        std::cerr << "State transition to  " << pState << " failed "
            << err.what() << std::endl;
        std::cerr << "Failing the system\n";
        try {
            sm.setGlobalState("NotReady");   // brings everything down.
        }
        catch(...) {}                         // Ignore exceptions from that.
        std::exit(EXIT_FAILURE);
    }
    
}
