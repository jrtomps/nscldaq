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
    std::string directory = getProgramParentDir();
    directory            += "/";
    directory            += name;
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
    std::string directory = getProgramParentDir();
    directory            += "/";
    directory            += name;
    
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
    std::string directory = getProgramParentDir();
    directory            += "/";
    directory            += name;
    
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
    CVarMgrApi* pApi = m_pMonitor->getApi();
    std::string directory = getProgramParentDir();
    directory           += "/";
    directory           += name;
    
    std::string wd = pApi->getwd();
    try {

        pApi->cd(directory.c_str());
        pApi->set("enable", "true");
    }
    catch (...) {
        pApi->cd(wd.c_str());
        throw;
    }
    
    pApi->cd(wd.c_str());
}