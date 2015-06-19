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
# @file   title.cpp
# @brief  Set/show title.
# @author <fox@nscl.msu.edu>
*/

#include "titleopts.h"
#include "CStateManager.h"
#include <iostream>

int main(int argc, char** argv)
{
    // process arguments:
    // Note the title is present if there are 'file' parameters..otherwise
    // we just look at it:
    
    struct gengetopt_args_info pa;
    cmdline_parser(argc, argv, &pa);
    
    CStateManager sm(pa.request_uri_arg, pa.subscribe_uri_arg);
    
    if(pa.inputs_num == 0) {
        std::cout << sm.title() << std::endl;
    } else {
        std::string newTitle;
        for (int i = 0; i < pa.inputs_num; i++) {
            newTitle += pa.inputs[i];
            newTitle += " ";
        }
        // There will be a trailing blank but who cares.
        
        sm.title(newTitle.c_str());
    }
    return EXIT_SUCCESS;
}
