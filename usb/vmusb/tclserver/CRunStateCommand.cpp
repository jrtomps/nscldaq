/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <config.h>
using namespace std;

#include "CRunStateCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <TclServer.h>
#include <CRunState.h>

/*!
   Construct the command. 
   - The base class is created with the command string "Module".
     creation is done with registration.
   - the member data consists of a reference to the list of currently 
     defined modules (this is held by the interpreter thread main object
     as member data so that it will never go out of scope).
     \param interp : CTCLInterpreter&
       The interpreter on which this command will be registered.
     \param modules :  vector<CControlModule*>&
        Reference to the list of modules that have been defined already.
*/
CRunStateCommand::CRunStateCommand(CTCLInterpreter& interp,
			      TclServer&       server) :
  CTCLObjectProcessor(interp, "runstate"),
  m_Server(server)  
{

}
//! Destroy the module.. no op provided only as a chain to the base class destructor.
CRunStateCommand::~CRunStateCommand()
{}


/*!
   The command executor.  The command must have at least 2 object elements:
   - the command name ("Module"),
   - The subcommand .. which must be either "create", "configure" or cget.

   \param interp : CTCLInterpreter& 
       Reference to the interpreter that is running this command.
    \param vector<TCLObject>& objv
       Reference to an array of objectified Tcl_Obj objects that contain the
       command line prarameters.
    \return int
    \retval - TCL_OK  - If eveything worked.
    \retval - TCL_ERROR - on failures.
*/
int
CRunStateCommand::operator()(CTCLInterpreter& interp,
			   vector<CTCLObject>& objv)
{
  // validate the parameter count.

  if (objv.size() != 1) {
    m_Server.setResult("'runstate' called with parameters when there should be none.");
    return TCL_ERROR;
  }

  std::string strState = "unknown";
  CRunState* state = CRunState::getInstance();
  switch (state->getState()) {
    case CRunState::Idle :
      strState = "idle";
      break;
    case CRunState::Active :
      strState = "active";
      break;
    case CRunState::Paused :
      strState = "paused";
      break;
    case CRunState::Starting :
      strState = "starting";
      break;
    case CRunState::Stopping :
      strState = "stopping";
      break;
    default:
      strState = "unknown";
  }
  
  m_Server.setResult(strState);
  
  return TCL_OK; 
  
}
