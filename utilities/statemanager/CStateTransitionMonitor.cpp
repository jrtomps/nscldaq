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
    std::string programPath = m_programParentPath;
    programPath += "/";
    programPath += name;
    
    std::string wd = m_pRequestApi->getwd();
    m_pRequestApi->cd(programPath.c_str());
    
    bool result  = getBool("standalone");
    
    m_pRequestApi->cd(wd.c_str());
    
    return result;

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
    std::string fullPath = m_programParentPath;
    fullPath += "/";
    fullPath += name;
    
    std::string wd =m_pRequestApi->getwd();
    m_pRequestApi->cd(fullPath.c_str());
    
    bool enabled = getBool("enable");
    bool salone  = getBool("standalone");
    
    m_pRequestApi->cd(wd.c_str());
    return (enabled && (!salone));
}
/**
 * getBool
 *   Return the value of a boolean variable:
 *
 * @param name - Name of the var.
 * @return bool
 */
bool
CStateTransitionMonitor::getBool(std::string name)
{
    return (m_pRequestApi->get(name.c_str()) == "true") ? true : false;
}
