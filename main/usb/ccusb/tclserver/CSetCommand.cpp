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
#include <CCCUSB.h>
#include <tcl.h>

using namespace std;

/*!
  Construct the command.
*/
CSetCommand::CSetCommand(CTCLInterpreter&   interp,
			 TclServer&         server,
			 CCCUSB&            vme) :
  CTCLObjectProcessor(interp, "Set"),
  m_Server(server),
  m_Vme(vme)
{}
CSetCommand::~CSetCommand()
{}



/*
   Execute the set command.  Set the class comments for syntax.
*/
int
CSetCommand::operator()(CTCLInterpreter& interp,
			vector<CTCLObject>& objv)
{
  // Must be 4 words in the command:

  if (objv.size() != 4) {
    m_Server.setResult("ERROR Set: Incorrect number of command words. Need: Set name point value");
    return TCL_ERROR;
  }
  // Pull out the values.. All values are strings:

  string name = objv[1];
  string point= objv[2];
  string value= objv[3];

  // Need to find the module:

  CControlModule* pModule = m_Server.findModule(name);
  if (!pModule) {
    string msg("ERROR Set: Control module: ");
    msg += name;
    msg += " cannot be found";
    m_Server.setResult( msg);

    return TCL_ERROR;
  }
  // Now try the command returning any string error that is thrown:

  string result = pModule->Set(m_Vme, point.c_str(), value.c_str());
  m_Server.setResult( result);
  return TCL_OK;
  
}
