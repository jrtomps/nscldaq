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
# @file   CVarDirTree.h
# @brief  Define the class that manipulates directory trees.
# @author <fox@nscl.msu.edu>
*/
#ifndef __CVARDIRTREE_H
#define __CVARDIRTREE_H



#include <utility>
#include <string>
#include <vector>
#include <stdexcept>

class CVariableDb;


/**
 * @class CVariableDb
 *    This class contains:
 *    - Static methods that know how to break up a path into its components.
 *    - Code to create new directories.
 *    - Code to destroy existing directories
 *    - Code to get information about the subdirectories that exist underneath a directory path.
 *
 *  Directory paths work the same as for linux/unix
 *  -  The path separator is /
 *  -  The special string .. means up one level of the directory tree.
 *  -  Paths that don't begin in / are relative to the cwd.
 *  -  Paths that do begin in / are root directory relative.
 */
class CVarDirTree
{
    // Exported data types:
    
public:
    typedef struct _DirInfo {
        std::string s_name;
        int         s_id;
        int         s_parentId;
    } DirInfo, *pDirInfo;
 
    class CException : public std::runtime_error
    {
    public:
        CException(std::string what) noexcept :
            runtime_error(what) {}
        CException(const char* what) noexcept :
            runtime_error(what) {}
    };
    // class level data:
public:    
    static const char m_pathSeparator = '/';
    
    // object level data:
private:
    CVariableDb&   m_db;
    DirInfo        m_wd;
    int            m_rootId;
    
    
    
    //  static public methods:

public:
    static std::vector<std::string> parsePath(const char* path);
    static bool                     isRelative(const char* path);
    
    // Canonicals:
    
public:
    CVarDirTree(CVariableDb& db);
    virtual ~CVarDirTree();
    
    // Getters:
    
    DirInfo getwd()     const {return m_wd;} 
    int     getRootid() const {return m_rootId; }
    
    // operations:
    
    void mkdir(const char* path, bool createIntermediate = true);
    void rmdir(const char* path);
    void cd(const char* path);
    
    // queries
    
    std::vector<DirInfo>     ls();
    std::string              wdPath();
    
private:
    std::vector<std::string> canonicalize(const char* path);
    DirInfo                 find(const char* path);
    bool                    empty(int id);
    DirInfo                 getRootInfo();
    
    
    
};
#endif