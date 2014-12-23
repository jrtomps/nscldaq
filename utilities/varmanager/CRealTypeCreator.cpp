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
# @file   CRealTypeCreator.cpp
# @brief  Implement the creator for real data types.
# @author <fox@nscl.msu.edu>
*/

#include "CRealTypeCreator.h"
#include "CRealType.h"

/**
 * constructor
 *    Construct the base class and get our type id (which may register it).
 * @param db - references an sqlite database.
 */
CRealTypeCreator::CRealTypeCreator(CSqlite& db) :
    CDataTypeCreatorBase(db),
    m_typeId(lookupId("real"))
{}

/**
 * operator()
 *   The creational.
 * @return CDataType*  - pointer to a new CRealType object.
 */
CDataType*
CRealTypeCreator::operator()()
{
    return new CRealType(m_typeId);
}
/**
 * describe
 *   @return std::string - description of what we're creating.
 */
std::string
CRealTypeCreator::describe()
{
    return "real";
}
