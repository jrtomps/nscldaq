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
# @file   resume.cpp
# @brief  Command to resume a paused run.
# @author <fox@nscl.msu.edu>
*/

#include "resumeopts.h"

#include "CStateManager.h"
#include <iostream>
#include <cstdlib>
#include "tracker.h"
#include "transition.h"

int main(int argc, char** argv)
{
    // Parse the args and create the stat manager object:
    
    struct gengetopt_args_info pa;
    cmdline_parser(argc, argv, &pa);
    
    CStateManager sm(pa.request_uri_arg, pa.subscribe_uri_arg);
    bool verbose = pa.verbose_flag != 0;

    // The state must be paused to resume:
    
    std::string gState = sm.getGlobalState();
    if ((gState != "Paused")  && (gState != "Resuming")){
        std::cerr << "Only runs in the 'Paused' or 'Resuming states can be resumed. ";
        std::cerr << "The state is now '" << gState << "'\n";
        std::exit(EXIT_FAILURE);
    }
    // Figure out the transition callback value:
    
    CStateManager::TransitionCallback cb(0);
    if (verbose) {
        cb = tracker;
    }
    
    try {
        // If nobody did a pre-resume:
        
        if (gState == "Paused")  {
            transition(sm, "Resuming", verbose);
        }
        // Make the run active.
        
        transition(sm, "Active", verbose);
    }
    catch(std::runtime_error& err) {
        std::cerr << "Resume run failed: " << err.what() << std::endl;
        std::cerr << "Failing the system\n";
        try {
            sm.setGlobalState("NotReady");   // brings everything down.
        }
        catch(...) {}                         // Ignore exceptions from that.
        std::exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;

}
