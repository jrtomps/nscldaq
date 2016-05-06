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
# @file   CEnumeration.h
# @brief  Define a class that knows how to manipulate enumerated types.
# @author <fox@nscl.msu.edu>
*/

#ifndef _CENUMERATION_H
#define _CENUMERATION_H

#include <vector>
#include <set>
#include <string>
#include <stdexcept>

class CSqlite;

/**
 * @class CEnumeration
 *     Really just a namespace for a bunch of static methods. The methods
 *     provided support manipulation and querying enumerated type metadata.
 */
class CEnumeration {
    // Static member functions:
    
public:
    static int create(
        CSqlite& db, const char* pTypeName, std::vector<std::string> values
    );
    static int id(CSqlite& db, const char* pEnumName);
    static void addValue(
        CSqlite& db, const char* pTypeName, const char* value
    );
    static std::vector<std::string> listValues(
        CSqlite& db, const char* pTypeName
    );
    static std::vector<std::string> listEnums(CSqlite& db);
    
    static int getValueId(CSqlite& db, int typeId, const char* pValue);

    static bool typeExists(CSqlite& db, const char* pTypeName);
    
    // Utilities

private:
    
    static int  addType(CSqlite& db, const char* pTypeName);
    static void addValue(CSqlite& db, int typeId, const char* value);
    static bool noDups(std::vector<std::string> values);
    static std::set<std::string> values(CSqlite& db, int typeId);
    
    // nested classes:
public:
    class CException : public std::runtime_error {
    public:
        CException(std::string what) noexcept :
            runtime_error(what) {}
        CException(const char* what) noexcept :
            runtime_error(what) {}    
    };
};


#endif