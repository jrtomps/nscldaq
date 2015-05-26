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
# @file   CStateTransitionMonitor.cpp
# @brief  Monitor state transitions across global and local states.
# @author <fox@nscl.msu.edu>
*/

#include "CStateTransitionMonitor.h"
#include <CVarMgrApiFactory.h>
#include <CVarMgrApi.h>
#include <URL.h>
#include <CVarMgrSubscriptions.h>
#include <stdlib.h>
#include <stdio.h>

static const std::string DefaultSubscriptionService("vardb-changes");

/**
 * constructor
 *   @param reqURI   - URI to be used in constructing the VarMgrApi (server).
 *                     Must be a TCP URI.
 *   @param subURI   - URI (must be TCP) for constructing the subscription API.
 *
 * Once these are constructed:
 *   - The program parent path is determined.
 *   - The monitor thread is constructed and started.
 */
CStateTransitionMonitor::CStateTransitionMonitor(
    const char* reqURI, const char* subURI
) :
    m_pRequestApi(0),
    m_pSubscriptions(0),
    m_pMonitor(0)
{
    Enter();
    try {
        createReqAPI(reqURI);
        createSubAPI(subURI);
        locateParentPath();
        // Leave();
        // startMonitorThread();

    }
    catch(...) {
        releaseResources();
        Leave();
        throw;
    }
}
/**
 * destructor
 */
CStateTransitionMonitor::~CStateTransitionMonitor()
{
#ifdef MONITOR_OPERATIONAL
    if (m_pMonitor) {
        m_pMonitor->scheduleExit();
        m_pMonitor->join();
        delete m_pMonitor();
    }
#endif
    releaseResources();
}

/**
 * allprograms
 *    Enumerate all of the programs.  This is just asking for the directories
 *    that live in the program parent dir.
 * @return std::vector<std::string> vector of programs.
 */
std::vector<std::string>
CStateTransitionMonitor::allPrograms()
{
    
    std::string wd = m_pRequestApi->getwd();
    
    m_pRequestApi->cd(m_programParentPath.c_str());
    std::vector<std::string> result = m_pRequestApi->ls();
    
    m_pRequestApi->cd(wd.c_str());                // Put things back the way they were.
    
    return result;
}
/**
 * activeProgramsd
 *   Filters the results from allPrograms to only those that are active.
 *   active programs are those with either enabled false or standalone true
 *   In either case, the program is not participating in global state tansitions.
 *
 * @return std::vector<std::string> names of active programs.
 */
std::vector<std::string>
CStateTransitionMonitor::activePrograms()
{
    std::vector<std::string> all = allPrograms();
    std::vector<std::string> result;
    for (int i = 0; i < all.size(); i++) {
        if (isActive(all[i])) {
            result.push_back(all[i]);
        }
    }
    return result;
}
/**
 * isStandalone
 *   @param name - name of a program.
 *   @return bool - True if that program's standalone flag is true.
 */
bool
CStateTransitionMonitor::isStandalone(const char* name)
{
    
    bool result  = getBool(name, "standalone");
    
    return result;

}

/**
 * isEnabled
 *   @param name - Name of the program to test.
 *   @return bool - Boolean; true if that program is enabled.
 */
bool
CStateTransitionMonitor::isEnabled(const char* name)
{
    
    bool result  = getBool(name, "enable");
    
    
    return result;
    
}
/**
 * transitionTimeout
 *    Returns the number of seconds in the state transition timeout.
 *    This is in the global RunState directory.
 *
 *  @return int
 */
int
CStateTransitionMonitor::transitionTimeout()
{
    std::string timeoutStr = m_pRequestApi->get("/RunState/Timeout");
    return atoi(timeoutStr.c_str());
}
/**
 * setTransitionTimeout
 *    Set a new value for the state transition timeout.
 *
 *  @param secs - Timeout in seconds.
 */
void
CStateTransitionMonitor::setTransitionTimeout(int secs)
{
    char timeoutStr[100];
    sprintf(timeoutStr, "%d", secs);
    m_pRequestApi->set("/RunState/Timeout", timeoutStr);
    
}
/*-----------------------------------------------------------------------------
 * Private utilities:
 */


/**
 * createReqAPI
 *    Create the requst API.  Other than ensuring this is a TCP uri,
 *    we let the factory do the work for us:
 *
 *  @param uri - the URI that describes the connection.
 */
void CStateTransitionMonitor::createReqAPI(const char* uri)
{
    URL url(uri);
    if (url.getProto() != "tcp") {
        throw CException("The Request URI must be a tcp:// URI");
    }
    
    m_pRequestApi = CVarMgrApiFactory::create(uri);
}
/**
 * createSubAPI
 *    Create the subscription api.
 *
 *   @param uri - URI describing the connection (must be tcp:)
 */
void CStateTransitionMonitor::createSubAPI(const char* uri)
{
    URL parsedURI(uri);
    if (parsedURI.getProto() != "tcp") {
        throw CException("The Susscription URI must be a tcp:: uri");
    }
    std::string host = parsedURI.getHostName();
    int port = parsedURI.getPort();
    
    try {
        if (port != -1) {
            m_pSubscriptions = new CVarMgrSubscriptions(host.c_str(), port);
        } else {
            std::string path = parsedURI.getPath();
            if (path == "/") path = DefaultSubscriptionService;
            m_pSubscriptions = new CVarMgrSubscriptions(host.c_str(), path.c_str());
        }
    }
    catch (std::string msg) {
        throw CException(msg);
    }
    catch (const char* msg) {
        throw CException(msg);
    }
    catch (std::exception& e) {
        throw CException(e.what());
    }
}
/**
 * locateParentPath
 *    Given that the REQ api is build, locate the parent path for the
 *    programs in the state system and put it in m_programParentPath
 *
 */
void CStateTransitionMonitor::locateParentPath()
{
    std::string parent = m_pRequestApi->get("/RunState/ReadoutParentDir");
    if (parent == "") parent = "/RunState";
    
    m_programParentPath = parent;
}

/**
 * releaseResources
 *   delete the objects.  Note that this does not exit/join/delete the
 *   monitor thread.
 */
void
CStateTransitionMonitor::releaseResources()
{
    delete m_pRequestApi;
    delete m_pSubscriptions;
    
    m_pRequestApi = 0;
    m_pSubscriptions = 0;
}

/**
 * isActive
 *   Given a program name return true if the program is both enabled and
 *   not standalone.
 *
 * @param name - Program name.
 * @return bool
 */
bool
CStateTransitionMonitor::isActive(std::string name)
{
    
    bool enabled = getBool(name.c_str(), "enable");
    bool salone  = getBool(name.c_str(), "standalone");
    
    return (enabled && (!salone));
}
/**
 * getBool
 *   Return the value of a boolean variable:
 *
 * @param program - Name of the program.
 * @param name - Name of the var.
 * @return bool
 */
bool
CStateTransitionMonitor::getBool(std::string program, std::string name)
{
    std::string value = getVar(program.c_str(), name.c_str());
    
    return (value == "true") ? true : false;
}

/**
 * varPath
 *   Given a program name and a variable name return the full variable path.
 *
 * @param   program  - Name of the program.
 * @param   name     - Name of the variable.
 * @return std::string - full path to this hypothetical variable (no effort is made
 *                       to ensure the variable exists).
 */
std::string
CStateTransitionMonitor::varPath(const char* program, const char* name)
{
    std::string fullPath = m_programParentPath;
    fullPath += "/";
    fullPath += program;
    fullPath += "/";
    fullPath += name;
    
    return fullPath;
}
/**
 * getVar
 *    Get the string value of a variable given its program name.
 * @param program - Name of the program.
 * @param name    - Name of the variable.
 * @return std::string - variable's value.
 */
std::string
CStateTransitionMonitor::getVar(const char* program, const char* name)
{
    std::string fullPath = varPath(program, name);
    
    return m_pRequestApi->get(fullPath.c_str());
}