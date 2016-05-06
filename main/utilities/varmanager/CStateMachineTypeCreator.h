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
# @file   CStateMachineTypeCreator.h
# @brief  Creator for statemachine family data types.
# @author <fox@nscl.msu.edu>
*/
#ifndef _CSTATEMACHINETYPECREATOR_H
#define _CSTATEMACHINETYPECREATOR_H

#include <CTypeFactory.h>


class CSqlite;

class CStateMachineTypeCreator : public CDataTypeCreator
{
private:
    CSqlite&      m_db;
    int           m_typeId;
    std::string   m_typeName;
public:
    CStateMachineTypeCreator(CSqlite& db, std::string typeName);
    
    virtual CDataType* operator()();
    virtual std::string describe() const;
    static int stateMachineId(CSqlite& db, std::string typeName);
};
#endif