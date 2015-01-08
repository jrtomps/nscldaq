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
# @file   CDataTypeCreatorBase.cpp
# @brief  Implement utilities for all data type creators.
# @author <fox@nscl.msu.edu>
*/
#include "CDataTypeCreatorBase.h"
#include <CSqlite.h>
#include <CSqliteStatement.h>


/**
 * constructor
 *   @param db - Reference to the sqlite data base in which the variables
 *               whose types are being created will live
 */
CDataTypeCreatorBase::CDataTypeCreatorBase(CSqlite& db) :
    m_db(db)
{}

/**
 * lookupId
 *   Return the id associated with a data type name.  If the type is not
 *   yet defined in the database it is registered and the resulting id returned.
 *
 * @param const char* typeName - pointer to the type name string.
 * @return int id              - Id of the type name.
 */
int
CDataTypeCreatorBase::lookupId(const char* typeName)
{
    int id = getTypeId(typeName);
    if (id > 0) {
        return id;
    } else {
        return registerType(typeName);
    }
    
}

/**
 * registerType
 *    Register a new type in the database and returns the id.
 * @param typeName - name of the new type.
 * @return int     - id of the new type.
 *
 * @note It's the caller's responsibility to avoid double registration.
 */
int
CDataTypeCreatorBase::registerType(const char* typeName)
{
    CSqliteStatement s(
        m_db,
        "INSERT INTO variable_types (type_name) VALUES (?)"
    );
    s.bind(1, typeName, -1, SQLITE_TRANSIENT);
    ++s;
    return s.lastInsertId();
}
/**
 * getTypeId
 *   Lookup the id of a type.
 *    @param typeName - name of the type to find
 *    @return int < 0 if the type is not registered, otherwise the actual
 *                type id.
 */
int
CDataTypeCreatorBase::getTypeId(const char* typeName)
{
    CSqliteStatement s(
        m_db,
        "SELECT id FROM variable_types WHERE type_name = ?"
    );
    s.bind(1, typeName, -1, SQLITE_TRANSIENT);
    ++s;
    if (s.atEnd()) {
        return -1;
    } else {
        return s.getInt(0);
    }
}