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
# @file   timeout.cpp
# @brief  Set/get the state transition timeout.
# @author <fox@nscl.msu.edu>
*/

#include "timeoutopts.h"
#include "CStateManager.h"
#include <iostream>
#include <stdlib.h>


int main(int argc, char** argv)
{
    // Parse parameters and create the state manager.
    
    struct gengetopt_args_info pa;
    cmdline_parser(argc, argv, &pa);
    
    CStateManager sm(pa.request_uri_arg, pa.subscribe_uri_arg);
    
    if (pa.inputs_num == 0) {
        std::cout << sm.timeout() << std::endl;
    } else if (pa.inputs_num == 1) {
        const char* timeoutstr = pa.inputs[0];
        char* endptr;
        unsigned timeout = strtoul(timeoutstr, &endptr, 0);
        if (endptr == timeoutstr) {
            std::cerr << "The timeout value must be an unsigned integer was: ";
            std::cerr << timeoutstr << std::endl;
            cmdline_parser_print_help();
            std::exit(EXIT_FAILURE);
        }
        sm.timeout(timeout);
    } else {
        // illegal
        
        std::cerr << "Only one parameter is allowed - the timeout in seconds";
        std::exit(EXIT_FAILURE);
    }
    
    return EXIT_SUCCESS;
}