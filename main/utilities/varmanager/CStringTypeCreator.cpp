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
# @file   CStringTypeCreator.cpp
# @brief  Implementation of creational for string data types.
# @author <fox@nscl.msu.edu>
*/
#include "CStringTypeCreator.h"
#include "CStringType.h"

/**
 * constructor
 *   @param db - sqlite database handle that determines where type registrations
 *               go.
 */
CStringTypeCreator::CStringTypeCreator(CSqlite& db) :
    CDataTypeCreatorBase(db),
    m_typeId(lookupId("string"))
{}

/**
 * operator()
 *    @return CDataType*  - pointer to a newly created CStringType object.
 */
CDataType*
CStringTypeCreator::operator()()
{
    return new CStringType(m_typeId);
}
/**
 * describe
 *    Descsribe the type of thing we create
 *   @return std::string
 */
std::string
CStringTypeCreator::describe() const
{
    return "string";
}
