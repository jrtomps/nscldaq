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
# @file  CVarDirTree.cpp
# @brief  Implementation of the directory tree manipulation api.
# @author <fox@nscl.msu.edu>
*/

#include "CVarDirTree.h"
#include "CVariableDb.h"
#include <CSqliteStatement.h>
#include <CSqliteTransaction.h>

#include <sstream>


/**
 * static methods and static functions.
 *
 */

/**
 *  The unbound functions below come from a stack overflow question:
 *    http://stackoverflow.com/questions/236129/split-a-string-in-c
 */

/**
 *  @param s     - input string
 *  @param delim - Split delimeter.
 *  @param elems - References a vector into which the elements are inserted.
 *  @return reference to elems after the insertions.
 */
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


/**
 * @param s - String to split
 * @param delim  - split delimeter.
 * @return vector of strings.
 */
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

/**
 * isRelative
 *   @param path - a directory path.  This may or may not have a variable on the
 *                 back end.  Does't matter.
 *   @return bool - True if the path is relative to a cwd of some sort.
 *                  False if the path is absolute.
 */   
bool
CVarDirTree::isRelative(const char* path)
{
    return (path[0] != m_pathSeparator);    
}

/**
 * parsePath
 *   Parse a path string:
 *   -  Any leading / is removed.
 *   -  Any empty path elements (e.g. /this/is/an/empty/element//back/there)
 *      get removed.
 * @param path - Path to split.
 * @return std::vector<std::string> The path divided.
 */
std::vector<std::string>
CVarDirTree::parsePath(const char* path)
{
    
    std::vector<std::string> result = split(path, '/');
    std::vector<std::string> cleanResult;
    for (int i =0; i < result.size(); i++) {
        if (result[i] != "") {
            cleanResult.push_back(result[i]);
        }
    }
    return cleanResult;
}

/*-----------------------------------------------------------------------------
 * Object level methods.
 */

/**
 * constructor
 *  @param db - the variable database on which we will operate.
 */
CVarDirTree::CVarDirTree(CVariableDb& db) :
    m_db(db)
{
    // Working directory gets set to root...propagate exceptions back out:
    m_wd = getRootInfo();
        
    // And we save the root id from that
    
    m_rootId = m_wd.s_id;
}

/**
 * destructor
 */
CVarDirTree::~CVarDirTree()
{
}

/**
 * mkdir
 *   Create a new directory
 *
 *  @param path   - Path to the directory can be relative or absolute and can contain
 *                  .. elements.
 *  @param createIntermediate - if true (default) any  missing intermediate directories are
 *                              also created.
 */
void
CVarDirTree::mkdir(const char* path, bool createIntermediate)
{
    // TODO: for now handle the absolute case with createIntermediate true:
    
    
    CSqliteStatement makeDir(
        m_db,
        "INSERT INTO directory (name, parent_id) VALUES (:name, :parent_id)"
    );
    CSqliteStatement findSubDir(
        m_db,
        "SELECT id FROM directory WHERE parent_id = :parent AND name = :name"
    );
    
    std::vector<std::string> pathVec = canonicalize(path);
    
    int parent = m_rootId;

    // Traverse the path downward.  once we find a missing path
    // element, create it.
    
    int creates(0);
    CSqliteTransaction t(m_db); // All or nothing with directory path.
    try {     
        for (int i =0; i < pathVec.size(); i++) {
            
            findSubDir.bind(1, parent);
            findSubDir.bind(2, pathVec[i].c_str(), -1, SQLITE_TRANSIENT);
            ++findSubDir;
            if(findSubDir.atEnd()) {
                // Need to create the path element...if createIntermediate
                // is false and we're not at the terminal node of the path,
                // fail:
                
                if (!createIntermediate && (i != (pathVec.size() -1))) {
                    throw CException(
                        "CVarDirTree::mkdir Attemped to create intermediate path \
    elements but that was disabled."
                    );
                }
                
                makeDir.bind(1, pathVec[i].c_str(), -1, SQLITE_TRANSIENT);
                makeDir.bind(2, parent);
                ++makeDir;
                creates++;
                parent = makeDir.lastInsertId();
    
            } else {
                // just set up to traverse the next element.
                
                parent = findSubDir.getInt(0);
            }
            
            // Reset the statements... doesn't hurt to reset both of them.
            
            findSubDir.reset();
            makeDir.reset();
        }
    }
    catch (...) {
        // If there were exceptions rollback the transction
        
        t.rollback();
        throw;
    }
    if (creates == 0) {
        std::string msg = "Attempted to create a duplicate directory: ";
        msg += path;
        throw CException(msg);
    }
}
/**
 * rmdir
 *    Remove a directory.  Note that the directory must be empty to be removed.
 * @param path path relative or absolute to the directory.
 * @throw CVarDirTree::CException if the directory does not exist.
 * @throw CVarDirTree::CException if the directory is not empty.
 */
void
CVarDirTree::rmdir(const char* path)
{
    DirInfo d = find(path);              // Throws if does not exist.
    
    // If d is the root directory we can never allow it to be removed:
    
    if (d.s_id == m_rootId) {
        throw CException("Not allowed to delete the root directory");
    }
    
    if (!empty(d.s_id)) {
        std::string msg(path);
        msg += " cannot be deleted because it is not empty";
        throw CException(msg);
    }
    
    CSqliteStatement del(
        m_db,
        "DELETE FROM directory WHERE id = :id"
    );
    del.bind(1, d.s_id);
    ++del;
}

/**
 * cd
 *    Change the current working directory.
 *
 *  @param path - absolute or relative path to new directory (note .. is ok)
 */
void
CVarDirTree::cd(const char* path)
{
    m_wd = find(path);
    
    
}
/**
 * ls
 *   Return a list of the directories in the current working directory.
 * @return std::vector<DirInfo>
 */
std::vector<CVarDirTree::DirInfo>
CVarDirTree::ls()
{
    std::vector<DirInfo> result;
    
    CSqliteStatement ls(
        m_db,
        "SELECT id, name, parent_id FROM directory \
            WHERE parent_id = :parent \
            ORDER BY name ASC          "
    );
    ls.bind(1, m_wd.s_id);
    
    while(! ((++ls).atEnd())) {
        DirInfo entry;
        entry.s_id  = ls.getInt(0);
        entry.s_name = reinterpret_cast<const char*>(ls.getText(1));
        entry.s_parentId = ls.getInt(2);
        
        result.push_back(entry);
    }
    
    return result;
}
/**
 * wdPath
 *   Return a string that represents the path to the working directory.
 *   e.g. "/this/that";
 *
 *  @return std::string - the path to the wd.
 */
std::string
CVarDirTree::wdPath()
{
    std::string path = m_wd.s_name;                // last path element.
    int parentId     = m_wd.s_parentId;             // we need to go upwards.
    
    CSqliteStatement up(
        m_db,
        "SELECT name, parent_id FROM directory WHERE id = :parent"
    );
    
    while(1) {
      up.bind(1, parentId);
      ++up;
      // If null we just prepend a "/" and we are done.
      
      if (up.columnType(0) == CSqliteStatement::null) {
        path = m_pathSeparator + path;
        break;
      } else {
        std::string name = reinterpret_cast<const char*>(up.getText(0));
        parentId         = up.getInt(1);
        up.reset();
        path = name + m_pathSeparator + path;
      }
    }
    return path;
}
/*---------------------------------------------------------------------------------
 * Private utilities.
 */

/**
 * canonicalize
 *    Take a path that has ..'s in it and might be relative and turn it into
 *    a canonical absolute path.  For example if the current working directory is:
 *    /this/that and the path is ../other the canonicalized path is other.
 *    An exception will be thrown if an attempt is made to .. above the root.
 * @param path - the path
 * @return std::vector<std::string> - Vector of path elements in the canonicalized
 *                                    path.
 */
std::vector<std::string>
CVarDirTree::canonicalize(const char* path)
{
    std::string sPath(path);

    // First construct the vectorized absolute path
    
    // If the path is relative, prepend the cwd:
    
    if (isRelative(path)) {
        std::string cwd = wdPath();
        sPath = cwd + m_pathSeparator + sPath;
    }
    
    std::vector<std::string> pathVec = parsePath(sPath.c_str());
    
    // For each .. remaining in the path, we need to remove it
    // and the prior element of the vector.  If that would make us
    // run off the front of the vector throw and exception.
    
    
    std::vector<std::string>::iterator i = pathVec.begin();
    while(i != pathVec.end()) {
        if (*i == "..") {
            if(i == pathVec.begin()) {
                throw CException(
                    "CVarDirTree::canonicalize -- attempted t go above root directory"
                );
            }
            i--;
            i = pathVec.erase(i);
            i = pathVec.erase(i);
        } else {
            i++;
        }
    }
    return pathVec;                       // Empty pathVec implies /.
}
/**
 * find
 *   Locate a directory and return information about it.
 *   @param path - path (relative or absolute) to it.
 *   @return DirInfo about the directory.
 */
CVarDirTree::DirInfo
CVarDirTree::find(const char* path)
{
    CSqliteStatement findSubDir(
        m_db,
        "SELECT id, parent_id FROM directory WHERE parent_id = :parent AND name = :name"
    );
    
    
    std::vector<std::string> pathVec = canonicalize(path);
    
    int parentId = m_rootId;
    
    
    DirInfo d = getRootInfo();          // Empty path returns /
    
    for (int i = 0; i < pathVec.size(); i++) {
        findSubDir.bind(1, parentId);
        findSubDir.bind(2, pathVec[i].c_str(), -1, SQLITE_TRANSIENT);
        ++findSubDir;
        if (findSubDir.atEnd()) {
            std::string msg = "CVarDirTree::cd - bad path: ";
            msg += path;
            throw CException(msg);
        }
        d.s_name     = pathVec[i];
        parentId     = d.s_id       = findSubDir.getInt(0);
        d.s_parentId = findSubDir.getInt(1);
        
        findSubDir.reset();
    }
    return d;
    
}
/**
 * empty
 *   return true if the directory has nothing in it.
 *   TODO:  Check for variables as well as subdirs.
 *
 *  @param id - id of the directory to check.
 *  @return bool
 */
bool
CVarDirTree::empty(int id)
{
    CSqliteStatement echeck(
        m_db,
        "SELECT COUNT(*) AS c FROM directory WHERE parent_id = :pid"
    );
    echeck.bind(1, id);
    ++echeck;
    return (echeck.getInt(0) == 0);
}

/**
 * getRootInfo
 *   Return directory info for the root dir:
 */
CVarDirTree::DirInfo
CVarDirTree::getRootInfo()
{
    DirInfo result;
    
    CSqliteStatement rootFinder(
        m_db,
        "SELECT * FROM directory WHERE name IS null AND parent_id IS null"
    );
    ++rootFinder;
    if(rootFinder.atEnd()) {
        throw CException("CVarDirTree - can't locate the root directory");
    }
    // Columns are in order:
    //   id(0), name(1), parent_id(2)
    
    result.s_name = "";                        // Name is null.
    result.s_id   = rootFinder.getInt(0);      // 1 is hopefully what we get.
    result.s_parentId = rootFinder.getInt(2);  // 0 is as good as any value.

    return result;
}