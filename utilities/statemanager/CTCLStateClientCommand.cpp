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
# @file   CTCLStateClientCommand.cpp
# @brief  implement stateclient creational command.
# @author <fox@nscl.msu.edu>
*/


#include "CTCLStateClientCommand.h"
#include "TCLInterpreter.h"
#include "TCLObject.h"
#include "CTCLStateClientInstanceCommand.h"
#include <stdexcept>
#include <Exception.h>
#include <stdio.h>

/**
 * constructor
 *    Create the uh..creational command.
 *
 * @param interp  references the interpreter on which commands will be
 *                created.
 * @param command Command that our code is associated with.
 */
CTCLStateClientCommand::CTCLStateClientCommand(
    CTCLInterpreter& interp, const char* command
) :
    CTCLObjectProcessor(interp, command, true)
{
    
}
/**
 * destructor
 *    Delete all of the ensembles that have been created.
 */
CTCLStateClientCommand::~CTCLStateClientCommand()
{
    // Note that deletion invalidates iterators hence the
    // structure of this code:
    
    while(!m_createdCommands.empty()) {
        std::map<std::string, CTCLStateClientInstanceCommand*>::iterator p =
            m_createdCommands.begin();
        deleteEnsemble(p->first);
    }
}

/**
 * operator()
 *    Marshall the parameters for each operation and pass
 *    them on to the appropriate execution function. The
 *    'throw exception on error' pattern is used to regularize
 *    handling.
 *
 *    There are two forms of the command:
 *
 * \verbatim
 *    stateclient newcommand requri suburi programName
 * \verbatim
 *
 * If all works, this create a new command ensemble that represents
 * a program's connection to the state management system.
 *
 * \verbatim
 *    stateclient -delete existingcommand
 * \endverbatim
 *
 *   If this works, an existing connection to a program's state management
 *   system is destroyed along with its command ensemble.
 *
 *  @note it is not legal to form an ensemble named -delete.
 *
 *  @param interp - the interpreter executing the command.
 *  @param objv   - Vector of command line words that represent the command.
 *   
 */
int
CTCLStateClientCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 3, "Insufficient parameter count");
        
        // If it's a -delete marshall that
        
        if (std::string(objv[1]) == "-delete") {
            requireExactly( objv, 3, "Incorrect parameter count");
            std::string name = objv[2];
            deleteEnsemble(name);
        } else {
            // Create or error:
            
            requireExactly(objv, 5, "Incorrect parameter count");
            std::string command = objv[1];
            std::string requri  = objv[2];
            std::string suburi  = objv[3];
            std::string program = objv[4];
            
            createEnsemble(interp, command, requri, suburi, program);
        }
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("stateclient Unanticipated exception type caught");
        return TCL_ERROR;
    }
    
    return TCL_OK;
}
/**
 * deleteEnsemble
 *    Removes a command ensemble:
 *    - Look up the command ensemble in m_createdCommands
 *    - Delete the CTCLStateClientInstanceCommand in that entry.
 *    - Remove the entry from the dict.
 *
 * @note deleting a command object unregisters it from the interpreter.
 *
 * @param name - name of the command ensemble to delete (command name).
 */
void
CTCLStateClientCommand::deleteEnsemble(std::string name)
{
    std::map<std::string, CTCLStateClientInstanceCommand*>::iterator p =
        m_createdCommands.find(name);
    if (p != m_createdCommands.end()) {
        CTCLStateClientInstanceCommand* pCommand = p->second;
        m_createdCommands.erase(p);
        delete pCommand;
    } else {
        char message[1000];
        sprintf(message, "No such state client command: '%s'", name.c_str());
        throw std::invalid_argument(message);
    }
}
/**
 * createEnsemble
 *    Create a new command ensemble and record it in m_createdCommands.
 *
 *  @param interp - interpreter on which the ensemble is registered.
 *  @param name   - new command name.
 *  @param reqUri - Request URI.
 *  @param subUri - Subscription URI.
 *  @param programName - Name of the program we are connected to.
 */
void
CTCLStateClientCommand::createEnsemble(
    CTCLInterpreter& interp, std::string name,
    std::string reqUri, std::string subUri, std::string programName
)
{
    
    // No duplicates allowed:
    
    if (m_createdCommands.find(name) == m_createdCommands.end()) {
        
        CTCLStateClientInstanceCommand* pNewCommand =
            new CTCLStateClientInstanceCommand(
                interp, name, reqUri, subUri, programName
            );
            
        m_createdCommands[name] = pNewCommand;
    } else {
        char message[1000];
        sprintf(
            message, "state connection ommand '%s' was aready created",
            name.c_str()
        );
        throw std::invalid_argument(message);
    }
}