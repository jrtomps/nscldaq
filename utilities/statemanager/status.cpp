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
# @file   status.cpp
# @brief  Implement the status command.
# @author <fox@nscl.msu.edu>
*/

#include "statusopts.h"
#include "CStateManager.h"

#include <iostream>

int main(int argc, char**argv)
{
    struct gengetopt_args_info pa;
    cmdline_parser(argc, argv, &pa);
    
    CStateManager sm(pa.request_uri_arg, pa.subscribe_uri_arg);
    
    // We always want the global state:
    
    std::string globalState = sm.getGlobalState();
    
    std::cout << "Global State: " << globalState << std::endl;
    
    // If the --all flag is given we want the other participants.
    
    if (pa.all_flag) {
        std::vector<std::pair<std::string, std::string> > pStates =
            sm.getParticipantStates();
        if (pStates.size() > 0) {
            std::cout << "---------\nParticipant program States:\n";
            std::cout.width(32);
            std::cout << std::left;
        }
        for (int i = 0; i < pStates.size(); i++) {
            std::cout.width(32);
            std::cout << std::left;
            std::cout << pStates[i].first << " " << pStates[i].second <<
                std::endl;
        }
    }
}