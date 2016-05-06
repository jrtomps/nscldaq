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
# @file   CStatemachineType.h
# @brief  Defines a state machine data type.
# @author <fox@nscl.msu.edu>
*/
#ifndef _CSTATEMACHINETYPE_H
#define _CSTATEMACHINETYPE_H
#include "CEnumType.h"

/**
 * @class CStateMachineType
 *   A state machine data type is an enumerated type whose next values
 *   are constrained by allowed transitions from the current value.  In the
 *   special case where a variable of the state machine type does not have a
 *   current value only the default value (initial state) is allowed.
 *   State machine data types require knowledge of id of the variable they
 *   are working with as well as the normal data base information.
 *   This has necessitated a new parameter to the legal method that
 *   provides the id of the variable being checked.
 *
 *  To prevent tests which work from failing that parameter is defaulted to -1
 *  in base and derived classes -1 means the variable is being created and does
 *  not have a value yet.
 */
class CStateMachineType : public CEnumType {
private:
    CVariableDb& m_db;
public:
    CStateMachineType(int id, std::string typeName, CVariableDb& db);
    virtual ~CStateMachineType();
    
    virtual bool   legal(const char* attempt, int varid=-1) const;
};


#endif