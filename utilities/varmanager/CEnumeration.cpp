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
# @file   CEnumeration.cpp
# @brief  Implement the methods of the CEnumeration class.
# @author <fox@nscl.msu.edu>
*/

#include "CEnumeration.h"
#include "CVariableDb.h"
#include "CSqliteStatement.h"
#include "CSqliteTransaction.h"

#include <set>

/**
 * create
 *    Make a new enumeration data type
 *    -   Throw an exception if the type already exists.
 *    -   Throw an exception if there are duplicate values.
 *    -   Start a transaction
 *    -   Enter the type in the variable_types table.
 *    -   Enter each value in the enumeration_values table.
 *    -   commit the transaction.
 *
 *  @param db       - Reference to the database the enum gets defined in.
 *  @param pTypeName- Name of the new enumeration type.
 *  @param values   - Values the enumerated type can take.
 *  @return int     - Id of the newly created type.
 */
int
CEnumeration::create(
    CVariableDb& db, const char* pTypeName, std::vector<std::string> values
)
{
    // Make sure we're good to go:
    
    if (typeExists(db, pTypeName)) {
        throw CException("CEnumeration::create - Type already exists");
    }
    if (!noDups(values)) {
        throw CException(
            "CEnumeration::create - Duplicate value(s) not allowed"
        );
    }
    if (values.size() == 0) {
        throw CException("CEnumeration::create -- must be at least one value");
    }
    // Do the db operations in a transaction so it's all or nothing:
    
    CSqliteTransaction t(db);
    int id = addType(db, pTypeName);
    for (int i = 0; i < values.size(); i++) {
        addValue(db, id, values[i].c_str());
    }
    
    return id;                              // Commits the transaction.
}

/**
 * id
 *    Return the id of an enumeration type or throw an exception if the type
 *    does not exist, or is not an enumeration.
 *
 *  @param db        - Database to operate on.
 *  @param pEnumName - Name of the enumeration.
 *  @return int - the id of the data type.
 */
int
CEnumeration::id(CVariableDb& db, const char* pEnumName)
{
    // It's the inner join here that ensures the type we got was actually
    // an enumerated type; Limit ensures we don't get all value rows.
    
    CSqliteStatement idGetter(
        db,
        "SELECT t.id FROM variable_types t                     \
           INNER JOIN enumerated_values e ON e.type_id = t.id  \
           WHERE t.type_name = ?                               \
           LIMIT 1                                             "
    );
    idGetter.bind(1, pEnumName, -1, SQLITE_TRANSIENT);
    ++idGetter;
    
    if(idGetter.atEnd()) {
        throw CException("CException::id - no such enumeration data type");
    }
    
    return idGetter.getInt(0);
}

/**
 * addValue
 *    Adds a new value to an enumerated type:
 *    -  Get the id of the enumerated type (throws for all the right reasons).
 *    -  Get the current list of values as a set
 *    -  If the new value is in the set throw.
 *    -  Add the value to the enumerated values.
 *
 * @param db        - database we're operating on.
 * @param pTypeName - Name of the enumerated type.
 * @param value     - New value.
 */
void
CEnumeration::addValue(CVariableDb& db, const char* pTypeName, const char* value)
{
    int enumId = id(db, pTypeName);
    
    std::set<std::string> existingValues = values(db, enumId);
    if (existingValues.find(std::string(value)) != existingValues.end()) {
        throw CException(
            "CEnumeration::addValue -- value already exists for this enum"
        );
    }
    
    addValue(db, enumId, value);
}

/*------------------------------------------------------------------------
 * Utility methods:
 */
/** typeExists
 *    @param db        - Database we're working with.
 *    @param pTypeName - Type we want to know about.
 *    @return bool - true if the type exists.
 */
bool
CEnumeration::typeExists(CVariableDb& db, const char* pTypeName)
{
    CSqliteStatement typeFind(
        db,
        "SELECT * FROM variable_types WHERE type_name = ?"
    );
    typeFind.bind(1, pTypeName, -1, SQLITE_TRANSIENT);
    ++typeFind;
    return !typeFind.atEnd();                    // Exists if there is a record.
}
/**
 * addType
 *   Add a new type to the type table.. It's the caller's responsibility to ensure
 *   there are no duplicate types.
 *
 *  @param db        - Database we're working with.
 *  @param pTypeName - Name of the type to add.
 *  @return int      - Id of the new type.
 */
int
CEnumeration::addType(CVariableDb& db, const char* pTypeName)
{
    CSqliteStatement add(
        db,
        "INSERT INTO variable_types (type_name) VALUES (?)"
    );
    add.bind(1, pTypeName, -1, SQLITE_TRANSIENT);
    ++add;
    return add.lastInsertId();
}
/**
 * addValue
 *    Add a new enumeration value.
 *  @param db     - Data base we're working on
 *  @param typeId - the type id the value is being added for.
 *  @param value  - the new value.
 */
void
CEnumeration::addValue(CVariableDb& db, int typeId, const char* value)
{
    CSqliteStatement add(
        db,
        "INSERT INTO enumerated_values (type_id, value) VALUES(?,?)"
    );
    add.bind(1, typeId);
    add.bind(2, value, -1, SQLITE_TRANSIENT);
    
    ++add;
}
/**
 * noDups
 *    Determines if a bunch of values has any duplicates.
 *    We just convert the input vector to a set and there are no
 *    duplicates if the resulting sizes are the same:
 *
 *    @param values - values to check for dups.
 *    @return bool  - true if there are no dups.
 */
bool
CEnumeration::noDups(std::vector<std::string> values)
{
    std::set<std::string> uniqueValues;
    uniqueValues.insert(values.begin(), values.end());
    
    return uniqueValues.size() == values.size();
}
/**
 * values
 *    Returns the value set for an enumeration given its id.
 *    It's assumed that we already know this is an enumeration.
 *
 *  @param db    - database we're operating with.
 *  @param typeId - Type id of the enumerated type.
 *  @return std::set<std::string>
 */
std::set<std::string>
CEnumeration::values(CVariableDb& db, int typeId)
{
    std::set<std::string> result;
    CSqliteStatement vals(
        db,
        "SELECT value FROM enumerated_values WHERE type_id = ?"
    );
    vals.bind(1, typeId);
    
    while(! (++vals).atEnd()) {
        const char* value = reinterpret_cast<const char*>(vals.getText(0));
        result.insert(std::string(value));
    }
    return result;
}