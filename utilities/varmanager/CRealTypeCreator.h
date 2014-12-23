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
# @file   CRealTypeCreator.h
# @brief  Create CRealType objects.
# @author <fox@nscl.msu.edu>
*/
#ifndef _CREALTYPECREATOR_H
#define _CREALTYPECREATOR_H

#include "CDataTypeCreatorBase.h"
class CDataType;
class CSqlite;

/**
 * @class CRealTypeCreator
 *   Creates objects that represent real data type checkers.
 *   If necessary, registers the data type 'real' with the database.
 */
class CRealTypeCreator : public CDataTypeCreatorBase
{
    // object data:
    
private:
    int m_typeId;
    
    // canonicals:
public:
    CRealTypeCreator(CSqlite& db) ;
    
    // Required interfaces for type creators:
    
    CDataType* operator()();
    std::string describe();
};

#endif
