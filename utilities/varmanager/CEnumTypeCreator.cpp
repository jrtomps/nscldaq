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
# @file   CEnumTypeCreator.cpp
# @brief  Implementation of a creator for CEnumType objects
# @author <fox@nscl.msu.edu>
*/


#include "CEnumTypeCreator.h"
#include "CEnumeration.h"
#include "CEnumType.h"

class CVariableDb;

/**
 * construtor
 *    -  Initialize m_db, m_typeName
 *    -  Look up the enum id (throw if it does not exist.)
 *    -  Set m_id from that.
 * @param db - Reference to a database object
 * @param name - Name of the enumerator.
 *
 * @throw CEnumeration::CException
 */
CEnumTypeCreator::CEnumTypeCreator(CSqlite& db, std::string enumName) :
    m_db(db),
    m_typeName(enumName)
{
    m_id = CEnumeration::id(reinterpret_cast<CVariableDb&>(db), m_typeName.c_str());
    
}
/** operator()
 *    Create a new CEnumType object that represents our data type.
 *
 *  @return CDataType*
 */
CDataType*
CEnumTypeCreator::operator()()
{
    return new CEnumType(m_id, m_typeName, reinterpret_cast<CVariableDb&>(m_db));
}
/**
 * describe
 *   Describe what we can do:
 *
 *  @return std::string
 */
std::string
CEnumTypeCreator::describe() const
{
    std::string result = "Enumerated data type: ";
    result            += m_typeName;
    return result;
}