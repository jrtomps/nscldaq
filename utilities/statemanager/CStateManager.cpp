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
# @file   CStateManager.cpp
# @brief  Manage state of programs and global state. (implementation)
# @author <fox@nscl.msu.edu>
*/


#include "CStateManager.h"
#include "CStateTransitionMonitor.h"
#include <CVarMgrApi.h>

/**
 * constructor
 *   @param requestURI  - URI of the REQ port of the server.
 *   @param subscriptionURI - URI of the PUB/SUB port of the server.
 */
CStateManager::CStateManager(const char* requestURI, const char* subscriptionURI) :
    m_pMonitor(0),
    m_reqURI(requestURI),
    m_subURI(subscriptionURI)
{
    m_pMonitor = new CStateTransitionMonitor(m_reqURI.c_str(), m_subURI.c_str());        
}

/**
 * destructor
 */
CStateManager::~CStateManager()
{
    delete m_pMonitor;
}

/**
 * getProgramParentDir
 *    Return the directory that is the parent to program directories:
 * @return std::string
 */
std::string
CStateManager::getProgramParentDir()
{
    return m_pMonitor->programParentDir();
}
/**
 * setProgramParentDir
 *    Sets a new program parent dir.  Note that since the state monitor freezes
 *    the parent dir we need to kill/recreate it.
 *
 * @param path - new parent directory.
 * @throw std::runtime_error derived exception if the path does not exist.
 */
void
CStateManager::setProgramParentDir(const char* path)
{
    // Get the API, ensure the dir exists and set the ReadoutParentPath to it
    // if so.
    CVarMgrApi* pApi = m_pMonitor->getApi();
    std::string wd = pApi->getwd();
    try {
        pApi->cd(path);    
    } catch (...) {
        pApi->cd(wd.c_str());
        throw;
    
    }
    pApi->set("/RunState/ReadoutParentDir", path);
    
    delete m_pMonitor;
    m_pMonitor = new CStateTransitionMonitor(m_reqURI.c_str(), m_subURI.c_str());
}
/**
 * addProgram
 *   Add a new program to the system.
 *
 *  @param name - new program name (must be unique).
 *  @param def  - Pointer to the program's definition.
 */
void
CStateManager::addProgram(const char* name, const pProgramDefinition def)
{
    std::string directory = getProgramDirectoryPath(name);

    CVarMgrApi* pApi = m_pMonitor->getApi();
    std::string wd = pApi->getwd();
    try {
        // Make the program directory and cd to it:
        
        pApi->mkdir(directory.c_str());
        pApi->cd(directory.c_str());
        
        // Stock with the contents according to def:
        
        pApi->declare("State", "RunStateMachine");  // Default -- 0Initial.
        pApi->declare("path", "string", def->s_path.c_str());
        pApi->declare("enable", "boolean", def->s_enabled ? "true" : "false");
        pApi->declare("standalone", "boolean", def->s_standalone ? "true" : "false");
        pApi->declare("host", "string", def->s_host.c_str());
        pApi->declare("outring", "string", def->s_outRing.c_str());
        pApi->declare("inring", "string", def->s_inRing.c_str());
        
    }
    catch(...) {
        pApi->cd(wd.c_str());    // Be sure we are back to normal.
        throw;
    }
    pApi->cd(wd.c_str());
}
/**
 * getProgramDefinition
 *    Return a program definition struct for the specified program.
 *    Note that if the program does not exist, an exception is thrown
 *
 *  @param name - program name.
 *  @return CStateManager::ProgramDefinition
 *  @throw std::runtime_error
 */
CStateManager::ProgramDefinition
CStateManager::getProgramDefinition(const char* name)
{
    std::string directory = getProgramDirectoryPath(name);
    
    CVarMgrApi* pApi = m_pMonitor->getApi();
    std::string wd = pApi->getwd();
    ProgramDefinition result;
    
    try {
        pApi->cd(directory.c_str());             // Also tests for exists.
        result.s_enabled    = pApi->get("enable") == "true" ? true : false;
        result.s_standalone = pApi->get("standalone") == "true" ? true : false;
        result.s_path       = pApi->get("path");
        result.s_host       = pApi->get("host");
        result.s_outRing    = pApi->get("outring");
        result.s_inRing     = pApi->get("inring");
    }
    catch (...) {
        pApi->cd(wd.c_str());
        throw;
    }
    
    pApi->cd(wd.c_str());
    
    return result;
}
/**
 *  modifyProgramDefinition
 *     Modify the definition of a program.
 *
 *  @param name  - Name of the program.
 *  @param def   - New definition.
 *  @throw std::runtime_error - if there are problems.
 */
void
CStateManager::modifyProgram(const char* name, const pProgramDefinition def)
{
    CVarMgrApi* pApi = m_pMonitor->getApi();
    std::string directory = getProgramDirectoryPath(name);
    
    std::string wd = pApi->getwd();
    try {
        pApi->cd(directory.c_str());
        
        pApi->set("path",  def->s_path.c_str());
        pApi->set("enable",  def->s_enabled ? "true" : "false");
        pApi->set("standalone",  def->s_standalone ? "true" : "false");
        pApi->set("host",  def->s_host.c_str());
        pApi->set("outring",  def->s_outRing.c_str());
        pApi->set("inring",  def->s_inRing.c_str());
    }
    catch(...) {
        pApi->cd(wd.c_str());
        throw;
    }
    pApi->cd(wd.c_str());
}
/**
 * enableProgram
 *    Enable a program.  Enabling a program that is already enabled is a no-op.
 *
 * @param name - program name.
 */
void
CStateManager::enableProgram(const char* name)
{
    setProgramVar(name, "enable", "true");
}
/**
 * disableProgram
 *   Disable a program.
 *
 * @param name  - the program name
 */
void
CStateManager::disableProgram(const char* name)
{
    setProgramVar(name, "enable", "false");

}
/**
 * setProgramStandalone
 *    Puts a program into standalone mode.
 *
 * @param name - name of the program.
 */
void
CStateManager::setProgramStandalone(const char* name)
{
    setProgramVar(name, "standalone", "true");
}
/**
 * setProgramNoStandalone
 *    Takes a program out of standalone mode.
 *
 *    @param name - name of the program.
 */
void
CStateManager::setProgramNoStandalone(const char* name)
{
    setProgramVar(name, "standalone", "false");
}
/**
 * isProgramEnabled:
 *     True if the specified program is enabled:
 *
 *  @param name - name of the program.
 *  @return bool
 */
bool
CStateManager::isProgramEnabled(const char* name)
{
    return getProgramBool(name, "enable");
}
/**
 * isProgramStandalone
 *
 *   @param name - program name.
 *   @return bool - true if the named program is set standalone.
 */
bool
CStateManager::isProgramStandalone(const char* name)
{
    return getProgramBool(name, "standalone");
}
/**
 * listPrograms
 *    Returns a list of all programs.
 * @return std::vector<string>
 */
std::vector<std::string>
CStateManager::listPrograms()
{
    std::vector<std::string> result;
    
    // We are just going to return the names of the sub-directories
    // in the program directory.  The claim is that users shouldn not
    // put additional stuff there.  If this claim is false then we'll
    // need to check for the variables that make each subdir a program.
    
    std::string dir = getProgramParentDir();
    CVarMgrApi* pApi = m_pMonitor->getApi();
    result           = pApi->ls(dir.c_str());
    
    return result;
}
/**
 * listEnabledPrograms
 *    List programs for which enabled is set.
 * @return std::vector<std::string> names of enabled programs (possibly empty)
 */
std::vector<std::string>
CStateManager::listEnabledPrograms()
{
    std::vector<std::string> result;
    std::vector<std::string> all  = listPrograms();
    
    // Filter that down to the set that are enabled:
    
    for (int i = 0; i < all.size(); i++) {
        if (getProgramBool(all[i].c_str(), "enable")) {
            result.push_back(all[i]);
        }
    }
    
    return result;
}
/*----------------------------------------------------------------------
 * Private utilities
 */

/**
 * getProgramDirectory
 *
 *  @param name - name of a program.
 *  @return std::string - directory in whih that program lives.
 *  @note - no determination is done to ensure the program/dir exists.
 */
std::string
CStateManager::getProgramDirectoryPath(const char* name)
{
    std::string directory = getProgramParentDir();
    directory           += "/";
    directory           += name;
    
    return directory;
}
/**
 * getVarPath
 *    Returns the path to a variable in a program.
 *
 *  @param program - the program in which to return the path.
 *  @param name    - Name of the value.
 *  @return std::string - full path to the variable.
 */
std::string
CStateManager::getVarpath(const char* program, const char* name)
{
    std::string path = getProgramDirectoryPath(program);
    path += "/";
    path += name;
    
    return path;
}
/**
 * setProgramVar
 *    set a new value for a program variable.
 *
 *  @param program - name of the program.
 *  @param var     - name of the variable.
 *  @param value   - New value for the variable.
 */
void
CStateManager::setProgramVar(
    const char* program, const char* var, const char* value
)
{
    std::string varPath = getVarpath(program, var);
    CVarMgrApi* pApi    = m_pMonitor->getApi();
    
    pApi->set(varPath.c_str(), value);
}

/**
 * getProgramVar
 *    Returns the value of a program variable:
 *
 *  @param program - name of the program.
 *  @param var     - Name of the variable.
 *  @return std::string - value of the variable.
 */
std::string
CStateManager::getProgramVar(const char* program, const char* var)
{
    std::string path = getVarpath(program, var);
    CVarMgrApi* pApi = m_pMonitor->getApi();
    return pApi->get(path.c_str());
}
/**
 * getProgramBool
 *    Returns the value of a program boolean variable:
 *  @param program - name of the program.
 *  @param var     - Name of the variable.
 *  @return bool   - Value.
 */
bool
CStateManager::getProgramBool(const char* program, const char* var)
{
    std::string value = getProgramVar(program, var);
    
    if (value == "true") return true;
    if (value == "false") return false;
    
    // Not a bool value:
    
    
    std::string errorMessage("Variable: ");
    errorMessage += var;
    errorMessage += " for program ";
    errorMessage += program;
    errorMessage += " does not have a boolean value: ";
    errorMessage += value;
    throw std::runtime_error(errorMessage);
}