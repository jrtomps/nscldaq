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
# @file   programdir.cpp
# @brief  Set/get program directory path.
# @author <fox@nscl.msu.edu>
*/

#include "programdiropts.h"
#include "CStateManager.h"
#include <iostream>
#include <cstdlib>


int main(int argc, char** argv)
{
    // Parse the args and create the state manager
    // object
    
    struct gengetopt_args_info pa;
    cmdline_parser(argc, argv, &pa);
    
    CStateManager sm(pa.request_uri_arg, pa.subscribe_uri_arg);
    
    if(pa.inputs_num == 0) {
        // report
        std::cout << sm.getProgramParentDir() << std::endl;
    } else if (pa.inputs_num == 1) {
        
        try {
            sm.setProgramParentDir(pa.inputs[0]);
        }
        catch (std::exception& e) {
            std::cerr << "Could not set directory: " << e.what() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    } else {
        std::cerr << "Only one parameter allowed - a vardb directory path\n";
        cmdline_parser_print_help();
        std::exit(EXIT_FAILURE);
    }
    std::exit(EXIT_SUCCESS);
}