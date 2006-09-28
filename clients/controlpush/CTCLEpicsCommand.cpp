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
#include "CTCLEpicsCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CTCLChannelCommand.h"

#ifndef __HAVE_STD_NAMESPACE
using namespace std;
#endif


/*!
  Construct the command.. nothing much to it.
*/
CTCLEpicsCommand::CTCLEpicsCommand(CTCLInterpreter& interp, string command) :
  CTCLObjectProcessor(interp, command)
{
}

/*!
  Same for destruction.
*/
CTCLEpicsCommand::~CTCLEpicsCommand() {}

/*!
  Even the function call operator just needs to be sure there's a name
  present.
*/
int
CTCLEpicsCommand::operator()(CTCLInterpreter& interp,
			     vector<CTCLObject>& objv)
{
  if (objv.size() != 2) {
    string result = getName();
    result       += " -- incorrect number of parameters\n";
    result       += "Usage:\n";
    result       += "   epicschannel name\n";
    result       += "      name - the name of an epics channel or data record field\n";
    interp.setResult(result);

    return TCL_OK;
  }

  new CTCLChannelCommand(interp, string(objv[1]));

  return TCL_OK;
}
