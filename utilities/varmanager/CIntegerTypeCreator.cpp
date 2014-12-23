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
# @file   CIntegerTypeCreator.cpp
# @brief  Implement the integer type creational.
# @author <fox@nscl.msu.edu>
*/
#include "CIntegerTypeCreator.h"
#include "CIntegerType.h"
#include "CDataType.h"

/**
 * constructor
 *    Create the base class and then use it to figure out what my id is:
 *    @param db - sqlite database.
 */
CIntegerTypeCreator::CIntegerTypeCreator(CSqlite& db) :
    CDataTypeCreatorBase(db),
    m_typeId(lookupId("integer"))
{}


/**
 * operator()
 *    Create a CIntegerType and return it.
 *  @return CDatType* pointer to a newly created integer type object.
 */
CDataType*
CIntegerTypeCreator::operator()()
{
    return new CIntegerType(m_typeId);
}

/**
 * describe
 *  @return std::string description of what we're producing.
 */
std::string
CIntegerTypeCreator::describe()
{
    return "integer";
}