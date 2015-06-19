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
# @file   recording.cpp
# @brief  Enable/disable/report recording state.
# @author <fox@nscl.msu.edu>
*/
#include "recordingopts.h"
#include "CStateManager.h"
#include <iostream>
#include <strings.h>
#include <vector>
#include <string>
#include <stdexcept>

/**
 *  Note that we are going to accept several true/false values:
 *
 *  True:  on enabled enable true 1
 *  False: off disabled disable false 0
 */

// True and false value arrays (null terminated pointers).

static const char* trueStrings[] = {
    "on", "enabled", "enable", "true", "1", 0
};
static const char* falseStrings[] = {
    "off", "disabled", "disable", "false", "0", 0    
};


/**
 * isTrue
 *   @param value a possible value string.
 *   @return bool - indicating that the string is True or false.
 *   @throw std::domain_error if the string is not a valid true/false string.
 *   @note strcasecmp is posix compliant.
 */
bool
isTrue(const char* value)
{
    // True?
    
    const char** p = trueStrings;
    
    while(*p) {
        if (strcasecmp(value, *p) == 0) return true;
        p++;
    }
    
    // False?
    
    p = falseStrings;
    
    while(*p) {
        if (strcasecmp(value, *p) == 0) return false;
        p++;
    }
    // none of the above:
    
    throw std::domain_error("Not a valid boolean string");
}

/**
 * entry point
 */
int main(int argc, char** argv)
{
    struct gengetopt_args_info pa;
    cmdline_parser(argc, argv, &pa);
    
    CStateManager sm(pa.request_uri_arg, pa.subscribe_uri_arg);
    
    // We need exactly 0 or 1 parameters:
    
    if (pa.inputs_num == 0) {
        // Display the value:
        
        std::cout << (sm.recording() ? "Enabled" : "Disabled") << std::endl;
    } else if (pa.inputs_num == 1) {
        try {
            bool state = isTrue(pa.inputs[0]);
            sm.recording(state);
        }
        catch (std::domain_error& e) {
            std::cerr << e.what() << std::endl;
            cmdline_parser_print_help();
            std::exit(EXIT_FAILURE);
        }
    } else {
        // illegal
        
        std::cerr <<
            "There can be only one command line parameter - desired state\n";
        cmdline_parser_print_help();
        std::exit(EXIT_FAILURE);
    }
    std::exit(EXIT_SUCCESS);
}