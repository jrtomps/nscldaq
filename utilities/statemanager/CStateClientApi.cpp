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
# @file   CStateClientApi.cpp
# @brief  Implementation of the state client api
# @author <fox@nscl.msu.edu>
*/

#include "CStateClientApi.h"
#include <CVarMgrApi.h>
#include <CVarMgrApiFactory.h>
#include <CVarMgrSubscriptions.h>
#include <CMutex.h>
#include <URL.h>

#include <stdlib.h>

static const std::string DefaultSubscriptionService("vardb-changes");

/**
 * constructor
 *
 * @param reqURI      - URI for the request port, must be tcp.
 * @param subURI      - URI for the subscriptions, must be tcp.
 * @param programName - Name for which we are monitoring, must exist.
 * @note Binding to the program is a one-time affair, at constructino time.
 *       no attempt is going to be made to follow changes in 'ReadoutParentDir'
 *       The API, instead does a cd to the correct program directory.
 * @throw CStateClientApi::Exception in the event of errors.
 */
CStateClientApi::CStateClientApi(
    const char* reqURI, const char* subURI, const char* programName
) : m_pApi(0),
    m_pSubscriptions(0),
    m_lastState("unknown"),
    m_standalone(false),
    m_programName(programName),
    m_pMonitor(0)
{
    Enter();                   // The thread may need to use the Api in initialization.
    
    try {
        // Create the API elements:
        
        createApi(reqURI);
        createSubscriptions(subURI);
        
        // Figure out where our program directory is:
        
        m_standalone = (getProgramVar("standalone") == "true" )? true : false;
        
        // Initial known state -> m_lastState.

        

        if (m_standalone) {
            m_lastState = getProgramVar("State");
        } else {
            m_lastState = m_pApi->get("/RunState/State");
        }
        
        // Start the monitor thread:
        // Transfers ownership of m_pSubscriptions to the thread until
        // destruction kills off the thread:
        
        Leave();
        m_pMonitor = new CMonitorThread(programName, this, m_pSubscriptions);
        m_pMonitor->start();
        Enter();
    }
    catch (std::exception & e) {
        freeResources();
        Leave();
        throw CException(e.what());
    }
    catch (...) {
        freeResources();
        Leave();
        throw;
    }
    Leave();
}

/**
 * destructor
 */
CStateClientApi::~CStateClientApi()
{
    // Kill off the monitor thread:
    
    m_pMonitor->scheduleExit();
    m_pMonitor->join();
    delete m_pMonitor;
    m_pMonitor = 0;
    
    //  Deallocate dynamic storage.
    
    
    freeResources();
    
}

/**
 * title
 *   Return the current title string.  This is unconditionally in the
 *   /RunState directory:
 *
 * @return std::string
 */
std::string
CStateClientApi::title()
{
    Enter();
    std::string title = m_pApi->get("/RunState/Title");
    Leave();
    return title;
}
/**
 * runNumber
 *    Return the run number (converted to an integer).
 *    Note that the strong typing of the database library ensures the
 *    value is convertible so we don't check that.
 *
 * @return int
 */
int
CStateClientApi::runNumber()
{
    Enter();
    int runnum = atoi(m_pApi->get("/RunState/RunNumber").c_str());
    Leave();
    return runnum; 
}

/**
 * recording
 *   @return bool - true if the global state Recording flag is "true" false
 *                  if not.
 */
bool
CStateClientApi::recording()
{
    Enter();
    std::string recording = m_pApi->get("/RunState/Recording");
    Leave();
    return (recording == "true") ? true : false;
}

/**
 * inring
 *   @return std::string - Value of the input ring variable (global state).
 */
std::string
CStateClientApi::inring()
{
    
    return getProgramVar("inring");   // getProgramVar diddle mutex.
}

/**
 * outring
 *  @return std::string - value of the output ring
 */
std::string
CStateClientApi::outring()
{
    return getProgramVar("outring");
}

/**
 * isEnabled
 *    @return bool  - true if the program is enabled else false.
 */
bool
CStateClientApi::isEnabled()
{
    return (getProgramVar("enable") == "true") ? true : false;
}


/**
 * waitTransition
 *   Wait for a transition to be posted to the buffer queue
 *
 * @param[out] newState - state after the transition completes.
 * @param timeout - # milliseconds to wait for timeout (-1 means forever).
 *
 * @return bool - false if no transition, true if there was one.
 * @note - It's not safe to enter our Guard until we get the transitions
 *         from the queue because that can deadlock us with the
 *         monitor thread.
 */
bool
CStateClientApi::waitTransition(std::string& newState, int timeout)
{
    
    /*
     * This code is a bit contorted due to the way the threads can interact.
     * here's the idea.  If there are no state changes queued up and ready,
     * we wait.  The wait may timeout but in the time between the timeout and
     * the next getAll, transitions  may be queued so we ignore the timeout
     * and look at whether or not we came away with any transitions after
     * that last getAll.
     *
     * The first getAll is needed because if there is not a transition
     * during the wait, there won't be a signal and if there are transitions
     * queued, we don't want to block at all.
    */
    
    std::list<std::string> transitions = m_StateChanges.getAll();
    if (transitions.empty()) {
        m_StateChanges.wait(timeout);
        transitions = m_StateChanges.getAll();
    }
    
    if (transitions.empty()) {
        newState = m_lastState;
        return false;
    } else {
        while (!transitions.empty()) {
            m_lastState = transitions.front();
            transitions.pop_front();
        }
        newState = m_lastState;
        return true;
    }
    
}
/*-----------------------------------------------------------------------------
 *  Communication methods:
 */

/**
 * postTransition
 *   Called by the monitor thread to post a state transition.
 *   enters the state transition in to the queue.
 *
 *  @param transition - the string for the new state.
 */
void
CStateClientApi::postTransition(std::string transition)
{
    m_StateChanges.queue(transition);
}
/**
 * updateStandalone
 *   Change the state of the standalone flag.. this code assumes that this can
 *   be done atomically with no synchronzation (eg. bool is a simple type)
 *
 *   @param newValue - new value of the standalone flag.
 */
void
CStateClientApi::updateStandalone(bool newValue)
{
    m_standalone = newValue;
}

/*---------------------------------------------------------------------------*/
/* Private utilities.                                                        */


/**
 * createApi
 *   Validates an API URI and creates the URI.
 *   - The URI must be tcp:
 *   - The API  must successfully connect.
 *
 * @param uri - the uri of the connection.
 * @note m_pApi is filled in with the resulting 
 */
void
CStateClientApi::createApi(const char* uri)
{
    URL parsedURI(uri);
    if (parsedURI.getProto() != "tcp") {
        throw CException("The API URI must be a tcp: uri");
    }
    m_pApi = CVarMgrApiFactory::create(uri);
}
/**
 * createSubscriptions
 *    Create the subscription object given the URI.
 *    - URI must be tcp
 *    - URI must refer to an advertised service.
 *    - If the URI does not have a service, the
 *      DefaultSubscriptionService will be used instead.
 *      
 * @param uri
 */
void
CStateClientApi::createSubscriptions(const char* uri)
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
 * freeResources
 *   Free all dynamic resources.
 */
void
CStateClientApi::freeResources()
{
    delete m_pApi;
    delete m_pSubscriptions;
    
    // Multiple calls are ok (e.g. freed and destroyed).
    
    m_pApi  = 0;
    m_pSubscriptions = 0;
}
/**
 * getProgramDirectory
 *    Return our program directory.
 * @return std::string
 */
std::string
CStateClientApi::getProgramDirectory()
{
    Enter();
    std::string parent = m_pApi->get("/RunState/ReadoutParentDir");
    if (parent == "") parent = "/RunState";
    
    parent += "/";
    parent += m_programName;
    Leave();
    return parent;
}
/**
 * getProgramVar
 *    Get a program variable.
 *
 *   @param varname - the variable name
 *   @return std::string - the value of the var.
 */
std::string
CStateClientApi::getProgramVar(const char* varname)
{
    Enter();
    std::string fullVarname = getProgramDirectory();
    fullVarname += "/";
    fullVarname += varname;
    std::string value = m_pApi->get(fullVarname.c_str());
    Leave();
    return  value;
}

/*------------------------------------------------------------------------
 * Implementation of the monitor thread class
 */


/**
 *  Construction just initializes data - we call back into the
 *  api to get the standalone flag.
 *
 * @param name - the program's name.
 * @param api  - Pointer to the object that's spawning us, assumed to be CStateClientApi.
 * @param subs- Subscription object being transferred to our ownership.
 *
 * @note -subscriptions are established when we start running.
 */
CStateClientApi::CMonitorThread::CMonitorThread(
    std::string name, CStateClientApi* api, CVarMgrSubscriptions* subs
) :
    m_name(name), m_pApi(api), m_pSubs(subs), m_exit(false), m_standalone(api->isStandalone())
{}

/**
 * Destruction - most of the work is done when exiting.
 */
CStateClientApi::CMonitorThread::~CMonitorThread()
{}

/**
 * scheduleExit
 *   Set the exit flag so that next time it's checked we exit.
 *
 */
void
CStateClientApi::CMonitorThread::scheduleExit()
{
    m_exit = true;
}

/**
 * init
 *     Thread initialization.  After this method executes the
 *     condition variable is signalled indicating the thread is
 *     off and running.  In order not to miss notifications we need
 *     to get our subscriptions set here.  Therefore the constructor
 *     must release the mutex prior to starting the thread.
 */
void
CStateClientApi::CMonitorThread::init()
{
    std::string programPath = m_pApi->getProgramDirectory();
    subscribe(programPath);
 
}

/**
 * operator()
 *    Entry point of the thread:
 *    - Establish subscriptions:
 *       *  Global state
 *       *  Stand alone State
 *       *  standalone flag.
 *       *  Enter the main loop posting changes until the m_exit is true:
 */
void
CStateClientApi::CMonitorThread::operator()()
{
    std::string programPath = m_pApi->getProgramDirectory();
    
    std::string globalStateVar = "/RunState/State";
    std::string localStateVar  = programPath;
    localStateVar += "/State";
    std::string standaloneVar  = programPath;
    standaloneVar += "/standalone";
    
    
    while (!m_exit) {
        if (m_pSubs->waitmsg(1000)) {
            CVarMgrSubscriptions::Message m = m_pSubs->read();
            
            // We only care about assignments:
            
            if(m.s_operation == "ASSIGN") {
                
                if (m.s_path == standaloneVar) {
                    m_standalone = (m.s_data == "true") ? true : false;
                    m_pApi->updateStandalone(m_standalone);
                } else if (
                    (m_standalone && (m.s_path == localStateVar)) ||
                    ((!m_standalone) && (m.s_path == globalStateVar))
                ) {
                     m_pApi->postTransition(m.s_data);   
                
                }
            }
        }
    }
}
/**
 * subscribe
 *    Set the subscriptions.
 *
 *    @param path - the path to the program's private state directory.
 */
void
CStateClientApi::CMonitorThread::subscribe(std::string path)
{
    // We're using a tight set of subscriptions:
    
    m_pSubs->subscribe("/RunState/State");              // Global state.
    
    std::string localState = path;
    localState += "/State";
    m_pSubs->subscribe(localState.c_str());           // Standalone state.
    
    std::string standalone = path;
    standalone += "/standalone";
    m_pSubs->subscribe(standalone.c_str());          // Standalone/global flag.
}