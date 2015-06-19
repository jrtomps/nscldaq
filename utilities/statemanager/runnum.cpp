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
# @file   runnum.cpp
# @brief  set/display the run number.
# @author <fox@nscl.msu.edu>
*/
#include "programdiropts.h"
#include "CStateManager.h"
#include <iostream>
#include <cstdlib>


int main(int argc, char** argv)
{
    // Process the parameters:
    
    struct gengetopt_args_info pa;
    cmdline_parser(argc, argv, &pa);
    CStateManager sm(pa.request_uri_arg, pa.subscribe_uri_arg);
    
    //What we do depends on the number of un named params:
    
    if (pa.inputs_num == 0) {
        std::cout << sm.runNumber() << std::endl;
    } else if (pa.inputs_num == 1) {
        char* endPtr;
        unsigned run = std::strtoul(pa.inputs[0], &endPtr, 0);
        if (endPtr == pa.inputs[0]) {
            std::cerr << "Run Number must be an unsigned integer was " <<
                pa.inputs[0] << std::endl;
            std::exit(EXIT_FAILURE);
        }
        sm.runNumber(run);
    } else {
        std::cerr << "Only one parameter, a run number is allowed\n";
        std::exit(EXIT_FAILURE);
    }
    
    return EXIT_SUCCESS;
}