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
 * @file CAllParticipantsCommand.cpp
 * @brief implements the "getAllParticipants" command.
 */
#include "CAllParticipantsCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>
#include "CConnectivity.h"
#include <stdexcept>

/**
 * constructor
 *   Register the command on an interpreter making it available to
 *   scripts running in that interpreter:
 *
 * @param interp - Reference to the interpreter on which the command
 *                 is going to be registered.
 * @param cmd    - Optional command name - defaults to getAllParticipants
 */
CAllParticipantsCommand::CAllParticipantsCommand(CTCLInterpreter& interp, const char* cmd) :
  CTCLObjectProcessor(interp, cmd)
{}

/**
 * destructor
 *   Unregisters the command from the interpreter.
 */
CAllParticipantsCommand::~CAllParticipantsCommand() {}

/**
 * operator()
 *   Executes the command.  
 *   - Figure out the hostname.
 *   - Construct a CConnectity object on that host.
 *   - Ask the CConnectivity object to deliver the hosts.
 *   - Marshall the resulting std::vector<std::string> into
 *     a Tcl list which is returned,
 *
 * @note a try/catch block is used to centralize error handling.
 *
 * @param interp - Reference to the interpreter that is executing
 *                 the command.
 * @param objv   - The words of the command.
 *
 * @return int   - TCL_OK - everything worked. TCL_ERROR command failed.
 */
int
CAllParticipantsCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
  bindAll(interp, objv);
  try {
    std::string host("localhost");
    requireAtMost(objv, 2, "Only a host parameter is allowed");
    if (objv.size() == 2) {
      host = std::string(objv[1]);
    }

    CConnectivity  c(host.c_str());
    std::vector<std::string> all = c.getAllParticipants();

    CTCLObject result; 
    result.Bind(interp);

    for (int i = 0; i < all.size(); i++) {
      result += all[i];
    }

    interp.setResult(result);
  }
  catch (CException& e) {
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
    interp.setResult("Unanticipated C++ exception type caught");
    return TCL_ERROR;
  }

  return TCL_OK;
}
