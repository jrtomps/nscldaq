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
# @file   begin.cpp
# @brief  Command utility to begin a run.
# @author <fox@nscl.msu.edu>
*/


#include "beginopts.h"

#include "CStateManager.h"
#include <iostream>
#include <cstdlib>
#include "tracker.h"

// Do a state transition/wait.

static void transition(CStateManager& sm, const char* pState, bool verbose)
{
    sm.setGlobalState(pState);
    CStateManager::TransitionCallback cb(0);
    
    if (verbose) {
        cb = tracker;
    }
    try {
        sm.waitTransition(cb, nullptr);
    }
    catch(std::runtime_error& err) {
        std::cerr << "Begin run failed: " << err.what() << std::endl;
        std::cerr << "Failing the system\n";
        try {
            sm.setGlobalState("NotReady");   // brings everything down.
        }
        catch(...) {}                         // Ignore exceptions from that.
        std::exit(EXIT_FAILURE);
    }
    
}

int main(int argc, char** argv)
{
    // Parse the args and create the stat manager object:
    
    struct gengetopt_args_info pa;
    cmdline_parser(argc, argv, &pa);
    
    CStateManager sm(pa.request_uri_arg, pa.subscribe_uri_arg);
    bool verbose = pa.verbose_flag != 0;
    
    // Require the state to be "Ready"
    
    std::string gstate = sm.getGlobalState();
    if (gstate != "Ready") {
        std::cerr << "To begin a run the global state must be 'Ready' ";
        std::cerr << " however it is: '" << gstate << "'\n";
        std::exit(EXIT_FAILURE);
    }
    // initiate the state transition:
    
    transition(sm, "Beginning", verbose);
    transition(sm, "Active", verbose);
    
    return EXIT_SUCCESS;
    
}