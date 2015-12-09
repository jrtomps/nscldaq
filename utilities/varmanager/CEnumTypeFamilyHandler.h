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
# @file   CEnumTypeFamilyHandler.h
# @brief  Class that sees if an unknown type is an enum.
# @author <fox@nscl.msu.edu>
*/
#ifndef _CENUMTYPEFAMILYHANDLER_H
#define _CENUMTYPEFAMILYHANDLER_H

#include "CUnknownTypeHandler.h"

/**
 * @class CEnumTypeFamilyhandler
 *    If an instance of this class is registered with a type factory, that type
 *    factor will be able to recognize and properly instantiate enumerated type
 *    checkers.
 */
class CEnumTypeFamilyHandler : public CUnknownTypeHandler
{
public:
    CEnumTypeFamilyHandler(CSqlite& db);
    
    CDataType* create(const char* typeName, CSqlite& db, CTypeFactory& factory);
};
#endif