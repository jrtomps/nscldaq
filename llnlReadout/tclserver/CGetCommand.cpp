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
#include "CSetCommand.h"

#include "TclServer.h"
#include <TCLObject.h>
#include <TCLInterpreter.h>
#include "CControlModule.h"
#include <CVMUSB.h>
#include <tcl.h>

using namespace std;

/*!
  Construct the command.
*/
CGetCommand::CGetCommand(CTCLInterpreter& interp,
			 TclServer&       server,
			 CVMUSB&          vme) :
  CTCLObjectProcessor(interp, "Get"),
  m_Server(server),
  m_Vme(vme)
{}
CGetCommand::~CGetCommand()
{}

/*
   Execute the get command.  See the class comments for syntax.
*/
int
CGetCommand::operator()(CTCLInterpreter& interp,
			vector<CTCLObject*> objv)
{
  // Need 3 words on the command line:

  if (objv.size() != 3) {
    setResult(interp,
	      "Get: Incorrect number of command parameters : need Get name point");
    return TCL_ERROR;
  }

  // Get the pieces of the command:


  string name  = objv[1];
  string point = objv[2];
  
  // Locate the object:

  CControlModule* pModule = m_Server.findModule(name);
  if (!pModule) {
    string msg("Get: unable to find module ");
    msg += name;
    setResult(interp, msg);
    return TCL_ERROR;
  }

  string result =   pModule->Get(m_Vme, point);
  setResult(interp, result);
  return TCL_OK;

}


/*
  Set the result from a string:
*/
void
CGetCommand::setResult(CTCLInterpreter& interp, string msg)
{
  Tcl_Obj* result = Tcl_NewStringObj(msg.c_str(), -1);
  Tcl_SetObjResult(interp.getInterpreter(), result);
  
  
}
