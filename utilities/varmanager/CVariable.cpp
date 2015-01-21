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
# @file   CVariable.cpp
# @brief  Implementation of the variable class.
# @author <fox@nscl.msu.edu>
*/


#include "CVariable.h"
#include "CVariableDb.h"
#include "CVarDirTree.h"
#include "CTypeFactory.h"
#include "CDataType.h"

#include <CSqliteStatement.h>

/**
 * create
 *    Create a new variable without the concept of a default directory.
 *    This means that all relative paths are relative the root directory.
 *
 *  @param db   - Variable database connection object.
 *  @param path - Path to the variable (the variable name is the terminal element)
 *  @param type - Desired variable type string (e.g. "integer").
 *
 *  @return - Pointer to a newly constructed CVariable that was just added to
 *            the database.
 */
CVariable*
CVariable::create(
    CVariableDb& db, const char* path, const char* type, const char* initial)
{
    // Break up the path into the directory and the variable name.
    // then we can create a CVarDirTree to determine the id of the owning directory:
    
    std::pair<std::string, std::string> p = breakPath(path);
    
    std::string dirname = p.first;
    std::string varname = p.second;

    CVarDirTree dir(db);
    dir.cd(dirname.c_str());
    int dirId = dir.getwd().s_id;     // directory_id field.
    
    // If the variable already exists, throw an exception...can't double create:
    
    if (findId(db, dir, varname.c_str()) > 0) {
        throw CException("CVariable::create - already exists");
    }
    
    
    // Get the type object so that we can know the type id and the
    // default value if needed.  Failure to get the creator means
    // the type is invalid:
    
    CTypeFactory fact(db);
    CDataType*   dtype = fact.create(type);
    if (!dtype) {
        throw CException("CVariable::create attempted on invalid data type");
    }
    int tid = dtype->id();
    
    // Value is from initial or a default value if initial is null.
    
    std::string value;
    if (initial) {
        value = initial;
    } else {
        value = dtype->defaultValue();
    }
    // If the value is not legal throw as well:
    
    bool valueOk = dtype->legal(value.c_str(), -1);
    delete dtype;             // Don't need this object any more.
    
    if (!valueOk) {
        throw CException("CVariable::create - invalid integer value");
    }
    
    // Insert the variable into the table.

    CSqliteStatement insert(
        db,
        "INSERT INTO variables (name, directory_id, type_id, value) \
            VALUES(?, ?, ?, ?)"   
    );
    
    insert.bind(1, varname.c_str(), -1, SQLITE_TRANSIENT);
    insert.bind(2, dirId);
    insert.bind(3, tid);
    insert.bind(4, value.c_str(), -1, SQLITE_TRANSIENT);
    ++insert;
    return new CVariable(db, insert.lastInsertId());
    
}
/**
 * create (relative)
 *    Creates potentially relative to a directory
 *
 *  @param db - Refereces the database in which the variable will be created.
 *  @param dir- Director object relative to which relative paths go.
 *  @param path - Path to variable.
 *  @param type - Type of variable.
 *  @param value - Optional value.
 *
 *  @return CVariable* Pointer to a newly constructed variable on the reated var.
 *  @note we just build an absolute path and call the other constructor.
 */
CVariable*
CVariable::create(
    CVariableDb& db, CVarDirTree& dir, const char* path, const char* type,
    const char* initial)
{
    std::string absPath;
    
    if (CVarDirTree::isRelative(path)) {
       std::string cdpath = dir.wdPath();
       absPath = cdpath + CVarDirTree::m_pathSeparator + path;
    } else {
        absPath = path;
    }
    return create(db, absPath.c_str(), type, initial);
}

/**
 * list
 *    Produce a listing of the variables that are in a directory.
 *
 *   @param db - Pointer to the variable database.
 *   @param dir - Directory relative to which any path is evaluated.
 *   @param path - Optional path relative to the directory (or absolute) to list
 *
 *   @return std::vector<VarInfo> this will be sorted by variable name.
 *   @retval empty means obviously there are no variables in the directory.
 *   @throw CVarDirTree::CException - if the path leads us nowhere.
 */
std::vector<CVariable::VarInfo>
CVariable::list(CVariableDb* db, CVarDirTree& dir, const char* path)
{
    std::vector<VarInfo> result;
    
    // Copy the dir and if necessary cd it to path.
    
    CVarDirTree vardir(*db, dir.getwd().s_id);
    if (path) {
        vardir.cd(path);
    }
    
    // Given the directory, query the db for variables in that dir:
    
    int dirId = vardir.getwd().s_id;
    CSqliteStatement s(
        *db,
        "SELECT v.id AS id, v.name, v.type_id, t.type_name        \
            FROM variables v                                      \
            INNER JOIN variable_types t ON t.id = v.type_id       \
            WHERE v.directory_id = ?                              \
            ORDER BY v.name ASC"
    );
    s.bind(1, dirId);
    
    ++s;
    while (!s.atEnd()) {
        CVariable::VarInfo v;
        v.s_id     = s.getInt(0);
        v.s_name   = reinterpret_cast<const char*>(s.getText(1));
        v.s_type   = reinterpret_cast<const char*>(s.getText(3));
        v.s_typeId = s.getInt(2);
        v.s_dirId  = dirId;
        
        result.push_back(v);
        
        ++s;
    }
    
    
    return result;
}

/**
 * destroy
 *    Destroy an existing variable
 * @param db - Database the variable lives in.
 * @param id - ID (primary key) of the variable in the variables table.
 */
void
CVariable::destroy(CVariableDb& db, int id)
{
    CSqliteStatement exists(
        db,
        "SELECT COUNT(*) FROM variables WHERE id = ?"
    );
    exists.bind(1, id);
    ++exists;
    if (exists.getInt(0) == 0) {
        throw CException("CVariable::destroy(byid) - No such variable");
    }
    CSqliteStatement destroyer(
        db,
        "DELETE FROM variables WHERE id  = ?"
    );
    destroyer.bind(1, id);
    ++destroyer;
}
/**
 * destroy
 *   Destroy a variable given an object pointer:
 *
 *  @param pVar - pointer to an object that wraps the variable we are destroying
 *  @param doDelete - True if we should delete the variable.
 */
void
CVariable::destroy(CVariable* pVar, bool doDelete)
{
    destroy(pVar->m_db, pVar->m_myId);
    
    if (doDelete) delete pVar;
}
/**
 * destroy
 *   Destroy a variable given a directory and relative (or absolute) path.
 *   We'll create a variable object which reduces it to a previously solved
 *   problem
 *
 *  @param db    - Reference to the database that has the variable.
 *  @param dir   - Reference to the directroy object that establishes the
 *                 dir for relative paths,.
 *  @param path  - Path to the variable, either absolute or, if relative, relative
 *                to the wd of dir.
 */
void
CVariable::destroy(CVariableDb& db, CVarDirTree& dir, const char* path)
{
    CVariable theVar(db, dir, path);
    destroy(&theVar, false);
}
/**
 * destroy
 *    Destroy a variable given an absolute path to it.
 *    Create a variable object reducing this to a previously solved problem.
 *
 * @param db    - Reference to database in which the variable lives.
 * @param path  - Absolute path to the variable.
 */
void
CVariable::destroy(CVariableDb& db, const char* path)
{
    CVariable theVar(db, path);
    destroy(&theVar, false);
}
/*-----------------------------------------------------------------------------
 * Canonicals
*/

/**
 * construct with abs path
 *
 * @param db - Reference to the variable data base.
 * @param path - Path to the variable.
 */
CVariable::CVariable(CVariableDb& db, const char* path) :
  m_db(db),
  m_pDataType(0)
{
    CVarDirTree d(db);
    int id = findId(db, d, path);
    if (id < 0) {
        throw CException("CVariable constructor - no such variable");
    } else {
        load(id);
    }
}
/**
 *  Construct with path relative to a directory.
 *
 *  @param db - Database containing the variable.
 *  @param dir - Directory relative to which the path is.
 *  @param path - Path to the variable relatibve to the directory.
 */
CVariable::CVariable(CVariableDb& db, CVarDirTree& dir,  const char* path) :
    m_db(db),
    m_pDataType(0)
{
        int id = findId(db, dir, path);
        if (id < 0) {
            throw CException("CVariable constructor - no such variable");
        } else {
            load (id);
        }
    }

/**
 * construct (by id).
 *
 * @param id - the id of the variable to construct around.
 */
CVariable::CVariable(CVariableDb& db, int id) :
    m_db(db)
{
    load(id);
}

/**
 * destructor
 *   Kill off the data type object.
 */
CVariable::~CVariable()
{
    delete m_pDataType;
}


/*------------------------------------------------------------------------
 * Object operations.
 */

/**
 * set
 *  Modify the value of variable.
 *  @param value - New value.
 *  @throw CException - if the value is not legal for the type.
 */
void
CVariable::set(const char* value)
{
    if(!m_pDataType->legal(value, m_myId)) {
        throw CException("CVariable::set - Illegal value");
    }
    
    CSqliteStatement s(
        m_db,
        "UPDATE variables SET value=? WHERE id=?"
    );
    s.bind(1, value, -1, SQLITE_TRANSIENT);
    s.bind(2, m_myId);
    
    ++s;
}


/**
 * get
 *   @return std::string - current value of the variable.S
 */
std::string
CVariable::get()
{
    
    CSqliteStatement fetch(
        m_db,
        "SELECT value FROM variables WHERE id = ?"
    );
    fetch.bind(1, m_myId);
    ++fetch;
    if (fetch.atEnd()) {
        throw CException("CVariable::get - the variable no longer exists");
    }
    return reinterpret_cast<const char*>(fetch.getText(0));
}


/**
 * getId
 *    @return id - The id of variable (primary key in its table).
 */
int
CVariable::getId() const
{
    return m_myId;
}

/**
 * getName
 *    Return the name of the variable (this is exclusive of the path).
 *    E.g. the variable /this/is/a/variable will return "variable".
 *
 *   @return std::string
 */
std::string
CVariable::getName() const
{
    return m_myName;
}

/**
 * getDirectory
 *    Return the directory part of the variable path  e.g.
 *    /this/is/a/variable will return /this/is/a.
 *
 * @return std::string
 */
std::string
CVariable::getDirectory()
{
    CVarDirTree myDirData(m_db, m_myDir);
    return myDirData.wdPath();
}
/**
 * getTypeId
 *   Return the id of our data type.
 *
 *   @return int
 */
int
CVariable::getTypeId() const
{
    return m_pDataType->id();
}
/*--------------------------------------------------------------------------
 * Private utility methods.
 */


/**
 * load
 *    Load the object member data for the selected variable.
 *  @param id - primary key of the object.
 */
void
CVariable::load(int id)
{
    CSqliteStatement idget(
        m_db,
        "SELECT  name, type_name, directory_id           \
          FROM variables v                                \
          INNER JOIN variable_types t ON t.id = v.type_id \
          WHERE v.id = ?                                  \
          "
    );
    idget.bind(1, id);
    ++idget;
    if (idget.atEnd()) {
        throw CException("CVariable::load  no such variable");
    }
    
    m_myId = id;
    m_myName = reinterpret_cast<const char*>(idget.getText(0));
    m_myDir = idget.getInt(2);
    
    // Use the type name to craete a data type checker.
    
    CTypeFactory f(m_db);
    m_pDataType = f.create(
        reinterpret_cast<const char*>(idget.getText(1))
    );
}
/**
 * findId
 *   Given a path spec, and default directory to establish relative base,
 *   return the id of the specified variable.
 *
 * @param db  - reference to the database in which the variables live.
 * @param dir - Reference to the default directory object.
 * @param path - Path to the variable (relative to the dir).
 * @return int - Id of the variable
 * @retval -1 if no such variable.
 */
int
CVariable::findId(CVariableDb& db, CVarDirTree& dir, const char* path)
{
    // Break up the path into the directory and the variable name.
    // then we can create a CVarDirTree to determine the id of the owning directory:
    
    std::pair<std::string, std::string> p = breakPath(path);
    std::string dirname = p.first;
    std::string varname = p.second;

    
    // CD to the directory.   If that fails, map the error to a CVariable::Exception
    
    CVarDirTree locald(db, dir.getwd().s_id);   // Don't want to change dir.
    try {
        
        locald.cd(dirname.c_str());
    }
    catch (std::runtime_error e) {
        throw CException(e.what());
    }
    int dirId = locald.getwd().s_id;     // directory_id field.
    
    // Get the id of the variable whose directory id matches the one we
    // cd'd to.
    
    CSqliteStatement exists(
        db,
        "SELECT id FROM variables WHERE name = ? AND directory_id = ?"
    );
    exists.bind(2, dirId);
    exists.bind(1, varname.c_str(), -1, SQLITE_TRANSIENT);
    ++exists;
    if (exists.atEnd()) {
        return -1;                                // No such variable.
    } else {
        return exists.getInt(0);                  // Variable id.
    }
}
/**
 * breakPath
 *    Breaks a path into the directory path and variable name
 *  @param path - path to variable.
 *  @return std::pair<std::string, std::string> first string is the dir path,the
 *          second is the variable name.
 */
std::pair<std::string, std::string>
CVariable::breakPath(const char* path)
{
    // Break up the path into the directory and the variable name.
    // then we can create a CVarDirTree to determine the id of the owning directory:
    
    std::string dirname;
    std::string varname;
    std::vector<std::string> pathElements = CVarDirTree::parsePath(path);
    varname = pathElements[pathElements.size()-1];   // name field.
    
    std::string separator;
    if (!CVarDirTree::isRelative(path)) {
        separator = CVarDirTree::m_pathSeparator;    // abs paths lead with /.
    }
    
    for (int i = 0; i < pathElements.size() - 1; i++) {
        dirname += separator;
        dirname += pathElements[i];
        
        // Ensure subsequent path elements are divided by /
        
        separator = CVarDirTree::m_pathSeparator;
    }

    return std::pair<std::string, std::string>(dirname, varname);    
}
