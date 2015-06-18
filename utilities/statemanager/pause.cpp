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
# @file   pause.cpp
# @brief  Pause a run.
# @author <fox@nscl.msu.edu>
*/

#include "pauseopts.h"

#include "CStateManager.h"
#include <iostream>
#include <cstdlib>
#include "tracker.h"

int main(int argc, char** argv)
{
    // Parse the parameters and create the state manager.
    
    struct gengetopt_args_info pa;
    
    cmdline_parser(argc, argv, &pa);
    CStateManager sm(pa.request_uri_arg, pa.subscribe_uri_arg);
    bool verbose = pa.verbose_flag != 0;
    
    // Ensure that we are Active else we can't pause.
    
    std::string gState = sm.getGlobalState();
    if (gState != "Active") {
        std::cerr << "Only runs in the 'Active' state can be paused ";
        std::cerr << "Run state is currently: '" << gState << "'\n";
        std::exit(EXIT_FAILURE);
    }
    
    // If verbose, we need to set up a callback.
    
    CStateManager::TransitionCallback cb = 0;
    if (verbose) {
        cb = tracker;
    }
    
    // Process the transition.
    
    sm.setGlobalState("Pausing");
    try {
        sm.waitTransition(cb, nullptr);
    }
    catch(std::runtime_error& err) {
        std::cerr << "Pause run failed: " << err.what() << std::endl;
        std::cerr << "Failing the system\n";
        try {
            sm.setGlobalState("NotReady");   // brings everything down.
        }
        catch(...) {}                         // Ignore exceptions from that.
        std::exit(EXIT_FAILURE);
    }
}