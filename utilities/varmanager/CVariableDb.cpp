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
        "CREATE TABLE directory (                              \
            id         INTEGER PRIMARY KEY NOT NULL,              \
            name       VARCHAR(256) DEFAULT NULL,                 \
            parent_id  INTEGER DEFAULT NULL                       \
        )"
    );
    
    CSqliteStatement::execute(
        db, "INSERT INTO directory (id) VALUES (1)"
    );
    // The variable types table and the varible types themselves:
    
    CTypeFactory::createSchema(db);
    CTypeFactory factory(db);                 // Registers the types.
    
    // Add the constraint types table:
#if 0  
    CSqliteStatement::execute(
        db,
        "CREATE TABLE constraint_types (                        \
            id               INTEGER PRIMARY KEY NOT NULL,      \
            constraint_name  VARCHAR(256) NOT NULL              \
        )"
    );
    CSqliteStatement::execute(
        db,
        "INSERT INTO constraint_types (constraint_name) VALUES  \
        ('range') "
    );
    
    // Add the constraint application table and define which constraints
    // apply to which types:
    
    CSqliteStatement::execute(
        db,
        "CREATE TABLE constraint_allowed (              \
            type_id           INTEGER NOT NULL,         \
            constraint_id     INTEGER NOT NULL          \
        )"
    );
    allowConstraint(db, "range", "integer");
    allowConstraint(db, "range", "real");
    
    // Define the range_constraint_data table which has limits for
    // the range constraint type:
    
    CSqliteStatement::execute(
        db,
        "CREATE TABLE range_constraint_data (                   \
            id                INTEGER PRIMARY KEY NOT NULL,     \
            low               REAL DEFAULT NULL,                \
            high              REAL DEFAULT NULL                 \
        )"
    );
    // Define the table that holds variable constraints:
    
    CSqliteStatement::execute(
        db,
        "CREATE TABLE constraints (                             \
            id                  INTEGER PRIMARY KEY NOT NULL,   \
            variable_id         INTEGER NOT NULL,               \
            constraint_type_id  INTEGER NOT NULL,               \
            constraint_data_id  INTEGER                         \
        )"
    );
#endif
    // Define the variables table and the table that actualy contains
    // constraints on variables:
    
    CSqliteStatement::execute(
        db,
        "CREATE TABLE variables (                               \
            id                  INTEGER PRIMARY KEY NOT NULL,   \
            name                VARCHAR(256) NOT NULL,          \
            directory_id        INTEGER NOT NULL,               \
            type_id             INTEGER NOT NULL,               \
            value               VARCHAR(256)  NOT NULL          \
        )"
    );
    
    // Defines the enumeated values: TODO:  This should be moved
    // into the chain of responsibility element the factory creates
    // for enum types.
    
    CSqliteStatement::execute(
        db,
        "CREATE TABLE enumerated_values (                       \
            id                INTEGER PRIMARY KEY NOT NULL,     \
            type_id           INTEGER NOT NULL,                 \
            value             VARCHAR(256) NOT NULL             \
        )"
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
    if (! checkTable("variable_types")) {
        throw CException("Variable types table missing");
    }
#if 0
    if (! checkTable("constraint_types")) {
        throw CException("Constraint types table missing");
    }
    if (!checkTable("constraint_allowed")) {
        throw CException("Constraints allowed on data types join table missing");
    }
    if (!checkTable("range_constraint_data")) {
        throw CException("Range constraint data table missing");
    }
    if (!checkTable("constraints")) {
        throw CException("Variable Constraints table missing");
    }
#endif
    if(!checkTable("variables")) {
        throw CException("Variables table missing");
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
/**
 * allowConstraint
 *    Make an entry in the constraint_allowed join table that indicates
 *    a specified constraint type can be applied to a specific
 *    data type:
 * @param db             - database id reference.
 * @param constraintType - Type of constraint
 * @param dataType       - Data type.
 * @throw CVariableDb::CException if error.
 */
void
CVariableDb::allowConstraint(
    CSqlite& db, const char* constraintType, const char* dataType
)
{
    // Get the ids of the constraint and data types:
    
    CSqliteStatement cid(
        db,
        "SELECT id FROM constraint_types WHERE constraint_name = ?"
    );
    CSqliteStatement tid(
        db,
        "SELECT id FROM variable_types WHERE type_name = ?"
    );
    
    cid.bind(1, constraintType, -1, SQLITE_TRANSIENT);
    ++cid;
    if (cid.atEnd()) {
        throw CException("Constraint type does not exist in allowConstraint");
    }
    int constraintId = cid.getInt(0);
    
    tid.bind(1, dataType, -1, SQLITE_TRANSIENT);
    ++tid;
    if (cid.atEnd()) {
        throw CException("Data type does not exist in allowConstraint");
    }
    int typeId = tid.getInt(0);
    
    // Insert the join table entry:
    
    CSqliteStatement add(
        db,
        "INSERT INTO constraint_allowed (type_id, constraint_id)  \
           VALUES (?,?)"
        
    );
    add.bind(1, typeId);
    add.bind(2, constraintId);
    ++add;
    
    
}