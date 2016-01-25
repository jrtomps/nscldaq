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
# @file   CServiceApi.cpp
# @brief  Implement the service API.
# @author <fox@nscl.msu.edu>
*/


#include "CServiceApi.h"
#include <CVarMgrApi.h>
#include <CVarMgrApiFactory.h>

#include <sstream>


// The service data top level directory:

const char* CServiceApi::m_ServiceDir("/Services");

/**
 * constructor
 *
 * @param const char* reqUri - the request URI.
 */
CServiceApi::CServiceApi(const char* reqUri) :
    m_pApi(0)
{
    m_pApi = CVarMgrApiFactory::create(reqUri);        
}

/**
 * destructor
 */
CServiceApi::~CServiceApi()
{
    delete m_pApi;
}

/**
 * exists
 *
 * @return bool
 * @retval true - if the service directory exists.
 * @retval valse - if not.
 *
 * @note this assumes the directroy is in /. 
 */
bool
CServiceApi::exists()
{
    std::vector<std::string> dirs = m_pApi->ls();
    for (int i =0; i < dirs.size(); i++) {
        if (dirs[i] == std::string(m_ServiceDir).substr(1)) return true;
    }
    return false;
    
}
/**
 * create
 *    Creates the services directory
 *  @throw std::exception - if the directory already exists.
 */
void
CServiceApi::create()
{
    m_pApi->mkdir(m_ServiceDir);
}
/**
 * create
 *   Create a program.
 *
 *  @param name - name of the program, becomes the dirname.
 *  @param command - Command to turn (path).
 *  @param host    - Host in which to run it.
 */
void
CServiceApi::create(const char* name, const char* command, const char* host)
{
    m_pApi->cd(m_ServiceDir);
    m_pApi->mkdir(name);
    
    m_pApi->cd(name);
    m_pApi->declare("path", "string", command);
    m_pApi->declare("host", "string", host);
    m_pApi->declare("editorx", "integer", "0");
    m_pApi->declare("editory", "integer", "0");
    
    m_pApi->cd("/");
}

/**
 * setHost
 *   Sets a new hostname for a service.
 *
 *  @param name - Name of program.
 *  @param host -new host name.
 */
void
CServiceApi::setHost(const char* name, const char* host)
{
    setDir(name);
    
    m_pApi->set("host", host);
    m_pApi->cd("/");
}
/**
 * setCommand
 *    Set a new command for a service.
 *
 *    @param name - name of the program.
 *    @param command - new command.
 */
void
CServiceApi::setCommand(const char* name, const char* command)
{
    setDir(name);
    m_pApi->set("path", command);
    m_pApi->cd("/");
}
/**
 * setEditorPosition
 *    Set a new position for the service when displayed in the experiment
 *    editor.
 *
 *  @param name - name of the service.
 *  @param x    - X coordinate.
 *  @param y    - Y coordinate.
 */
void
CServiceApi::setEditorPosition(const char* name, int x, int y)
{
    setDir(name);
    m_pApi->set("editorx", intToString(x).c_str());
    m_pApi->set("editory", intToString(y).c_str());
    
    m_pApi->cd("/");
}
/**
 * getEditorXPosition
 *    Return the x coordinate of a service's position in the exp. editor.
 *
 *  @param name - name of the service
 *  @return int - x coord.
 */
int
CServiceApi::getEditorXPosition(const char* name)
{
    setDir(name);
    int value = atoi(m_pApi->get("editorx").c_str());
    m_pApi->cd("/");
    return value;
}

/**
 * getEditorYPosition
 *    Return the x coordinate of a service's position in the exp. editor.
 *
 *  @param name - name of the service
 *  @return int - x coord.
 */
int
CServiceApi::getEditorYPosition(const char* name)
{
    setDir(name);
    int value = atoi(m_pApi->get("editory").c_str());
    m_pApi->cd("/");
    return value;
}

/**
 * remove
 *    Deletes a program.
 * @param name - name of the program.
 */
void
CServiceApi::remove(const char* name)
{
    
    
    recursiveDelete(programPath(name).c_str());
    m_pApi->cd("/");
}

/**
 * list
 *    List the defined programs.  The program list is returned as an std::map;
 *    program names are the keys and the values are a pair consisting of
 *    the path and host values (first, second), in that order.
 *
 * @return std::map<std::string, std::pair<std::string, std::string> > - see above.
 */
std::map<std::string, std::pair<std::string, std::string> >
CServiceApi::list()
{
    std::map<std::string, std::pair<std::string, std::string> > result;
    
    m_pApi->cd(m_ServiceDir);
    
    std::vector<std::string> progs = m_pApi->ls();
    for (int i =0; i < progs.size(); i++) {
        std::pair<std::string, std::string> progInfo = list(progs[i].c_str());
        result[progs[i]] = progInfo;
    }
    
    m_pApi->cd("/");
    return result;
}

/**
 * list
 *    List info about a program given its name
 *
 *  @param name - Name of the program.
 *  @return std::pair<std::string, std::string> - path, host pair.
 */
std::pair<std::string, std::string>
CServiceApi::list(const char* name)
{
    std::pair<std::string, std::string> result;
    std::string dirName = programPath(name);
    dirName += "/";
    
    result.first  = m_pApi->get((dirName + "path").c_str());
    result.second = m_pApi->get((dirName + "host").c_str());
    
    return result;
}
/*----------------------------------------------------------------------------
 *  Private utilities:
 */

/**
 * programPath
 *    return the path to a program.
 *  @param name - name of the program.
 *  @return std::string - dif that holds the program. Might not exist.
 */
std::string
CServiceApi::programPath(const char* name)
{
    std::string dir = CServiceApi::m_ServiceDir;
    dir            += "/";
    dir            += name;
    return dir;
}
/**
 * setDir
 *    cd to a program's directory:
 *
 * @param name -program name
 */
void
CServiceApi::setDir(const char* name)
{
    m_pApi->cd(programPath(name).c_str());
}
/**
 * recursiveDelete
 *   Delete everything in a path -- recursing through directories.
 *
 * @param name path to delete.
 */
void
CServiceApi::recursiveDelete(const char* path)
{
    
    m_pApi->cd(path);

    
    // Delete the vars:
    
    std::vector<CVarMgrApi::VarInfo> vars = m_pApi->lsvar();
    for (int i =0; i < vars.size(); i++) {
        m_pApi->rmvar(vars[i].s_name.c_str());
    }
    
    // Delete the stuff in the directories:
    
    std::vector<std::string> subdirs = m_pApi->ls();
    for (int i = 0; i < subdirs.size(); i++) {
        recursiveDelete(subdirs[i].c_str());
    }
    
    
    // Delete the directory itself:
    
    m_pApi->cd("..");
    m_pApi->rmdir(path);
    
}
/**
 * intToString
 *    Return the string representation of an integer.
 *
 * @param v - value to convert.
 * @return std::sting - string rep of v.
 */
std::string
CServiceApi::intToString(int v)
{
    std::ostringstream s;
    s << v;
    
    return s.str();
}