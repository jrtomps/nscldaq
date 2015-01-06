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
# @file   CStringTypeCreator.h
# @brief  Creates string variable types.
# @author <fox@nscl.msu.edu>
*/

#ifndef _CSTRINGTYPECREATOR_H
#define _CSTRINGTYPECREATOR_H

#include "CDataTypeCreatorBase.h"
class CDataType;
class CSqlite;

/**
 * @class CStringTypeCreator
 *    Creator for CStringType.h  Also registers the string variable type
 *    as needed.
 */
class CStringTypeCreator : public CDataTypeCreatorBase
{
    // Object data
private:
    int  m_typeId;
    
    // Canonicals
    
public:
    CStringTypeCreator(CSqlite& db);
    
    // Methods required by the base class:
    
    virtual CDataType* operator()();
    virtual std::string describe() const;
};


#endif