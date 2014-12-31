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
# @file   CEnumType.h
# @brief  Type handler for all enumerated types.
# @author <fox@nscl.msu.edu>
#
*/
#ifndef _CENUMTYPE_H
#define _CENUMTYPE_H

#include "CDataType.h"
#include <set>

class CVariableDb;

/**
 * @class CEnumType
 *   This is a type implementation class for all enumerated types.
 *   it gets instantiated with a CVariableDb object so that it can query the db
 *   for information about the valid values for the specific enumerated type
 *   it represents.
 *
 *   In practice, a creator is registered for each enumerated type that has been
 *   defined.  That creator will instatiate us with the correct type id which,
 *   in turn, determines which set of values in the enumerated_values table
 *   are valid for this specific enumerant.  The value with the lowest id
 *   (inserted first for this type) is default in this implementation.
 */
class CEnumType : public CDataType
{
private:
    CVariableDb&  m_db;
    
    // Canonicals:
public:
    CEnumType(int id, std::string typeName, CVariableDb& db);
    virtual ~CEnumType();
    
    // Interface methods required by CDataType
    
public:
    virtual bool        legal(const char* attempt) const;
    virtual std::string defaultValue() const;

    // Utilities:
private:
    std::set<std::string> getLegalValues() const;
};


#endif
