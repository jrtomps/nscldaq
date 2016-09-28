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
 * @file CGetProducersCommand.cpp
 * @brief Implement TCL Command to get the hosts that are pumping data into proxy rings.
 *
 */

#include "CGetProducersCommand.h"
#include "CConnectivity.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>
#include <stdexcept>


/**
 * constructor
 *    Adds the command to an interpreter.
 *    @param interp - references the interpreter to which the command is added.
 *    @param cmd    - optional command name string (defaults to "producers").
 */
CGetProducersCommand::CGetProducersCommand(CTCLInterpreter& interp, const char* cmd) :
    CTCLObjectProcessor(interp, cmd, true)
{}

/**
 * destructor
 */
CGetProducersCommand::~CGetProducersCommand() {}

/**
 * operator()
 *     - Figure out which host we're asking about.
 *     - Get the vector of hostnames from that host
 *     - Marshall the vector into a Tcl list to return as the result.
 *
 * @param interp - Interpreter running the command.
 * @param objv   - Command words (no more than two).
 * @return int   - TCL_OK - normal completion, TCL_ERROR - failed.
 * @note - exceptions are used to manage the failure paths into a single code path.
 */
int
CGetProducersCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        std::string host = "localhost";                  // Default host.
        requireAtMost(objv, 2, "Command requires at most a host");
        if (objv.size() ==2) {
            host = std::string(objv[1]);
        }
        // Now that we know the host, construct a CConnectivity object and
        // get the producer vector:
        
        CConnectivity c(host.c_str());
        std::vector<std::string> producers = c.getProducers();
        
        // Marshall and set the result:
        
        CTCLObject result;
        result.Bind(interp);
        for (int i =0; i < producers.size(); i++) {
            result += producers[i];
        }
        
        interp.setResult(result);
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::exception & e) {
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
        interp.setResult("Unexpected C++ exception type caught");
        return TCL_ERROR;
    }
    
    return TCL_OK;
}


