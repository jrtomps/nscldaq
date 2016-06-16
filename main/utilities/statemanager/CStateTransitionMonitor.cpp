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
#include <unistd.h>
#include <iostream>
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
        Leave();
        startMonitorThread();

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
    if (m_pMonitor) {
        m_pMonitor->scheduleExit();
        m_pMonitor->join();
        delete m_pMonitor;
    }
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

/**
 * getNotifications
 *    Return notifications queued or wait if desired, until some come
 *    in.
 *
 *  @param max - Maximum notifications that will be accepted, -1 means no limit,
 *              0 is not legal.
 *  @param timeout - Number of milliseconds to block if there are not any notifications
 *               in the queue. -1 means as long as needed, 0 means poll once.
 *  @return std::vector<Notification> - Vector (possibily empty) of the notifications.
 */
std::vector<CStateTransitionMonitor::Notification>
CStateTransitionMonitor::getNotifications(int max, int timeout)
{
    std::vector<Notification> result;
    
    if (max == 0) {
        throw CException("CStateTransitionMonitor::getNotifications max cannot be 0");
    }
    // If we can get stuff without blocking that's what we'll do..
    
    Notification Not;
    while (m_notifications.getnow(Not) &&
        ((result.size() < max) || (max == -1))
    ) {
        result.push_back(Not);    
    }
    
    // Otherwise block, then get stuff after we awaken:
    
    if (result.size() == 0) {
        m_notifications.wait(timeout);
        while (m_notifications.getnow(Not) &&            // Refactor.
            ((result.size() < max) || (max == -1))
        ) {
            result.push_back(Not);    
        }
    }
    return result;
}
/**
 * updateProgramParentPath
 *    - Set our new cached parent.
 *    - Kill the monitor thread
 *    - Start a new monitor thread with the updated info.
 * @param path - new parent dir path
 */
void
CStateTransitionMonitor::updateProgramParentPath(const char* path)
{
    m_pMonitor->scheduleExit();
    m_pMonitor->join();
    delete m_pMonitor;
    m_programParentPath = path;
    startMonitorThread();
}

/** postNotification
 *   Post a new notification to the queue.
 *
 *   @param n - the element to post.
 */
void
CStateTransitionMonitor::postNotification(CStateTransitionMonitor::Notification n)
{
    m_notifications.queue(n);
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
/**
 * startMonitorThread
 *    Start the monitor thread.
 *
 */
void
CStateTransitionMonitor::startMonitorThread()
{
    m_pMonitor = new MonitorThread(m_pSubscriptions, this);
    m_pMonitor->start();
}

/*------------------------------------------------------------------------------
 * Monitor thread implementation:
 */

/**
 * constructor
 *   @param pApi - Pointer to a subscsription API.
 *   @param parent - The CStateTransitionMonitor* that created us.
 */
CStateTransitionMonitor::MonitorThread::MonitorThread(
    CVarMgrSubscriptions* pApi, CStateTransitionMonitor* parent
) :
    m_parentDir(parent->programParentDir()),
    m_exiting(false),
    m_pApi(pApi),
    m_pParent(parent)
{
}

CStateTransitionMonitor::MonitorThread::~MonitorThread()
{
      m_pApi->unsubscribe("/RunState/State");
      m_pApi->unsubscribe(m_parentDir.c_str());
}
/**
 *  init
 *     Initialize synchronized with the parent.  This is executing the
 *     thread.
 */
void
CStateTransitionMonitor::MonitorThread::init()
{
        // Subscribe to the global state variable.
    
    m_pApi->subscribe("/RunState/State");
    m_pApi->subscribe(m_parentDir.c_str());
    
    // We  are really only interested in the state variables and changes to the
    // program homedir: so hence these filters:
    
    m_pApi->addFilter(CVarMgrSubscriptions::accept, "*/State");    // All state vars.
    m_pApi->addFilter(CVarMgrSubscriptions::accept, m_parentDir.c_str());   // Only stuff in the parent dir.
    

}
/**
 * operator()
 *    The main loop.
 *    Accept and process messages from the subscription. The messages
 *    are cooked into CStateTransitionMonitor::Notification objects which are
 *    posted to the message queue between the threads.
 */
void
CStateTransitionMonitor::MonitorThread::operator()()
{
    std::string notificationType;
    while (!m_exiting) {
        CVarMgrSubscriptions::Message msg;
        if (m_pApi->waitmsg(0)) {
            msg = m_pApi->read();
            bool post = false;
            CStateTransitionMonitor::Notification notmsg; // s_type, s_state, s_program.
            if (m_pApi->checkFilters(msg.s_path.c_str())) {
                
                // Something involving states or programs:
                
                
                
                if (msg.s_operation == "ASSIGN") {
                    // State change:
                    
                    notmsg.s_state = msg.s_data;              // State value.
                    
                    // Global or program?
                    
                    if (msg.s_path == "/RunState/State") {
                        // Global:
                        
                        post = true;                        // Post this transition.
                        notmsg.s_type = CStateTransitionMonitor::GlobalStateChange;
                        notificationType = "Global State Change";
                    } else {
                         post = true;
                         notmsg.s_type    = CStateTransitionMonitor::ProgramStateChange;
                         notmsg.s_program = programFromVarPath(msg.s_path);
                         notificationType = "Program State Change";
                    }
                } else if (msg.s_operation == "MKDIR") {
                    // Be sure the path  is the paernt directory.  The data will be
                    // the program name:
                    
                    if (msg.s_path == m_parentDir) {
                        notmsg.s_type = CStateTransitionMonitor::ProgramJoins;
                        notmsg.s_program = msg.s_data;
                        post = true;
                        notificationType = "Join";
                    }
                } else if (msg.s_operation == "RMDIR") {
                    if (msg.s_path == m_parentDir) {
                        notmsg.s_type = CStateTransitionMonitor::ProgramLeaves;
                        notmsg.s_program = msg.s_data;
                        post = true;
                        notificationType = "Leaves";
                    }
                }   
            } else {
                // Could be a varchanged if here:
                
                if (msg.s_operation  == "ASSIGN" ) {
                     notmsg.s_type    = VarChanged;
                     notmsg.s_state   = msg.s_path;
                     notmsg.s_program = msg.s_data;
                     post = true;
                     notificationType = "Assign";
                }
            }
            // Only post if the operation resulted in  postable op.
                
            if (post) {
                m_pParent->postNotification(notmsg);
            } 
        } else {
	  usleep(100*1000);
	}
    }
}
/**
 * programFromVarPath
 *    Given the path of a variable, returns the name of the program.   The
 *    variable path name will be of the form:
 *   \verbatim
 *      ${m_parentDir}/programName/varname
 *   \endverbatim
 *
 * Where:
 *    *  ${m_parentDir} is the value of our m_parentDir variable and is the
 *    *  directory in which programs live.
 *    *  programName - is the grail that we seek.
 *    *  varname     - is the name of some variable in the program's directory
 *    *                that we care nothing for.
 *
 *  @param varpath - The variable path from which we are going to extract
 *                   the program name (see above).
 *  @return std::string - the programName in the discussion above.
 */
std::string
CStateTransitionMonitor::MonitorThread::programFromVarPath(std::string varpath)
{
    // first remove the leading ${m_parentDir}/ from the string.
    
    size_t newFirst = m_parentDir.size() + 1; // points past the '/'.
    std::string result = varpath.substr(newFirst);
    
    // Now we need to figure out where the '/' is between the program
    // directory name and the variable name and only retain the stuff
    // before it.
    
    size_t slashpos = result.find('/');
    
    // due to zero based indexing the following is correct:
    
    return result.substr(0, slashpos);
    
    
}
