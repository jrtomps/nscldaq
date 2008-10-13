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


#include "CReadoutMain.h"
#include <TCLLiveEventLoop.h>
#include <options.h>
#include <iostream>

using namespace std;

/*!
   Entry point for the readout main object.
   - Process command arguments.
   - Setup the experiment object.
   - Invoke user initialization of the experiment object.
   - Start the TCP/IP server if the --port switch is supplied.
   - Start the tcl interpreter using the event loop.

*/
int
CReadoutMain::operator()()
{
  int argc;
  char** argv;
  struct gengetopt_args_info   parsedArgs;
  getProgramArguments(argc, argv);
  cmdline_parser(argc, argv, &parsedArgs);


  // Initialize the application;
  // this include user initialization.

  SetupRunVariables();
  SetupStateVariables();
  SetupReadout();		// From derived class.
  SetupScalers();	   	// Allowed to be null (the default).


  // If asked to, create the Tcl server component and hook it into the event loop

  // Hook stdin into the event loop so we can run the event loop while processing
  // commands from stdin:

  if (parsedArgs.port_given) {
    cerr << "Would start the tcl server on port " << parsedArgs.port_arg << endl;
  }

  // Setup our eventloop.

  CTCLLiveEventLoop* pLoop = CTCLLiveEventLoop::getInstance();
  pLoop->start();
  return TCL_ERROR;
}


/*!
   Setup the built-in run variables.
*/
void
CReadoutMain::SetupRunVariables()
{
}

/*!
   Setup the built-in state variables.
*/
void
CReadoutMain::SetupStateVariables()
{
}
/*!
  Setup the scaler default trigger.
*/
void
CReadoutMain::SetupScalers()
{
}
