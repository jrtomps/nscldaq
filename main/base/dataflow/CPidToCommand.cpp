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
 * @file CPidToCommand.cpp
 * @brief Tcl interface to the Os::getProcessCommand API (implementation)
 */

#include "CPidToCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <unistd.h>

#include <os.h>


/**
* Constructor, just build the command.
*
* @param interp -References the interpreter on which this command will
*                be registered.
* @param cmd    - Command name string.
*/
CPidToCommand::CPidToCommand(CTCLInterpreter& interp, const char* cmd) :
  CTCLObjectProcessor(interp, cmd, true)
{}

/**
 * Destructor.
 */
CPidToCommand::~CPidToCommand() {}

/**
 * operator()
 *   Execute the command;
 *   - Require exactly one parameter.
 *   - Require that it can convert to a pid_t
 *   - Get the command name vector and convert it to a Tcl list.
 *
 * @param interp - interpreter that is runing the command.
 * @param objv   - Vector of Tcl Objects that make up the command.
 * @return int   - TCL_OK for success, TCL_ERROR if errors.
 */
int
CPidToCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  bindAll(interp, objv);
  try {
    requireExactly(objv, 2);
    pid_t pid = objv[1];

    std::vector<std::string> words = Os::getProcessCommand(pid);
    CTCLObject result;
    result.Bind(interp);

    for (int i = 0; i < words.size(); i++) {
      result += words[i];
    }
    interp.setResult(result);
  }
  catch (std::exception& e) {
    interp.setResult(e.what());
    return TCL_ERROR;
  }
  catch (CException& e) {
    interp.setResult(e.ReasonText());
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
    interp.setResult("Unanticipated exception typ e in pidToCommand");
    return TCL_ERROR;
  }

  return TCL_OK;
}
