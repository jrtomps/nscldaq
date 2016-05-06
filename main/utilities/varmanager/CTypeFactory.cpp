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
# @file   CTypeFactory.cpp
# @brief  Implement the extensible factory for variable data types.
# @author <fox@nscl.msu.edu>
*/
#include "CTypeFactory.h"
#include <CSqlite.h>
#include <CSqliteStatement.h>
#include <set>

// Default type crators:

#include "CIntegerTypeCreator.h"
#include "CRealTypeCreator.h"
#include "CStringTypeCreator.h"

#include "CUnknownTypeHandler.h"
#include "CEnumTypeFamilyHandler.h"
#include "CStateMachineTypeFamilyHandler.h"


template<class T>
static bool inline in(T item, std::set<T> theset) {
    return theset.count(item) > 0;
}



/**
 * constructor:
 *  Save the database handle.
 *  Register the default types.
 *
 *  @param db - Sqlite database handle reference.
 */
CTypeFactory::CTypeFactory(CSqlite& db) :
    m_db(db)
{   
    // Register the default type creators:
    
    registerDefaultCreators();
    registerDefaultUnknownHandlers();
    
}

/**
 *  createSchema
 * Create the schema needed to describe the variable types:
 *
 * @param db - Reference to an sqlite database on which to create the schema.
 */

void
CTypeFactory::createSchema(CSqlite& db)
{
    // Only create the schema if it does not already exist:
    
    
    CSqliteStatement::execute(
        db,
        "CREATE TABLE IF NOT EXISTS variable_types (             \
            id         INTEGER PRIMARY KEY NOT NULL,              \
            type_name  VARCHAR(256) NOT NULL                      \
        )"
    );

}
/**
 * addUnknownTypeHandler
 *    Adds a handler for unknown data types. See the commands in
 *    CUnknownTypeHandler.h for more about these.
 *
 * @param pHandler - pointer to the handler to add
 */
void
CTypeFactory::addUnknownTypeHandler(CUnknownTypeHandler* pHandler)
{
    m_typeUnknownHandlers.push_back(pHandler);
}
/**
 * create
 *    The factory method
 *    *   If the base class create produces a result we're done.
 *    *   If not we try the set of unknown handlers until they are either
 *        exhausted or one of them produces a result.
 * @param type  - data type name to create (e.g. "integer" or "derivedtype")
 * @return CDataType*
 * @retval If can't make one the null is returned.
 */
CDataType*
CTypeFactory::create(std::string type)
{
    CDataType* result = CExtensibleFactory<CDataType>::create(type);
    if (result) return result;
    
    std::list<CUnknownTypeHandler*>::iterator p = m_typeUnknownHandlers.begin();
    while (p != m_typeUnknownHandlers.end()) {
        result = (*p)->create(type.c_str(), m_db, *this);
        if(result) return result;
        p++;
    }
    return 0;
}

/*--------------------------------------------------------------------------
 * private utilities
 */

/**
 * registerDefaultCreators
 *    Create and register the default type creators.  Construction is sufficient
 *    to add them to variable_types table
 */
void
CTypeFactory::registerDefaultCreators()
{
    addCreator("integer", new CIntegerTypeCreator(m_db));
    addCreator("real",    new CRealTypeCreator(m_db));
    addCreator("string",  new CStringTypeCreator(m_db));
}
/**
 * registerDefaultUnknownHandlers
 *    Register the family handlers for all the normal data type families.
 */
void
CTypeFactory::registerDefaultUnknownHandlers()
{
     addUnknownTypeHandler(
        new CStateMachineTypeFamilyHandler(m_db)
    );
    addUnknownTypeHandler(new CEnumTypeFamilyHandler(m_db));
   
}