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
# @file   CVarMgrOpenCommand.cpp
# @brief  Implement the open command.
# @author <fox@nscl.msu.edu>
*/

#include "CVarMgrOpenCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <stdio.h>
#include <stdexcept>
#include "CVarMgrApiFactory.h"
#include "CVarMgrApi.h"

// Static member initialization:

std::map<std::string, CVarMgrApi*> CVarMgrOpenCommand::m_OpenDatabases;
int CVarMgrOpenCommand::m_handleNo(0);

/**
 * construct
 *
 * @param interp - interpreter on which the command is registered.
 * @param command - the command keyword.
 */
CVarMgrOpenCommand::CVarMgrOpenCommand(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command)
{}
/**
 *  destructor
 */
CVarMgrOpenCommand::~CVarMgrOpenCommand()
{}


/**
 * operator()
 *    Executes the command.
 *    - Binds the interpreter to the command words and ensures that there are sufficent
 *      parameters
 *    - Constructs an appropriate API object via the api factory.
 *    - Assign a handle and insert the object in the handle map.
 *
 * @param interp - interpreter that is executing the command.
 * @param objv   - Command words that make up the command.
 * @return int   - TCL_OK if all went well, TCL_ERROR if not.
 */
int
CVarMgrOpenCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    CVarMgrApi* pApi(0);
    char usage[100];
    
    bindAll(interp, objv);
    
    sprintf(usage, "Usage\n   %s database-uri", std::string(objv[1]).c_str());
    
   
    
    try {
        char handle[100];
        requireExactly(objv, 2, usage);
        
        std::string uri = objv[1];
        pApi = CVarMgrApiFactory::create(uri);
        
        
        sprintf(handle, "varmgr_%d", m_handleNo++);
        m_OpenDatabases[handle] = pApi;
        
        interp.setResult(handle);
    }
    catch (std::string msg) {
        
        interp.setResult(msg);
        delete pApi;
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        delete pApi;
        return TCL_ERROR;
    }
    return TCL_OK;
}
/**
 * handleToApi
 *   Convert a handle string to an API pointer
 *
 *   @param handle  - Handle to translate.
 *   @return CVarMgrApi* - Pointer to the API object or null if the handle
 *                         is invalid.
 */
CVarMgrApi*
CVarMgrOpenCommand::handleToApi(const char* handle)
{
    if (m_OpenDatabases.find(handle) == m_OpenDatabases.end()) {
        return 0;
    } else {
        return m_OpenDatabases[handle];
    }
}
/**
 * close
 *    Delete an api associated with a handle and remove it from the map.
 *
 *  @param handle - the handle
 *  @throw std::runtime_error - if this handle is not defined.
 */
void
CVarMgrOpenCommand::close(const char* handle)
{
    std::map<std::string, CVarMgrApi*>::iterator p = m_OpenDatabases.find(handle);
    if (p == m_OpenDatabases.end()) {
        throw std::runtime_error("Invalid handle");
    }
    delete p->second;
    m_OpenDatabases.erase(p);
    
}

