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
# @file   CStateMachineTypeCreator.cpp
# @brief  Implements the state machine type family creator class.
# @author <fox@nscl.msu.edu>
*/
#include "CStateMachineTypeCreator.h"
#include "CStateMachineType.h"
#include "CSqlite.h"
#include "CSqliteStatement.h"

class CVariableDb;

#include <stdexcept>

/**
 * constructor
 *    Construct the creator  We need to figure out the type id.
 * @param db  - Reference to the database with the type table.
 * @param typeName - name of the type we will create
 */
CStateMachineTypeCreator::CStateMachineTypeCreator(CSqlite& db, std::string typeName) :
    m_db(db),
    m_typeName(typeName)
{
    
    
    m_typeId = stateMachineId(db, typeName);
    if (m_typeId == -1) {
        throw std::runtime_error("No such state machine type");
    }
    
}
/**
 *
 *  Create a type object for the statemachine:
 */
CDataType*
CStateMachineTypeCreator::operator()()
{
    return new CStateMachineType(m_typeId, m_typeName, reinterpret_cast<CVariableDb&>(m_db));
}
/**
 * Data type description:
 */
std::string
CStateMachineTypeCreator::describe() const
{
    std::string result = "State machine data type: ";
    result += m_typeName;
    return result;
}
/**
 * stateMachineId
 * 
 * @param db - Database reference.
 * @param typeName - Name of data type.
 * @return int    - id of the type.
 * @retval -1     - Type is not a statemachine.
 */
int
CStateMachineTypeCreator::stateMachineId(CSqlite& db, std::string typeName)
{
    // figure out the typeid:
    // The join on the enumerated_values table ensures the type is enumerated and
    // the join on the state_transitions table ensures the transitions have constraints.
    
    CSqliteStatement findId(
        db,
        "SELECT vt.id FROM variable_types vt                            \
            INNER JOIN enumerated_values ev ON ev.type_id = vt.id       \
            INNER JOIN state_transitions st ON st.current_id = ev.id    \
            WHERE vt.type_name=?                                        \
            LIMIT 1                                                     \
        "
    );
    findId.bind(1, typeName.c_str(), -1, SQLITE_TRANSIENT);
    ++findId;
    
    if (findId.atEnd()) {
        return -1;
    }
    
    return findId.getInt(0);
}