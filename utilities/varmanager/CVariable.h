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
# @file   CVariable.h
# @brief  Defines the type for a variable in the database.
# @author <fox@nscl.msu.edu>
*/
#ifndef __CVARIABLE_H
#define __CVARIABLE_H

#include <stdexcept>
#include <string>
#include <vector>
#include <utility>

class CVariableDb;
class CVarDirTree;
class CDataType;

/**
 *  @class CVariable
 *     Encapsulates a variable and the operations that can be performed on it.
 *     -  Variables are strongly typed, and that affects the values it can take.
 *        In spite of this, variables are stored with the string representation
 *        of their values.  Typing only is a first cut on the legal types
 *        of variable values (see however constraints).
 *     -  Variables live at a terminal node of the directory hierarchy and therefore
 *        they have an associated directory id.
 *     -  Variables can have one or more constraints applied to them which
 *        further affects the set of values it can have.
 *    
 *
 *     Operations that can be performed on a variable:
 *     -   create    - Creates a new variable in the database (static factory method)
 *     -   destroy   - Destroys an existing variable (Warning if this is done there
 *                     cannot be any valid CVariable objects referring to that variable).
 *     -   construct - Creates a proxy for an existing variable in the database.
 *     -   set       - Set a new value for the variable.
 *     -   get       - Return the string representation of the current value of
 *                     the variable.
 *     -   addConstraint - Adds a new constraint on the variable.  Note that
 *                     adding a new constraint will check the variable's current
 *                     value against that constraint and throw an exception
 *                     without adding the constraint if the value does not
 *                     pass.
 *     -   clearConstraints - Removes all constraints on a variable.
 *
 */
class CVariable
{
    // Static methods:
public:    
    static CVariable* create(
        CVariableDb& db, const char* path, const char* type,
        const char* initial = 0
    );                                                             // Fully tested
    static CVariable* create(
        CVariableDb& db, CVarDirTree& dir, const char* path, const char* type,
        const char* initial = 0
    );                                                            // Fully tested
    
    static void destroy(CVariableDb& db, const char* path);
    static void destroy(CVariableDb& db, CVarDirTree& dir, const char* path);
    static void destroy(CVariableDb& db, int id);
    static void destroy(CVariable* pVariable, bool doDelete = true);
    
    static std::vector<std::string> list(
        CVariableDb* db, CVarDirTree& dir, const char* path=0
    );
    
    // object data:
    
private:
    CVariableDb&   m_db;                 // Database connection
    
    int            m_myId;               // primary key of variable.
    std::string    m_myName;             // Name of variable.
    CDataType*     m_pDataType;          // Data type.
    
    // Canonicals
public:
    CVariable(CVariableDb& db, const char* path);                // Fully Tested
    CVariable(CVariableDb& db, CVarDirTree& dir, const char* path); // F Tested
    CVariable(CVariableDb& db, int id);                        // Fully tested
    ~CVariable();
    
    // Operations.
public:    
    void set(const char* newValue);                           // fully tested
    std::string get();                                        // fully tested
    
    // void addConstraint(CConstraint& c);
    // void clearConstraints();
    
    // Getters:
public:
    int getId() const;
    std::string getName() const;
    std::string getDirectory();
    
    // Private utilities
    
private:
    void load(int id);
    static int  findId(CVariableDb& db, CVarDirTree& dir, const char* path);
    static std::pair<std::string, std::string> breakPath(const char* path);
    
    // Nested classes.
public:  
    class CException : public std::runtime_error
    {
    public:
        CException(std::string what) noexcept :
            runtime_error(what) {}
        CException(const char* what) noexcept :
            runtime_error(what) {}
    };
    
};


#endif