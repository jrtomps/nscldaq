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
# @file   CDataTypeCreatorBase.h
# @brief  Provide common functionality for all data type creators.
# @author <fox@nscl.msu.edu>
*/

#ifndef _CDATATYPECREATORBASE_H
#define _CDATATYPECREATORBASE_H
#include "CTypeFactory.h"

class CSqlite;

/**
 * @class CDataTypeCreatorBase
 *    Provides base class functionality for all data type creators.
 *    The functionality we need is:
 *    - Type registration with the database (if needed).
 *    - Type lookup from the database.
 *
 * @note CDataTypeCreator is a typedef for CCreator<CDataType> defined in
 */
class CDataTypeCreatorBase : public CDataTypeCreator
{
    // Object data:
private:
    CSqlite&   m_db;
    
    // Canonicals:
public:
    CDataTypeCreatorBase(CSqlite& db);
    virtual ~CDataTypeCreatorBase() { }                 // Class is not final.
    
    // Facilities offered to derived classes:
    
public:
    int lookupId(const char* typeName);
    
    //  The following definitions are for testability, as it's pure virtual
public:
    virtual CDataType* operator()() {return 0;}   // So you'd better not use this.
    virtual std::string describe() const {
        return "Creator base class -- derive from this";
    }
    
private:
    int registerType(const char* typeName);
    int getTypeId(const char* typeName);
};

#endif
