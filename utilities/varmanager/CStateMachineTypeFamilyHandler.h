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
# @file   CStateMachineTypeFamilyHandler.h
# @brief  Unknown type handler for the state machine family of types.
# @author <fox@nscl.msu.edu>
*/

#ifndef _CSTATEMACHINETYPEFAMILYHANDLER_H
#define _CSTATEMACHINETYPEFAMILYHANDLER_H

#include "CUnknownTypeHandler.h"

/**
 * @class CStateMachineTypeFamilyHandler
 *
 * Unknown type handler that matches type families that are
 * state machines.  Note that since state machines are a specialization of
 * enumerations (they are enums that constrain the next value given the
 * current value), this must be registered prior to the enumerated type family
 * handler.
 */
class CStateMachineTypeFamilyHandler : public CUnknownTypeHandler
{
public:
    CStateMachineTypeFamilyHandler(CSqlite& db);
    CDataType* create(const char* typeName, CSqlite& db, CTypeFactory& factory);
};

#endif
