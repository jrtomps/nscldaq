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
# @file CEnumTypeCreator.h
# @brief  Defines a class that can create enumerated data type (checkers).
# @author <fox@nscl.msu.edu>
*/

#ifndef _CENUMTYPECREATOR_H
#define _CENUMTYPECREATOR_H

#include "CTypeFactory.h"


#include <string>

class CSqlite; 

/**
 * @class CEnumTypeCreator
 *     Creates an enumerated type (CEnumType).  The specific type must
 *     already exist and be an enum, therefore this is not a good candidate
 *     for a class derived from CDataTypeCreatorBase.
 */
class CEnumTypeCreator : public CDataTypeCreator
{
private:
    CSqlite&    m_db;
    int         m_id;
    std::string m_typeName;
public:
    CEnumTypeCreator(CSqlite& db, std::string enumName);
    
    virtual CDataType* operator()();
    virtual std::string describe() const;
    
};


#endif