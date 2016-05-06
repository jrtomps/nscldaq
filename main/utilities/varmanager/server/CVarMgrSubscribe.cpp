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
# @file   CVarMgrSubscribe.cpp
# @brief  implement the varmgr::subscribe command
# @author <fox@nscl.msu.edu>
*/

#include "CVarMgrSubscribe.h"
#include "CVarMgrSubCommand.h"
#include "CVarMgrSubscriptions.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <URL.h>
#include <CPortManager.h>

#include <stdexcept>
#include <stdio.h>

static std::string DefaultServiceName = "vardb-changes";

/**
 * constructor
 *  @param[in] interp references the interpreter on the command is executed.
 *  @param[in] command string that invokes the command.
 */
CVarMgrSubscribe::CVarMgrSubscribe(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, true)
{}
/**
 * destructor
 */
CVarMgrSubscribe::~CVarMgrSubscribe() {}


/**
 * operator() (functor).
 *
 *   Executes the command:
 *   - Ensures there are three parameters.
 *   - Processes the URI.
 *   - Get the host/port pair.
 *   - Create the subscription object.
 *   - Create the subscription command (passing the subscription object).
 *
 * @param[in] interp - The interpreter that is executing the command.
 * @param[in] objv   - Vector of objects that make up the command.
 * @return int TCL_OK on succes, TCL_ERROR on failure.
 */
int
CVarMgrSubscribe::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    char usage[100];
    sprintf(usage, "Usage\n   %s uri command-name",
            std::string(objv[0]).c_str());
    bindAll(interp, objv);
    

    
    try {
        requireExactly(objv, 3, usage);
        
         // Figure out and create the subscriptions object.
        
        std::string uri = objv[1];
        std::string command  = objv[2];
        
        std::pair<std::string, int> connectionInfo = decodeUri(uri);
        CVarMgrSubscriptions* pSubscriptions =
            new CVarMgrSubscriptions(
                connectionInfo.first.c_str(), connectionInfo.second
            );
        
            new CVarMgrSubCommand(interp, command.c_str(), *pSubscriptions);
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    return TCL_OK;
}

/**
 * decodeUri
 *    Decode a URI into a hostname and a service port number.
 * @param[in] uri - The string URI of the connection request.
 * @return std::pair<std::string, int> - first is the hostname.
 *                                     - second is the port number.
 * @throw std::runtime_error  - If there is an error (e.g. no such service).
 */
std::pair<std::string, int>
CVarMgrSubscribe::decodeUri(std::string uri)
{
    URL url(uri);
    
    if (url.getProto() != "tcp") {
        throw std::runtime_error("URI must use the tcp protocol");
    }
    // Get the host name - must be present:
    
    std::string host = url.getHostName();
    if (host == "") {
        throw std::runtime_error("URI must include a host name.");
    }
    
    // If there's a port number there cannot be a path...and the
    // port number is all we need.
    
    int port = url.getPort();
    if (port > 0) {
        if (url.getPath() != "/") {
            throw std::runtime_error("Cannot have a Port and a service name in the same uri");
        }
        return std::pair<std::string, int>(host, port);
    }
    
    // Port  is missing, so there must be a 'path' and it must
    // translate to a known port in the host.  Missing path produces a default.
    
    std::string service = url.getPath();
    if (service == "/") {
        service = DefaultServiceName;
    }
    
    port = portFromService(host, service);
    
    return std::pair<std::string, int>(host, port);
}
/**
 * portFromService
 *    Determines the port number of the specified service.
 * @param[in] host    -  host in which the service is advertised.
 * @param[in] service - Service name string.
 * @return int - port number.
 * @throw std::runtime_error - if the service cannot be translated for any reason.
 * @note The user for the port lookup is the current effective
 *       user.
 */
int
CVarMgrSubscribe::portFromService(std::string host, std::string service)
{
    std::string me = CPortManager::GetUsername();
    CPortManager mgr(host);
    
    std::vector<CPortManager::portInfo> ports = mgr.getPortUsage();
    
    for (int i=0; i < ports.size(); i++) {
        if ((service == ports[i].s_Application) && (me == ports[i].s_User)) {
            return ports[i].s_Port;
        }
    }
    throw std::runtime_error("Unable to translate service to port number");
}
