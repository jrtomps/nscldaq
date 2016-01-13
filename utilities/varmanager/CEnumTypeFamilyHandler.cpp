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
# @file   CEnumTypeFamilyHandler.cpp
# @brief  Impelement the enum type family handler.
# @author <fox@nscl.msu.edu>
*/
#include "CEnumTypeFamilyHandler.h"
#include "CEnumTypeCreator.h"

#include "CSqlite.h"
#include "CSqliteStatement.h"
#include "CEnumeration.h"

/**
 * constructor
 *    Creates the schema for enumerations
 */
CEnumTypeFamilyHandler::CEnumTypeFamilyHandler(CSqlite& db)
{
    // If the enumerated_values table does not exist, create it:

    CSqliteStatement::execute(
        db,
        "CREATE TABLE IF NOT EXISTS enumerated_values (                       \
            id                INTEGER PRIMARY KEY NOT NULL,     \
            type_id           INTEGER NOT NULL,                 \
            value             VARCHAR(256) NOT NULL             \
        )"
    );
    // Create the default bool type:
    
    if(! CEnumeration::typeExists(db, "bool")) {
        std::vector<std::string> values;
        values.push_back("true");
        values.push_back("false");
        CEnumeration::create(db, "bool", values);        
    }

}

/**
 * create
 *    Presented with a typename, determines if that type is an enum
 *    If, so, registers an enum creator for that type with the factory
 *    and returns an instance created by that creator.
 * @param typeName - Type we are attempting to create.
 * @param db       - Database defining types/vars.
 * @param factory  - Factory in which we'll register the creator.
 * @return CDataType* - Pointer to the data type created.
 * @retval 0          - Type is not an enum.
 */
CDataType*
CEnumTypeFamilyHandler::create(const char* typeName, CSqlite& db, CTypeFactory& factory)
{
    // A creator will throw if created on a non enum
    try {
        CEnumTypeCreator* pCreator = new CEnumTypeCreator(db, std::string(typeName));
        factory.addCreator(typeName, pCreator);
        return (*pCreator)();
    } catch (...) {
        return 0;
    }
}