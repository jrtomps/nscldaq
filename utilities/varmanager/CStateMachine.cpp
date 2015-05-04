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
# @file   CStateMachine.cpp
# @brief  Implement the state machine API.
# @author <fox@nscl.msu.edu>
*/


#include "CStateMachine.h"
#include "CVariableDb.h"
#include "CVariable.h"
#include "CTypeFactory.h"
#include "CDataType.h"
#include "CEnumeration.h"

#include <CSqlite.h>
#include <CSqliteStatement.h>
#include <CSqliteTransaction.h>

CStateMachine::CException::CException(const std::string& msg) :
    std::runtime_error(msg) {}



/**
 * addTransition
 *
 *   Adds a transition to a transition map.  If the state does not exist
 *   it is added to the map.  If the state exists the indicated transition
 *   is added.  It is not an error to add a transition more than once. It does
 *   not have any additional affect.
 *
 *   @param map  - The transition map being affected.
 *   @param from - The from state.
 *   @param to   - the to state.
 */
void
CStateMachine::addTransition(
    CStateMachine::TransitionMap& map, std::string from, std::string to
)
{
    map[from].insert(to);
}
/**
 * create
 *    Create a state machine.  While we'd like to use the enum mechanisms
 *    we need a transaction around the entire creation of the states
 *    and entering of the transitions.  Enumeration creation creates its own
 *    transaction.
 *
 *    TODO: Add support for SAVEPOINTS in the Sqlite++ library since those
 *          can be nested while the straight BEGIN ...COMMIT/ROLLBACK cannot be.
 *
 * @param db          - the database in which we are creating the enum.
 * @param typeName    - Name of the new data type we're creating.
 * @param transitions - The map of state transitions for the state machine.
 *
 * @return int - type id of the new data type.
 * @throw CException if errors are detected.
 */
int
CStateMachine::create(
    CVariableDb& db, std::string typeName,
    CStateMachine::TransitionMap transitions
)
{
    /* Ensure this is a new type name:    */
    
    CTypeFactory f(db);
    CDataType*   pType;
    if (pType = f.create(typeName)) {
        delete pType;
        throw CException("Type by this name already exists");
    }
    
    /* By now unless there are database problems things should work. */
    
    std::vector<std::string>   states;
    std::map<std::string, int> stateIds;
    CStateMachine::TransitionMap::iterator p = transitions.begin();
    while (p != transitions.end()) {
        states.push_back(p->first);
        p++;
    }
    
    CSqliteTransaction t(db);
    try {
        
        
        // Create the new type:
        
        CSqliteStatement makeType(
            db, "INSERT INTO variable_types (type_name) VALUES(?)"
        );
        makeType.bind(1, typeName.c_str(), -1, SQLITE_TRANSIENT);
        ++makeType;
        int typeId = makeType.lastInsertId();
        
        // Create the states, saving their ids:
        
        for (int i =0; i < states.size(); i++) {
            stateIds[states[i]] = addState(db, typeId, states[i]);
        }
        // Add the transitions:
        
        TransitionMap::iterator p = transitions.begin();
        while (p != transitions.end()) {
            int fromId = stateIds[p->first];
            std::set<std::string>::iterator q = p->second.begin();
            
            while (q != p->second.end()) {
                int toId  = stateIds[*q];
                
                // Ids are > 0, if we have 0 it's because of a default
                // construction -- that is the to state does not exist.
                
                if (toId == 0) {
                    throw CException("To state does not exist in transition");    
                }
                
                addTransition(db, fromId, toId);              
                q++;
            }
            
            p++;
        }
        return typeId;
        
    }
    catch(...) {
        t.rollback();              // Problems rollback the transaction...
        throw;                     // and re-throw.
    }
}
/**
 * isStateMachine
 *    Determines if a type id corresponds to a state machine type:
 *
 *  @param db     - Database
 *  @param typeId - Id of the type to check
 *  @return bool  - True if typeId is a statemachine, false if not.
 */
bool
CStateMachine::isStateMachine(CVariableDb& db, int typeId)
{
    /* Throw if the typeId does not identify a type:  */
    
    CSqliteStatement isType(
        db,
        "SELECT COUNT(*) FROM variable_types WHERE id = ?"
    );
    isType.bind(1, typeId);
    ++isType;
    if (isType.getInt(0) == 0) {
        throw CException("typeId does not identify a data type");
    }
    
    /*  We're looking for this type in the enumerated_values tbl
     *  along with associated records in the state_transitions table.
     */
    CSqliteStatement isMachine(
        db,
        "SELECT COUNT(*) FROM enumerated_values v                   \
           INNER JOIN state_transitions t ON t.current_id = v.id    \
           WHERE v.type_id = ?                                      \
        "
    );
    isMachine.bind(1, typeId);
    ++isMachine;
    
    return (isMachine.getInt(0) > 0);
}
/**
 * validNextStates
 *    Given a state machine type and an initial state, provides the
 *    list of valid states that type can transition to.
 *
 *   @param db            - Variable database.
 *   @param typeId        - Id of the type.
 *   @param current       - Value to check transitions against.
 *   @return std::vector<std::string> - each element the name of a valid target
 *                           state
 */
std::vector<std::string>
CStateMachine::validNextStates(
    CVariableDb& db, int typeId, std::string current
)
{
    if (!isStateMachine(db, typeId)) {
        throw CException("Type is not a state machine");
    }
    
    std::vector<std::string> result;
    
    /*
     * et - enumerated values of to states.
     * tf - transitions givent the from state.
     */
    CSqliteStatement nextStates(
        db,
        "SELECT et.value FROM enumerated_values e                           \
            INNER JOIN state_transitions tf ON tf.current_id = e.id         \
            INNER JOIN enumerated_values et   ON et.id       = tf.next_id   \
            WHERE e.value   = ?                                             \
            AND   e.type_id = ?                                             \
            ORDER BY et.value ASC                                           \
        "
    );
    nextStates.bind(1, current.c_str(), -1, SQLITE_TRANSIENT);
    nextStates.bind(2, typeId);
    
    ++nextStates;
    while(!nextStates.atEnd()) {
        result.push_back(reinterpret_cast<const char*>(nextStates.getText(0)));
        ++nextStates;
    }
    
    // If there are no next states this must mean this is an invalid from
    // because the way the state map is built up requires that there be at least
    // one destination for any state.
    
    if (result.size() == 0) {
        throw CException("Invalid from state.");
    }
    
    return result;
}
/**
 * validNextStates (variable)
 *   Get the valid next states for a variable given its id.  There are,
 *   I'm sure, single queries that will give us this, but we're just going
 *   to get the variable's value and type and let this reduce to the
 *   prior overload.
 *
 *   @param db       - database.
 *   @param varid    - variable's id.
 *   @return std::vector<std::string> - each element the name of a valid next
 *                     state.
 */
std::vector<std::string>
CStateMachine::validNextStates(CVariableDb& db, int varid)
{
   CVariable v(db, varid);
   int typeId = v.getTypeId();
   std::string value = v.get();
   
   return validNextStates(db, typeId, value);
}
/**
 * getTransitionMap
 *    Reconstruct the transition map from the type id.
 *
 * @param db     - Database handle.
 * @param typeId - The type id
 */
CStateMachine::TransitionMap
CStateMachine::getTransitionMap(CVariableDb& db, int typeId)
{
    TransitionMap result;
    
    // Get the type name associated with the id:
    
    CSqliteStatement tName(
        db,
        "SELECT type_name FROM variable_types WHERE id=?"
    );
    tName.bind(1, typeId);
    ++tName;
    if (tName.atEnd()) {
        throw CException("No such data type id");
    }
    const char* pTypeName = reinterpret_cast<const char*>(tName.getText(0));
    
    
    // This gets the states because statemachines subclass enums.
    
    std::vector<std::string> states = CEnumeration::listValues(db, pTypeName);
    
    // Just add all next states from all prior states to the map:
    // This also ensures the typeId is a state machine.
    
    for (int i =0; i < states.size(); i++) {
        std::vector<std::string> nextStates = CStateMachine::validNextStates(
            db, typeId, states[i]
        );
        for (int j = 0; j < nextStates.size(); j++) {
            addTransition(result, states[i], nextStates[j]);
        }
    }
    
    return result;
}
/*---------------------------------------------------------------------------
 *
 *  Private utilities
 */

/**
 * addState
 *   Add a new state to the enumerated_values table.
 *
 *   @param db    - Database in which we are working.
 *   @param typeId- Id fo the type this is associated with
 *   @param value - New value to add
 *   @return int  - Id fo the new enumerated value.
 */
int
CStateMachine::addState(CVariableDb& db, int typeId, std::string value)
{
    CSqliteStatement insert(
        db,
        "INSERT INTO enumerated_values (type_id, value) VALUES(?,?)"
    );
    insert.bind(1, typeId);
    insert.bind(2, value.c_str(), -1, SQLITE_TRANSIENT);
    
    ++insert;
    return insert.lastInsertId();
}
/**
 * addTransition
 *   Add a transition to the state_transitions table.
 *   @param db         - Data base we are inserting in.
 *   @param from       - id of from state from enumerated_values.
 *   @param to         - id of to state from enumerated_values.
 */
void
CStateMachine::addTransition(CVariableDb& db, int from, int to)
{
    CSqliteStatement insert(
        db,
        "INSERT INTO state_transitions (current_id, next_id) VALUES(?,?)"
    );
    insert.bind(1, from);
    insert.bind(2, to);
    ++insert;
}