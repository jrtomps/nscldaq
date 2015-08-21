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
# @file   CTCLServiceApiInstance.cpp
# @brief  Implementation of service api instance commands.
# @author <fox@nscl.msu.edu>
*/
#include "CTCLServiceApiInstance.h"
#include "CServiceApi.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>
#include <stdexcept>


/**
 * constructor
 *   @param interp  - interpreter on which the command is registered.
 *   @param command - Command being registered
 *   @param uri     - URI to the database we are operating on.
 */
CTCLServiceApiInstance::CTCLServiceApiInstance(
    CTCLInterpreter& interp, const char* command, std::string uri
)  :
    CTCLObjectProcessor(interp, command, true),
    m_pApi(0)
{
    m_pApi = new CServiceApi(uri.c_str());        
}

/**
 * destructor:
 */
CTCLServiceApiInstance::~CTCLServiceApiInstance()
{
    delete m_pApi;
}

/**
 * operator()
 *    process the command.
 *
 * @param interp   - interpreter executing the command.
 * @param objv     - The command words.
 * @return int     - TCL_OK or TCL_ERROR.
 */
int
CTCLServiceApiInstance::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        // Require that there at least be a subcommand.
        
        requireAtLeast(objv, 2, "Missing subcommand");
        std::string subcommand = objv[1];
        
        if (subcommand == "exists") {
            interp.setResult(m_pApi->exists() ? "1" : "0");
        } else if (subcommand == "create") {
            m_pApi->create();
        } else if (subcommand == "createprog") {
            createProg(interp, objv);
        } else if (subcommand == "setHost") {
            setHost(interp, objv);
        } else if (subcommand == "setCommand") {
            setProgram(interp, objv);
        } else if (subcommand == "remove") {
            remove(interp, objv);
        } else if (subcommand == "listall") {
            listAll(interp, objv);
        } else if (subcommand == "list") {
            list(interp, objv);
        } else {
            throw std::logic_error("Invalid subcommand");
        }
        
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unexpected exception type caught in service creator");
        return TCL_ERROR;
    }
    return TCL_OK;
}

/*---------------------------------------------------------------------------
 * utility methods
 */

/**
 * createProg
 *    Create a program.
 *
 * @param interp   - interpreter executing the command.
 * @param objv     - command line parameters.
 */
void
CTCLServiceApiInstance::createProg(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(
        objv, 5, "createprog needs name, path, host in addition to subcommand"
    );
    std::string name = objv[2];
    std::string path = objv[3];
    std::string host = objv[4];
    
    m_pApi->create(name.c_str(), path.c_str(), host.c_str());
}
/**
 * setHost
 *    changes the host for a program.
 * @param interp   - interpreter executing the command.
 * @param objv     - command line parameters.
 */
void
CTCLServiceApiInstance::setHost(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(
        objv, 4, "setHost needs name and a new host in addtion to subcommand"
    );
    std::string name = objv[2];
    std::string host = objv[3];
    m_pApi->setHost(name.c_str(), host.c_str());
    
}
/**
 * setProgram
 *    set a new program path.
 * @param interp   - interpreter executing the command.
 * @param objv     - command line parameters.
 */
void
CTCLServiceApiInstance::setProgram(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(
        objv, 4, "setCommand needs name and a new path in addtion to subcommand"
    );
    std::string name = objv[2];
    std::string path = objv[3];
    m_pApi->setCommand(name.c_str(), path.c_str());
}

/**
 * remove
 *    remove a program from the database.
 *  @param interp   - interpreter executing the command.
 *  @param objv      - command line parameters.
 */
void
CTCLServiceApiInstance::remove(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(
        objv, 3, "remove command requires a program in addition to subcommand"
    );
    std::string name = objv[2];
    
    m_pApi->remove(name.c_str());
}
/**
 * listAll
 *    List all programs as a dictable.
 *
 *  @param interp   - interpreter executing the command.
 *  @param objv      - command line parameters.
 */
void
CTCLServiceApiInstance::listAll(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "listall requires no additional parameters");
    
    std::map<std::string, std::pair<std::string, std::string>> listing =
        m_pApi->list();
    
    CTCLObject result;
    result.Bind(interp);
    
    std::map<std::string, std::pair<std::string, std::string>>::iterator p =
        listing.begin();
    while(p != listing.end()) {
        std::string progName = p->first;
        std::string path     = p->second.first;
        std::string host     = p->second.second;
        
        CTCLObject name, info;
        name.Bind(interp);
        info.Bind(interp);
        
        name = progName;
        info = path;
        info += host;
        
        result += progName;
        result += info;
        
        p++;
    }
    
    interp.setResult(result);    
}
/**
 * list
 *   return the path/host pair for a program.
 */
void
CTCLServiceApiInstance::list(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(
        objv, 3, "list requires only a program name in addtion to the subcommand"
    );
    std::string name=objv[2];
    
    std::pair<std::string, std::string> info = m_pApi->list(name.c_str());
    
    CTCLObject result;
    result.Bind(interp);
    result += info.first;
    result += info.second;
    
    interp.setResult(result);
}
