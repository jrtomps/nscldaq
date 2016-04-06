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
# @file   preend.cpp
# @brief  Perform the preend operation - prepare the system to end the run.
# @author <fox@nscl.msu.edu>
*/

#include "preendopts.h"

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

    // The state must be one of paused or active:
    
     std::string gState = sm.getGlobalState();
    if ((gState != "Active") && (gState != "Paused") ) {
        std::cerr << "The preend command requires that the run be in either the ";
        std::cerr <<"'Active' or 'Paused' states but it is in the '" << gState;
        std::cerr << "' state now\n";
        std::exit(EXIT_FAILURE);
    }
    
    // Initiate the ending transition:
    
    try {
        transition(sm, "Ending", verbose);
    }
    catch (std::exception& e) {
        std::cerr << "preend failed:  " << e.what() << std::endl;
        std::cerr << "Failing the entire system\n";
        try {
            sm.setGlobalState("NotReady");
        } catch (...) { }
        std::exit(EXIT_FAILURE);
    }
    
    return EXIT_SUCCESS;
}