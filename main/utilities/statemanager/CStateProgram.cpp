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
# @file   CStateProgram.cpp
# @brief  Class that manipulates the state program part of the vardb.
# @author <fox@nscl.msu.edu>
*/

#include "CStateProgram.h"
#include <CVarMgrApi.h>
#include <CVarMgrApiFactory.h>
#include <string>
#include <sstream>
#include <memory>
#include <stdlib.h>

/**
 * constructor
 *   @param uri -  Uri used to create an API.
 *
 *   Since the factory is used to create the API we will own it and need to
 *   destroy it destruction time.
 */
CStateProgram::CStateProgram(const char* uri) :
    m_pApi(0), m_ownedApi(true), m_canTransact(false)
{
    m_pApi =  CVarMgrApiFactory::create(std::string(uri));
    try {
        std::unique_ptr<CVarMgrApi::Transaction> t(m_pApi->transaction());
        m_canTransact = true;
    }
    catch (...) {
        m_canTransact = false;
    }
}

/**
 * constructor
 *    @param pApi - An existing api object pointer.
 *
 *   Since we don't know how the api came into being storage management is the
 *   responsibility of the instantiator.  This is provided because we don't
 *   need/want proliferation of API objects, so CStateManager can just hand us
 *   the API its CStateTransitionMonitor held.
 */
CStateProgram::CStateProgram(CVarMgrApi* pApi) :
    m_pApi(pApi), m_ownedApi(false),
    m_canTransact(false)
{
    // see if m_pApi supports transactions:
    
    try {
        std::unique_ptr<CVarMgrApi::Transaction> t(m_pApi->transaction());
        m_canTransact = true;
    }
    catch (...) {
        m_canTransact = false;
    }
}

/**
 * destructor
 *    If we own the api, delete it:
 */
CStateProgram::~CStateProgram()
{
    if (m_ownedApi) {
        delete m_pApi;
    }
    
}
/**
 * getProgramParentDir
 *    Retrieve the directory that holds the state programs.
 *
 * @return std::string - Path to the directory in which state programs are defined.
 */
std::string
CStateProgram::getProgramParentDir()
{
    std::string parent = m_pApi->get("/RunState/ReadoutParentDir");
    if (parent == "") parent = "/RunState";
    
    return parent;
}
/**
 * setProgramParentDir
 *    Set a new program parent dir.  The caller must have ensured this
 *    directory already exists else an error will be thrown.
 *
 * @paran path - Path to new parent directory
 */
void
CStateProgram::setProgramParentDir(const char* path)
{
    CVarMgrApi* pApi = m_pApi;                  // Used to get this from monitor.
    std::string wd = pApi->getwd();
    try {
        pApi->cd(path);    
    } catch (...) {
        pApi->cd(wd.c_str());
        throw;
    
    }
    pApi->set("/RunState/ReadoutParentDir", path);    
}
/**
 * addProgram
 *   Add a new program to the system.
 *
 *  @param name - new program name (must be unique).
 *  @param def  - Pointer to the program's definition.
 */
void
CStateProgram::addProgram(const char* name, const pProgramDefinition def)
{
    std::string directory = getProgramDirectoryPath(name);

    CVarMgrApi* pApi = m_pApi;
    std::string wd = pApi->getwd();
    
    std::unique_ptr<CVarMgrApi::Transaction> t;
    try {
        // Make the program directory and cd to it:
        
        pApi->mkdir(directory.c_str());
        pApi->cd(directory.c_str());
        
        if (m_canTransact) {
            t.reset(m_pApi->transaction());
        }
        
        // Stock with the contents according to def:
        
        pApi->declare("State", "RunStateMachine");  // Default -- 0Initial.
        pApi->declare("path", "string", def->s_path.c_str());
        pApi->declare("enable", "bool", def->s_enabled ? "true" : "false");
        pApi->declare("standalone", "bool", def->s_standalone ? "true" : "false");
        pApi->declare("host", "string", def->s_host.c_str());
        pApi->declare("outring", "string", def->s_outRing.c_str());
        pApi->declare("inring", "string", def->s_inRing.c_str());
        
        // The variables below are used by the experiment editor to allow it to
        // save the object's position on the editor canvas.
        
        pApi->declare("editorx", "integer", 0);
        pApi->declare("editory", "integer", 0);
        
    }
    catch(...) {
        pApi->cd(wd.c_str());    // Be sure we are back to normal.
        if (t.get()) t->rollback();
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
CStateProgram::ProgramDefinition
CStateProgram::getProgramDefinition(const char* name)
{
    std::string directory = getProgramDirectoryPath(name);
    
    CVarMgrApi* pApi = m_pApi;
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
CStateProgram::modifyProgram(const char* name, const pProgramDefinition def)
{
    CVarMgrApi* pApi = m_pApi;
    std::string directory = getProgramDirectoryPath(name);
    
    std::string wd = pApi->getwd();
    std::unique_ptr<CVarMgrApi::Transaction> t;
    try {
        if (m_canTransact) {
            t.reset(m_pApi->transaction());
        }
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
        if (t.get()) t->rollback();
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
CStateProgram::enableProgram(const char* name)
{
    setProgramVar(name, "enable", "true");
}

/**
 * enableProgram
 *    Enable a program.  Enabling a program that is already enabled is a no-op.
 *
 * @param name - program name.
 */
void
CStateProgram::disableProgram(const char* name)
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
CStateProgram::setProgramStandalone(const char* name)
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
CStateProgram::setProgramNoStandalone(const char* name)
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
CStateProgram::isProgramEnabled(const char* name)
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
CStateProgram::isProgramStandalone(const char* name)
{
    return getProgramBool(name, "standalone");
}
/**
 * listPrograms
 *    Returns a list of all programs.
 * @return std::vector<string>
 */
std::vector<std::string>
CStateProgram::listPrograms()
{
    std::vector<std::string> result;
    
    // We are just going to return the names of the sub-directories
    // in the program directory.  The claim is that users shouldn not
    // put additional stuff there.  If this claim is false then we'll
    // need to check for the variables that make each subdir a program.
    
    std::string dir = getProgramParentDir();
    CVarMgrApi* pApi = m_pApi;
    result           = pApi->ls(dir.c_str());
    
    return result;
}
/**
 * listEnabledPrograms
 *    List programs for which enabled is set.
 * @return std::vector<std::string> names of enabled programs (possibly empty)
 */
std::vector<std::string>
CStateProgram::listEnabledPrograms()
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
/**
 * listStandalonePrograms
 *   Return only those programs that have the standalone flag set:
 *
 *  @return std::vector<std::string>
 */
std::vector<std::string>
CStateProgram::listStandalonePrograms()
{
    std::vector<std::string> result;
    std::vector<std::string> all  = listPrograms();
    
    // Filter that down to the set that are enabled:
    
    for (int i = 0; i < all.size(); i++) {
        if (getProgramBool(all[i].c_str(), "standalone")) {
            result.push_back(all[i]);
        }
    }
    
    return result;
}
/**
 * listInactivePrograms
 *   Return only those programs that are inactive.
 *   A program is inactive if either it is disabled or standalone.
 *
 * @return std::vector<std::string>
 */
std::vector<std::string>
CStateProgram::listInactivePrograms()
{
    std::vector<std::string> result;
    std::vector<std::string> all  = listPrograms();
    
    // Filter that down to the set that are enabled:
    
    for (int i = 0; i < all.size(); i++) {
        if (getProgramBool(all[i].c_str(), "standalone") ||
            (!getProgramBool(all[i].c_str(), "enable"))) {
            result.push_back(all[i]);
        }
    }
    
    return result;
}
/**
 * listActivePrograms
 *    Lists programs that are enabled and not standalone:
 *
 * @return std::vector<std::string>
 */
std::vector<std::string>
CStateProgram::listActivePrograms()
{
    std::vector<std::string> result;
    std::vector<std::string> all  = listPrograms();
    
    // Filter that down to the set that are enabled:
    
    for (int i = 0; i < all.size(); i++) {
        if ((!getProgramBool(all[i].c_str(), "standalone")) &&
            (getProgramBool(all[i].c_str(), "enable"))) {
            result.push_back(all[i]);
        }
    }
    
    return result;
}
/**
 * deleteProgram
 *    Delete a specified program.
 * @param name - name of the program to delete.
 */
void
CStateProgram::deleteProgram(const char* name)
{
    std::unique_ptr<CVarMgrApi::Transaction> t;
    if (m_canTransact) {
        t.reset(m_pApi->transaction());
    }
    try {
        // Get the name of the directory to delete:
        
        std::string progDir = getProgramDirectoryPath(name);
        
        // Delete all the variables in that directory:
        
        CVarMgrApi* pApi                      = m_pApi;
        std::vector<CVarMgrApi::VarInfo> vars =
            pApi->lsvar(progDir.c_str());
        
        // Step into that directory and delete them:
        
        std::string wd = pApi->getwd();
        try {
            pApi->cd(progDir.c_str());
            for (int i = 0; i < vars.size(); i++) {
                pApi->rmvar(vars[i].s_name.c_str());
            }
        }
        catch(...) {
            pApi->cd(wd.c_str());
            throw;
        }
        pApi->cd(wd.c_str());
        
        // Delete the directory too:
        
        pApi->rmdir(progDir.c_str());
    }
    catch (...) {
        if(t.get()) t->rollback();
        throw;
    }
}

/**
 * setEditorPostion
 *    Update the position at which this object will appear on the editor's canvas.
 *
 *   @param name - program name.
 *   @param x    - x coordinate.
 *   @param y    - y coordinate.
 */
void
CStateProgram::setEditorPosition(const char* name, int x, int y)
{
    std::unique_ptr<CVarMgrApi::Transaction> t;
    if (m_canTransact) {
        t.reset(m_pApi->transaction());
    }
    try {
        setProgramVar(name, "editorx", intToString(x).c_str());
        setProgramVar(name, "editory", intToString(y).c_str());
    }
    catch (...) {
        if (t.get()) t->rollback();
        throw;
    }
}
/**
 * getEditorXPosition
 *     Determine the x position of the state program object in the editor
 *     canvas the last time it was saved.
 * @param name - name of the object.
 * @return int - X coordinate of the object.
 */
int
CStateProgram::getEditorXPosition(const char* name)
{
    return atoi(getProgramVar(name, "editorx").c_str());
}
/**
 * getEditorYPosition
 *    Determine the y position of the state program object in the editor
 *    the last time it was saved.
 *  @param name - name of the program.
 *  @return int - y coordinate of the position.
 */
int
CStateProgram::getEditorYPosition(const char* name)
{
    return atoi(getProgramVar(name, "editory").c_str());
}
/*---------------------------------------------------------------------------
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
CStateProgram::getProgramDirectoryPath(const char* name)
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
CStateProgram::getVarpath(const char* program, const char* name)
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
CStateProgram::setProgramVar(
    const char* program, const char* var, const char* value
)
{
    std::string varPath = getVarpath(program, var);
    CVarMgrApi* pApi    = m_pApi;
    
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
CStateProgram::getProgramVar(const char* program, const char* var)
{
    std::string path = getVarpath(program, var);
    CVarMgrApi* pApi = m_pApi;
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
CStateProgram::getProgramBool(const char* program, const char* var)
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

/**
 * intToString
 *    Return the string representation of an integer.
 *
 *  @param v - the value to convert.
 *  @return std::string
 */
std::string
CStateProgram::intToString(int v)
{
    std::ostringstream str;
    str << v;
    
    return str.str();
}