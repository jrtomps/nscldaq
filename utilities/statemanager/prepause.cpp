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
# @file   prepause.cpp
# @brief  Prepare the system for a pause.
# @author <fox@nscl.msu.edu>
*/

#include "prepauseopts.h"
#include "transition.h"
#include "tracker.h"
#include "CStateManager.cpp"

int main(int argc, char** argv)
{
        // Parse the args and create the stat manager object:
    
    struct gengetopt_args_info pa;
    cmdline_parser(argc, argv, &pa);
    
    CStateManager sm(pa.request_uri_arg, pa.subscribe_uri_arg);
    bool verbose = pa.verbose_flag != 0;
    
    // Require the state to be "Ready"
    
    std::string gstate = sm.getGlobalState();
    if ((gstate != "Active") ) { 
        std::cerr << "To prepause a run the global state must be 'Active' ";
        std::cerr << " however it is: '" << gstate << "'\n";
        std::exit(EXIT_FAILURE);
    }
    // initiate the state transition:
    
        transition(sm, "Pausing", verbose);
    
    return EXIT_SUCCESS;
    
}