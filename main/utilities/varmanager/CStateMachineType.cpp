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
# @file   CStateMachineType.cpp
# @brief  Implement the type checker for the state machine type.
# @author <fox@nscl.msu.edu>
*/

#include "CStateMachineType.h"
#include <CSqliteStatement.h>
#include <CVariableDb.h>

#include <iostream>


/**
 * constructor
 *   @param id        - Type id.
 *   @param typeName  - Name of the type.
 *   @param db        - Database reference.
 */
CStateMachineType::CStateMachineType(int id, std::string typeName, CVariableDb& db):
    CEnumType(id, typeName, db),
    m_db(db)
{
        
}

/**
 * Destructor
 */
CStateMachineType::~CStateMachineType() {}

/**
 * legal - determine legality of a new state:
 *   If the varid is -1 the only legal state is the default state.
 *   otherwise the current variable value determines the legal next states.
 *   
 * @param attempt - desired new value
 * @param varid   - Id of the variable being modified.
 * @return bool   - True if attempt is legal.
 */
bool
CStateMachineType::legal(const char* attempt, int varid) const
{
    if (varid == -1) {
        return std::string(attempt) == defaultValue();
    }
    
    CSqliteStatement c (
        m_db,
        "SELECT COUNT(*) FROM state_transitions t                       \
            INNER JOIN enumerated_values vf ON vf.id = t.current_id     \
            INNER JOIN enumerated_values vt ON vt.id = t.next_id        \
            INNER JOIN variable_types    types ON types.id = vt.type_id \
            INNER JOIN variables         v     ON v.type_id = types.id  \
            WHERE v.id = ?                                              \
            AND   vt.value = ?                                          \
            AND   vf.value = v.value                                    \
        "
    );
    c.bind(1, varid);
    c.bind(2, attempt, -1, SQLITE_TRANSIENT);
    ++c;
    
 
    
    bool result = c.getInt(0) == 1;   
    return result;
    
}