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
# @file   CIntegerType.cpp
# @brief  Implement the integer data type.
# @author <fox@nscl.msu.edu>
*/

#include "CIntegerType.h"
#include <stdlib.h>
#include <string.h>

/**
 * constructor
 */
CIntegerType::CIntegerType(int id) :
    CDataType(id, "integer")
{
    
}



bool
CIntegerType::legal(const char* attempt, int varid) const
{
    char* endptr;
    
    strtol(attempt, &endptr, 0);
    return (endptr - attempt) == strlen(attempt);   // Need the whole string!!!
}
std::string
CIntegerType::defaultValue() const
{
    return "0";
}