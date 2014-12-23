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
# @file   CIntegerType
# @brief  Define the integer data type.
# @author <fox@nscl.msu.edu>
*/

#ifndef _CINTEGERTYPE_H
#define _CINTEGERTYPE_H
#include "CDataType.h"

/**
 * @class CIntegerType
 *    Integer data type.
 */
class CIntegerType : public CDataType
{
public:
    // canonicals:
    
    CIntegerType(int id);
    
    // Interface implementations:
    
    virtual bool        legal(const char* attempt) const;
    virtual std::string defaultValue()             const;
};

#endif