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
# @file   CTCLVarDbEventBuilder.cpp
# @brief  Implement the ::nscldaq::evb command ensemble.
# @author <fox@nscl.msu.edu>
*/

#include "CTCLVarDbEventBuilder.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CVardbEventBuilder.h"
#include "CTCLEvbInstance.h"
#include <Exception.h>
#include <stdexcept>

/**
 * constructor
 *    Create our command.
 *
 * @param interp  - pointer to the interpreter we are registering on.
 * @param command - Name of our ensemble command.
 */
CTCLVarDbEventBuilder::CTCLVarDbEventBuilder(
    CTCLInterpreter* interp, const char* command
) : CTCLObjectProcessor(*interp, command, true)
{
    
}

/**
 * destructor
 *   Destroy all of the command objects that have been
 *   created.  The map will take care of destroying itself.
 */
CTCLVarDbEventBuilder::~CTCLVarDbEventBuilder()
{
    std::map<std::string, CTCLObjectProcessor*>::iterator p;
    for (p = m_Connections.begin(); p != m_Connections.end(); p++) {
        delete p->second;
    }
}
/**
 * operator()
 *    Handle commands.
 *
 *  @param interp - references the interpreter running the command.
 *  @param objv   - Command line parameters.
 *
 *  @return int  TCL_OK for success or TCL_ERROR if not.
 */
int
CTCLVarDbEventBuilder::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 2, "Insuffient parameters");
        std::string subcommand = objv[1];
        
        if (subcommand == "create") {
            create(interp, objv);
        } else if (subcommand == "destroy") {
            destroy(interp, objv);
        } else {
            throw std::string("Invalide subcommand");
        }
    }
    catch(std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unexpected exception type");
        return TCL_ERROR;
    }
    return TCL_OK;    
}
/**
 * create
 *    Create a new command that is connected to a database and
 *    whose subcommands perform the operations on the event builder
 *    part of the database.
 *
 *  @param interp - references the interpreter running the command.
 *  @param objv   - Command line parameters.
 */
void CTCLVarDbEventBuilder::create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 4, "Incorrect number of command parameters");
    
    // Pull the new command name and connection identifier out
    // from the command:
    
    std::string newCommand = objv[2];
    std::string dbUri      = objv[3];
    
    // Create the database object
    
    CVardbEventBuilder *evbp = new CVardbEventBuilder(dbUri.c_str());
    
    // Create and register the new command.

    CTCLObjectProcessor* pCommand =
        new CTCLEvbInstance(interp, newCommand.c_str(), evbp);
    
    m_Connections[newCommand] = pCommand;
    
}
void CTCLVarDbEventBuilder::destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    
}