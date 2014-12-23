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
# @file   CStringType.h
# @brief  Data type for variables that take string values.
# @author <fox@nscl.msu.edu>
*/
#ifndef _CSTRINGTYPE_H
#define _CSTRINGTYPE_H
#include "CDataType.h"

/**
 * @class CStringType
 *    data type class for string valued variables
 *    - All values are legal.
 *    - The default value is the empty string.
 */
class CStringType : public CDataType {
    
    // Canonicals
public:
    CStringType(int id);
    
    // methods required by CDataType of concrete derivations:
    
public:
    virtual bool        legal(const char* attempt) const;
    virtual std::string defaultValue()             const;
};


#endif