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
# @file   CStringType.cpp
# @brief  Implement string data type for variables.
# @author <fox@nscl.msu.edu>
*/
#include "CStringType.h"


/**
 * constructor
 *   @param id  - id of the type (database value).
 */
CStringType::CStringType(int id) :
    CDataType(id, "string")
{}

/**
 * legal
 * @param attempt - Something the user wants to assign to a string.
 * @return bool - true - for strings there are no illegal values.
 */
bool
CStringType::legal(const char* attempt) const
{
    return true;
}

/**
 * defaultValue
 *  @return std::string - "" the default string value if none is provided.
 */
std::string
CStringType::defaultValue()             const
{
    return "";
}
