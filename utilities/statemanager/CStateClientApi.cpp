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
) : m_guard(0),
    m_pApi(0),
    m_pSubscriptions(0),
    m_lastState("unknown"),
    m_standalone(false)
{
    m_guard = new CMutex;
    
    try {
        // Create the API elements:
        
        createApi(reqURI);
        createSubscriptions(subURI);
        
        // Figure out where our program directory is:
        
        std::string parent = m_pApi->get("/RunState/ReadoutParentDir");
        if (parent == "") parent = "/RunState";
        
        std::string mydir = parent + "/";
        mydir += programName;
        
        m_pApi->cd(mydir.c_str());
        
        // Get the standalone state:
        
        std::string standalone = m_pApi->get("standalone");
        m_standalone = standalone == "true" ? true : false;
        
        // Initial known state -> m_lastState.
        
        if (m_standalone) {
            m_lastState = m_pApi->get("State");
        } else {
            m_lastState = m_pApi->get("/RunState/State");
        }
        
    }
    catch (std::exception & e) {
        freeResources();
        throw CException(e.what());
    }
    catch (...) {
        freeResources();
        throw;
    }
}

/**
 * destructor
 */
CStateClientApi::~CStateClientApi()
{
    
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
    return m_pApi->get("/RunState/Title");
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
    return atoi(m_pApi->get("/RunState/RunNumber").c_str());
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
    delete m_guard;
    delete m_pApi;
    delete m_pSubscriptions;
    
    // Multiple calls are ok (e.g. freed and destroyed).
    
    m_guard = 0;
    m_pApi  = 0;
    m_pSubscriptions = 0;
}