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
# @file   CRealType.h
# @brief  Variable type for real numbers.
# @author <fox@nscl.msu.edu>
*/
#ifndef _CREALTYPE_H
#define _CREALTYPE_H
#include "CDataType.h"

/**
 * @class CrealType
 *    Type class for variables that have real values:
 *    - Validation uses strtod
 *    - Default value is 0.0
 */
class CRealType : public CDataType
{
    // Canonicals:
public:
    CRealType(int id);
    
    // operations implemented by concrete data types:
    
    virtual bool        legal(const char* attempt, int varid=-1) const;
    virtual std::string defaultValue()             const;
};
#endif