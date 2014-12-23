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
# @file   CIntegerTypeCreator.h
# @brief  Create integer data types.
# @author <fox@nscl.msu.edu>
*/


#ifndef _CINTEGERTYPECREATOR_H
#define _CINTEGERTYPECREATOR_H
#include "CDataTypeCreatorBase.h"


class CDataType;
class CSqlite;

/**
 * @class CIntegerTypeCreator
 *    Create integer data type objects.
 */
class CIntegerTypeCreator : public CDataTypeCreatorBase
{
    // object state:
private:
    int    m_typeId;                     // Id type in database.
    
    // Canonicals
public:
    CIntegerTypeCreator(CSqlite& db);
    
    // Interface required by CCreator classes:
    
public:
    virtual CDataType* operator()();
    virtual std::string describe();
};

#endif