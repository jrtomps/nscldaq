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
# @file   CStateMachineTypeFamilyHandler.cpp
# @brief  Implement the state machine family handler.
# @author <fox@nscl.msu.edu>
*/

#include "CStateMachineTypeFamilyHandler.h"
#include "CStateMachineTypeCreator.h"
#include "CTypeFactory.h"
#include <CSqlite.h>
#include <CSqliteStatement.h>


/**
 * construction
 *     - Conditionally create the state machine schema.
 * @param db - reference to a CSqlite object.
 */
CStateMachineTypeFamilyHandler::CStateMachineTypeFamilyHandler(CSqlite& db)
{
    CSqliteStatement::execute(
        db,
        "CREATE TABLE IF NOT EXISTS state_transitions (               \
            id          INTEGER PRIMARY KEY NOT NULL,   \
            current_id  INTEGER NOT NULL,               \
            next_id     INTEGER NOT NULL                \
        )"
    );
}

/**
 * create
 *    If the type requested is a statemachine register an appropriate creator
 *    and use it to return the correct type object, otherwise, return null.
 * @param typeName - name of the type we are creating.
 * @param db       - reference to the database.
 * @param factory  - reference to the requesting factory so that we can register the creator.
 * @return CDataType* - Pointer to the created data type.
 * @retval null       - If typeName does not name a statemachine.
 */
CDataType*
CStateMachineTypeFamilyHandler::create(const char* typeName, CSqlite& db, CTypeFactory& factory)
{
    /*
     *  Hey where's the test? I hear you ask.  The statemachine type creator
     *  tests that the typename is a valid statemachine and throws an
     *  std::runtime_exception if it is not.
     */
    try {
        CStateMachineTypeCreator* pCreator = new CStateMachineTypeCreator(db, typeName);
        factory.addCreator(typeName, pCreator);
        return (*pCreator)();
    } catch(...) {
        return 0;
    }
    
}