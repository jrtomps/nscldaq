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
# @file   CEnumType.cpp
# @brief  Implement the enumerated type checker
# @author <fox@nscl.msu.edu>
*/

#include "CEnumType.h"
#include "CVariableDb.h"
#include <CSqliteStatement.h>

/**
 *  Constructor
 *     All the work is done by initializers (base class and member)
 *
 *  @param id       - the id (primary key) of the type.
 *  @param typeName - the name of the enumerated type
 *  @param db       - Reference to the database object.
 */
CEnumType::CEnumType(int id, std::string typeName, CVariableDb& db) :
    CDataType(id, typeName),
    m_db(db)
{}

/** Destructor
 */
CEnumType::~CEnumType() {}


/**
 * legal
 *    @param value - Value to test
 *    @return bool - true if the value is a legal string.
 */
bool
CEnumType::legal(const char* value, int varid) const
{
    std::set<std::string> legalValues = getLegalValues();
    return legalValues.find(std::string(value)) != legalValues.end();
}


/**
 * defaultValue
 *    Default value for the type.
 *  @return std::string - the default value.
 */
std::string
CEnumType::defaultValue() const
{
    CSqliteStatement defGetter(
        const_cast<CVariableDb&>(m_db),
        "SELECT value FROM enumerated_values WHERE type_id = ? ORDER by id ASC"
    );
    defGetter.bind(1, id());
    ++defGetter;
    if (defGetter.atEnd()) {
        return "";                  // Empty enum so empty string.
    } else {
        const char* result = reinterpret_cast<const char*>(defGetter.getText(0));
        return result;
    }
}


/*--------------------------------------------------------------------------
 * utilities
 */

/**
 * Return a set of legal values for the enum.
 */
std::set<std::string>
CEnumType::getLegalValues() const
{
    std::set<std::string> result;
    
    // The const cast is needed below because we are const
    // but the constructor for CSqliteStatement takes a non-const reference
    // and therefore would spoil our const-ness.
    
    CSqliteStatement lister(
        const_cast<CVariableDb&>(m_db),
        "SELECT value FROM enumerated_values WHERE type_id = ?"
    );
    lister.bind(1, id());
    
    while (!(++lister).atEnd()) {
        const char* item = reinterpret_cast<const char*>(lister.getText(0));
        result.insert(std::string(item));
    }
    return result;
}