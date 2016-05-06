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
# @file   CVarMgrApiFactory.cpp
# @brief  Implements the database API factory.
# @author <fox@nscl.msu.edu>
*/

#include "CVarMgrApiFactory.h"
#include <URL.h>
#include "CVarMgrFileApi.h"
#include "CVarMgrServerApi.h"
#include <CPortManager.h>
#include <os.h>

/**
 * create
 *    Us a URI to create an api:
 *    - file: protocols create CVarMgrFileApi objects.
 *    - tcp: protocls product CVarMgrServerApi objects.  See the class
 *      comments for information about how the port/service is derived.
 *
 * @param uri - URI that specifies the API to make and how it's construction
 *             is parameterized.
 */
CVarMgrApi* CVarMgrApiFactory::create(std::string uri)
{
    URL url(uri);
    std::string p = url.getProto();
    if (p == "file") {
        return createFileApi(url.getPath());
    } else if (p == "tcp") {
        
        // Note that empty paths look like '/' not ''
        
        std::string host = url.getHostName();
        int port         = url.getPort();
        std::string path = url.getPath();
        if (port == -1) {
            // Service name or default:
            
            if (path == "/") {
                return createServerApi(host);          // Default service.    
            } else {
                return createServerApi(host, path);    // supplied service.
            }
        } else {
            // Numeric port:
            
            if (path == "/") {               
                return createServerApi(host, port);
            } else {
                throw CVarMgrApi::CException("Cannot specify both port and service name in URI");
            }
        }
    } else {
        throw CVarMgrApi::CException("Invalid protocol on URI, must be 'file' or 'tcp'");
    }
    return 0;
}


/**
 * createFileApi
 *    Create an api object for direct access to a database file:
 * @param[in] path - Path to the database file (must have already been initialized).
 */
CVarMgrApi*
CVarMgrApiFactory::createFileApi(std::string path)
{
    return new CVarMgrFileApi(path.c_str());
}
/**
 * createServerApi
 *    Create a server API given the host and port number.
 * @param[in] host - The host on which the server is runing
 * @param[in] port - REQ/REP port for the server.
 * @return CVarMgrApi*  - Pointer; to the new API object (user must delete it).
 * @note while this will throw a zmq::error_t if the host is bad, if the host exists,
 *       zmq will only notice that there is no server (only form a connection) when the
 *       first message is sent.
 */
CVarMgrApi*
CVarMgrApiFactory::createServerApi(std::string host, int port)
{
    return new CVarMgrServerApi(host.c_str(), port);    
}
/**
 * createServerApi
 *   Create an api object by looking up the service name to get the connection port
 *
 * @param[in] host - host the server is running in.
 * @param[in] service - Service advertised as the server's REQ/REP port.
 * @return CVarMgrApi* - Pointer to the newly created API object.  This must
 *                 be deleted by the caller when no longer needed.
 */
CVarMgrApi*
CVarMgrApiFactory::createServerApi(std::string host, std::string service)
{
    CPortManager portManager(host);
    
    std::string  me = Os::whoami();
    std::vector<CPortManager::portInfo> ports = portManager.getPortUsage();
    
    for (int i = 0; i < ports.size(); i++) {
        if ((ports[i].s_User == me) && (ports[i].s_Application == service)) {
            return createServerApi(host, ports[i].s_Port);
        }
    }
    
    throw CVarMgrApi::CException("No such advertised service");
    
}

