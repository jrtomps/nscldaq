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
    
    CSqliteStatement vtypes(
        db,
        "SELECT COUNT(*) c FROM sqlite_master \
            WHERE type = 'table' AND name = 'variable_types'"
    );
    ++vtypes;
    if(vtypes.getInt(0) == 0) {
    
        CSqliteStatement::execute(
            db,
            "CREATE TABLE variable_types (                            \
                id         INTEGER PRIMARY KEY NOT NULL,              \
                type_name  VARCHAR(256) NOT NULL                      \
            )"
        );
    }    
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