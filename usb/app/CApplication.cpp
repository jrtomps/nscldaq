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
#include "CApplication.h"
#include <CConfiguration.h>
#include <TclServer.h>
#include <Globals.h>
#include "usbReadoutOptions.h"

#include <TCLInterpreter.h>
#include <CBeginRun.h>
#include <CEndRun.h>
#include <CPauseRun.h>
#include <CResumeRun.h>

#include <CPortManager.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////////////////
//  Canonicals:

/*!
   Destructor just ensures chaining by existing.
*/
CApplication::~CApplication()
{}

//////////////////////////////////////////////////////////////////////////////////////////
//
// Implemented member functions.

/*!
  Application entry point.  Parse the command lines and start calling the member functions
  to stitch together the application:

  \param argc   - Count of parameters.
  \param argv   - Array of pointers to command parameters.
*/
int
CApplication::main(int argc, char** argv)
{
  gengetopt_args_info    parsed;
  cmdline_parser(argc, argv, &parsed);

  // fetch the pieces out of the parsed structure.

  std::string daqConfigFile;
  std::string ctlConfigFile;
  std::string deviceSpecification ;
  std::string port;
  std::string application;

  if (parsed.daqconfig_given) daqConfigFile = parsed.daqconfig_arg;
  if (parsed.ctlconfig_given) ctlConfigFile = parsed.ctlconfig_arg;
  if (parsed.device_given)    deviceSpecification = parsed.device_arg;
  if (parsed.port_given)      port          = parsed.port_arg;
  if (parsed.application_given) application = parsed.application_arg;
  

  // Got all we need to get the application put together:

  selectInterface(deviceSpecification);
  selectDAQConfigFile(daqConfigFile);
  selectControlConfigFile(ctlConfigFile);
  createConfiguration();
  createTclServer(port, application);
  createMainInterpreter(argc, argv); // MUST BE LAST!!!!!!


  
}


/*!
   Figure out the actual daq configuration file.
   - If the configuration file has not been given, default it to
     $HOME/daqconfig.tcl
   - Ensure the configuration file is readable, if not complain before exiting.
   - Store the name in the Global variables.

   \param filespec  - File name from command line or "" if none provided.
*/
std::string
CApplication::selectDAQConfigFile(std::string filespec)
{
  std::string finalName = configFile(filespec, "daqconfig.tcl");

  Globals::configurationFilename = finalName;
  
  return finalName;
}

/*!
   Select the control configuration file:
   - If the user did not specify one onthe command line, default to
     $HOME/config/ctlconfig.tcl
   - Ensure the configuration file is readable, else complain noisily
   - Store the final file in the global variable.

   \param filespec - USer supplied file specification or "" if not supplied.

*/
std::string
CApplication::selectControlConfigFile(std::string filespec)
{
  std::string finalName          = configFile(filespec, "ctlconfig.tcl");
  Globals::controlConfigFilename = finalName;
  return finalName;
}

/*!
   Create the configuration object.  The configuration object is responsible
   for managing the configuration of the data taking devices.  It does
   this by having a Tcl interpreter that is enlarged with a set of commands
   that allow scripts to create objects that manage specific supported data taking
   devices.

   This function invokes the pure virtual function setupConfiguration, implemented by
   a concrete sub-class to stock the configuration object's script with the
   appropriate set of commands.

 
   A pointer to the configuration object  is placed in the pConfig global pointer.
*/
void
CApplication::createConfiguration()
{
  Globals::pConfig = new CConfiguration();
  
  setupConfiguration(*Globals::pConfig);

}

/*!
  Create the main interpreter and register the various commands in it.
  Note that this function actually calls Tcl_Main and our static member function
  tclAppInit will be where new commands are registered.  This function must be the
  last thing called in e.g. main as it will not return until the program exits.
  This is because the Tcl interpreter main event loop is entered.
*/
void 
CApplication::createMainInterpreter(int argc, char** argv)
{
  Tcl_Main(argc, argv, tclAppInit);
}

/*!
  Create and start the Tcl server application.  The server is augmented 
  by a set of data that control the set of slow control devices that can be
  controlled by the clients.  These are inserted via a call to the pure virtual
  setupTclServer method.

  
  \param port  - Text string that defines the port on which the server will listen.
                 If this is blank or "managed" the port manager will be used to 
		 allocate a suitable free port.
 
  \param application - If the port manager is used, this provides the service
                 name by which other applications can look us up.  This defaults to
		 "SlowControls"

   It is a bad error for the port to not be empty nor "SlowControls" but not be an integer.
*/

void
CApplication::createTclServer(std::string port, std::string application)
{
  int iPort;

  if (application == string("")) application = "SlowControls";

  // figure out the actual port:

  if ((port == string("managed")) || (port == string(""))) {
    iPort = getManagedPort(application);
  }
  else {
    char* pEnd;
    iPort = strtoul(port.c_str(), &pEnd, 0);
    if (*pEnd) {
      std::cerr << "ERROR - Port must be an integer, \"managed\" or empty" << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  TclServer* pServer = new TclServer();
  setupTclServer(*pServer);
  pServer->start(iPort, Globals::controlConfigFilename.c_str());
  
}

/*
  Figures out the name of a configuration file, and its readability.

  Parameters:
     userSupplied  - The user supplied path. This can be empty
                     indicating a default name should be used.
     defaultTail   - If the userSupplied is blank, the file used is
                     $HOME/config/<defaultTail>

  An error message and exit results if the final file is not readable.

*/


std::string
CApplication::configFile(std::string userSupplied,
			 std::string defaultTail)
{
  // Default the name if not provided.

  if (userSupplied == std::string("") ) {
    std::string home = getenv("HOME");
    std::string file = home;
    file            += "/config/";
    file            += defaultTail;
    userSupplied     = file;
  }

  //  Ensure that we can actually read this file

  int status = access(userSupplied.c_str(), R_OK);
  if (status == -1) {
    std::string msg = "Cannot read the configuration script: ";
    msg            += userSupplied;
    perror(msg.c_str());
    exit(EXIT_FAILURE);
  }

  return userSupplied;

}
/*
**   Initialize the main interpreter by adding commands to it.
**   first we will wrap the interpreter in a CTCLInterpreter object
**   so that we can use CTCLObjectprocssor and CTCLProcessor objects
**   to extend the interpreter.
**
**   Parameters:
**       interp - The raw interpreter object pointer.
**   Returns:
**       TCL_OK if all goes well.
*/

int
CApplication::tclAppInit(Tcl_Interp* pInterp)
{
  CTCLInterpreter* pInterpObj = new CTCLInterpreter(pInterp);
  CTCLInterpreter& interp(*pInterpObj);

  CBeginRun* pBegin = new CBeginRun(interp);
  CEndRun*   pEnd   = new CEndRun(interp);
  CPauseRun* pPause = new CPauseRun(interp);
  CResumeRun* pResume = new CResumeRun(interp);

  return TCL_OK;
}

/*
** Interact with the port manager to obtain a listener port.
**
** Parameters:
**   application  - Name of the application we will register .
** Returns:
**   port on which to listen for connections.
*/
int
CApplication::getManagedPort(std::string application)
{
  CPortManager* pManager = new CPortManager();

  return pManager->allocatePort(application);
}
