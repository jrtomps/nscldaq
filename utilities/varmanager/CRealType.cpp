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
# @file   CRealType.cpp
# @brief  Implement the real data type class.
# @author <fox@nscl.msu.edu>
*/
#include "CRealType.h"

#include <stdlib.h>
#include <string.h>


/**
 * constructor
 *  @param id  - the id of the type (database id).
 */
CRealType::CRealType(int id) :
    CDataType(id, "real")
{}

/**
 * legal
 *   @param attempt - a value someone is attempting to assign to a real.
 *   @return bool - true if the value is a legal real string-rep false otherwise.
 */
bool
CRealType::legal(const char* attempt) const
{
   char* endpointer;
   strtod(attempt, &endpointer);
   return (endpointer - attempt) == strlen(attempt);
}
/**
 * defaultValue
 * @return std::string - the default value to give a real valued variable if not
 *                       initialized
 */
std::string
CRealType::defaultValue()             const
{
    return "0.0";
}
