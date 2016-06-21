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
#include "CTypeFactory.h"


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
    // The directory table and the root directory.
    
    CSqliteStatement::execute(
        db,
        "CREATE TABLE IF NOT EXISTS directory (                   \
            id         INTEGER PRIMARY KEY NOT NULL,              \
            name       VARCHAR(256) DEFAULT NULL,                 \
            parent_id  INTEGER DEFAULT NULL                       \
        )"
    );
    CSqliteStatement::execute(
       db,
       "CREATE INDEX IF NOT EXISTS directoryname ON directory (name)");
    
    CSqliteStatement::execute(
        db, "INSERT INTO directory (id) VALUES (1)"
    );
    // The variable types table and the varible types themselves:
    
    CTypeFactory::createSchema(db);
    CTypeFactory factory(db);                 // Registers the types.
    

    // Define the variables table.
    
    CSqliteStatement::execute(
        db,
        "CREATE TABLE IF NOT EXISTS variables (                               \
            id                  INTEGER PRIMARY KEY NOT NULL,   \
            name                VARCHAR(256) NOT NULL,          \
            directory_id        INTEGER NOT NULL,               \
            type_id             INTEGER NOT NULL,               \
            value               VARCHAR(256)  NOT NULL          \
        )"
    );
    CSqliteStatement::execute(
       db,
      "CREATE INDEX variablename ON variables (name)"
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
    if (! tableExists("directory")) {
        throw CException("Diretory table missing");
    }
    if (! tableExists("variable_types")) {
        throw CException("Variable types table missing");
    }

    if(!tableExists("variables")) {
        throw CException("Variables table missing");
    }
    
}

