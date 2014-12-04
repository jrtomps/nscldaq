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
# @file   CVariableDb.cpp
# @brief  Implements the CVariableDb class
# @author <fox@nscl.msu.edu>
*/

#include "CVariableDb.h"
#include <CSqlite.h>
#include <CSqliteStatement.h>

/*----------------------------------------------------------------------------
 * Implement the exception nested class
 */
/**
 * constructor
 *  @param message - the error message
 */
CVariableDb::CException::CException(std::string message) noexcept :
    m_message(message) {}
    
/**
 * copy constructor
 * @param rhs - the exception from which we are constructed.
 */
CVariableDb::CException::CException(const CException& rhs) noexcept :
    m_message(rhs.m_message) {}

/**
 * assignmentCSq
 *  @param rhs - The object being assigned to this.
 *  @return *this
 */
CVariableDb::CException&
CVariableDb::CException::operator=(const CException& rhs) noexcept 
{
    m_message = rhs.m_message;
    return *this;
}

/**
 * what
 *  @return const char*  - the error message string.
 */
const char*
CVariableDb::CException::what() const noexcept
{
    return m_message.c_str();
}


/*----------------------------------------------------------------------------
 * Implement the CVariableDb class itself.
 */

/**
 * create [static]
 *
 *   Create a new database.  This implies:
 *   -  Opening/creating a new Sqlite3 database (must be new)
 *   -  Creating the schema within that database.
 * @param name - path to the database file.
 */
void
CVariableDb::create(const char* name)
{
    CSqlite db(name, CSqlite::create | CSqlite::readwrite);
    createSchema(db);
}

/**
 * constructor
 *    Construct a variable database
 *    - Construct the base class
 *    - Check the validity of the schema.
 *  @param fileName - Path to the existing database file.
 *  @throw CSqliteException (constrution failed).
 *  @throw CVariableDb::Exception - the schema is not valid.
 */
CVariableDb::CVariableDb(const char* fileName) :
    CSqlite(fileName, CSqlite::readwrite)
{
    checkSchema();        
}


/**
 * Destructor
 */
CVariableDb::~CVariableDb() {}

/*--------------------------------------------------------------------------
 * private methods
 */


/**
 * createSchema [private, static]
 *    Create the schema for the database
 *
 *   @param db - the database handle object.
 */
void
CVariableDb::createSchema(CSqlite& db)
{
    CSqliteStatement::execute(
        db,
        "CREATE TABLE directory (                              \
            id         INTEGER PRIMARY KEY NOT NULL,              \
            name       VARCHAR(256) DEFAULT NULL,                 \
            parent_id  INTEGER DEFAULT NULL                       \
        )"
    );
    
    CSqliteStatement::execute(
        db, "INSERT INTO directory (id) VALUES (1)"
    );
}


/**
 * checkSchema [private]
 *   Ensure the required tables exist in a database.
 *   If not throws our CException.
 */
void
CVariableDb::checkSchema()
{
    if (! checkTable("directory")) {
        throw CException("Diretory table missing");
    }
}

/**
 * checkTable
 *   @param table -name of the table.
 *   @return bool - true if the database contains the specified table.
 */
bool
CVariableDb::checkTable(const char* table)
{
    CSqliteStatement s(
        *this,
        "SELECT COUNT(*) c FROM sqlite_master            \
            WHERE type='table' AND name=:name"
    );
    s.bind(1, table, -1, SQLITE_TRANSIENT);
    ++s;
    return (s.getInt(0) > 0);
}