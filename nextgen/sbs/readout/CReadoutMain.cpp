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

#include <options.h>


#include "CReadoutMain.h"
#include <TCLLiveEventLoop.h>
#include <CAuthorizedTclServer.h>
#include <Exception.h>
#include <CInvalidArgumentException.h>
#include <CPortManager.h>
#include <netdb.h>
#include <stdlib.h>

#include <iostream>

using namespace std;

/*!
  Constructor just initializes member variables:

*/
CReadoutMain::CReadoutMain() :
  m_pTclServer(0)
{
}

/*!
   Destructor kills off dynamic objects which ought to do any
   port cleanup etc.
*/
CReadoutMain::~CReadoutMain()
{
  delete m_pTclServer;
}

///////////////////////////////////////////////////////////////////////////////////////////

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

  try {
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
    startTclServer(string(parsedArgs.port_arg));

    }
    
    // Setup our eventloop.
    
    CTCLLiveEventLoop* pLoop = CTCLLiveEventLoop::getInstance();
    pLoop->start();
  }
  catch (CException& e) {
    cerr << "CReadoutMain::operator() last chance exception handler caught CException object:\n";
    cerr << e.ReasonText() << endl;
  }
  catch (std::string msg) {
    cerr << "CReadoutMain::operator() last chance exception handler caught a string exception\n";
    cerr << msg << endl;
  }
  catch (const char* msg) {
    cerr << "CReadoutMain::operator() last chance exception handler caught const char* exception\n";
    cerr << msg << endl;

  }  
  catch (...) {
    cerr << "CReadoutMain::operator() last chance exception handler caught  an unknown exception type\n";
  }
  exit(EXIT_FAILURE);

}
  
///////////////////////////////////////////////////////////////////////////////////////////


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

//////////////////////////////////////////////////////////////////////////////////////////////

/*!
   Starts the Tcl server component. 
   \param port - Port on which the program will listen for connections:
   - If port is an integer this is the literal port.
   - If port is the string "managed" the port manager is used to allocate a port
     the application name will then be "Readout"
   - If the port manager is any other string, we attempt to locate it in the system's
     service database (getservbyname()), and use the resulting port if there is one.

   \throw CInvalidArgumentException if the port is not valid.

*/
void
CReadoutMain::startTclServer(string port)
{
  // Is port a number

  char* endPtr;
  unsigned long portNumber = strtoul(port.c_str(), &endPtr, 0);
  int  thePort;
  if (*endPtr == '\0') {
    // Numerical port.

    thePort = portNumber;
  }
  else if (port == string("managed")) {
    CPortManager* pManager = new CPortManager(); // Ensures we hold the connection to the end.
    thePort = pManager->allocatePort(string("Readout"));
  }
  else {
    struct servent* pEntry = getservbyname(port.c_str(), "tcp");
    if(pEntry) {
      thePort = ntohs(pEntry->s_port);
    }
    else {
      throw CInvalidArgumentException(port, "Must be a numeric port 'managed' or a known service name",
				      "Starting tcp/tcl server component");
    }
  }
  m_pTclServer = new CAuthorizedTclServer(getInterpreter(), thePort);
}
