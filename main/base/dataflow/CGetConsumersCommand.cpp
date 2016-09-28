/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * @file CGetConsumersCommand.cpp
 * @brief Implement Tcl command to determine which hosts consume our ring buffer data.
 */


#include "CGetConsumersCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CConnectivity.h"
#include <Exception.h>
#include <stdexcept>

/**
 * constructor
 *    Registers a the new command with an interpreter.
 *    @param interp - the interpreter on which the command will be registered.
 *    @param cmd    - The command that will be registered (defaults to "consumers").
 */
CGetConsumersCommand::CGetConsumersCommand(CTCLInterpreter& interp, const char* cmd) :
    CTCLObjectProcessor(interp, cmd, true)
    {}
    
/**
 * destructor
 *   unregisters the command.
 */
CGetConsumersCommand::~CGetConsumersCommand() {}

/**
 * operator()
 *    Executes the command once on an interpreter.
 *    - Determines which  host to interrogate.
 *    - Creates a CConnectivity object and interrogates the consumer list.
 *    - Marshalls the consumer list into a CTCLObject list which is set as the
 *     intperpreter result.
 *     
 *    @param interp   - References the interpreter on which the command will run.
 *    @param objv     - The command words.
 *    @return int     - TCL_OK -on success, TCL_ERROR on failure with the result
 *                      a human readable error message.
 *    @note - exceptions are used to manage error handling in a centralized
 *            way.
 */
int
CGetConsumersCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        std::string host = "localhost";
        requireAtMost(objv, 2, "Only an optional host parameter can be supplied");
        if (objv.size() ==2) {
            host = std::string(objv[1]);
        }
        // Now that we know the host, create get the vector of hosts:
        
        CConnectivity c(host.c_str());
        std::vector<std::string> consumers = c.getConsumers();
        
        // Marshall the consumers vector into a Tcl list -> result:
        
        CTCLObject result;
        result.Bind(interp);
        
        for (size_t i = 0; i < consumers.size(); i++) {
            result += consumers[i];
        }
        interp.setResult(result);
    }
    catch(CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Caught an unexpected C++ exception type");
        return TCL_ERROR;
    }

    return TCL_OK;
}