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

static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";   
//////////////////////////CExitCommand.cpp file////////////////////////////////////
#include <config.h>
#include "CExitCommand.h"                  
#include "CRunState.h"
#include "CReadoutMain.h"
#include <tcl.h>
#include <TCLResult.h>
#include <unistd.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
/*!
   Default constructor.  This is called when declarations of the form e.g.:
   -  CExitCommand  object;
   are performed.
*/
CExitCommand::CExitCommand ()
   : CDAQTCLProcessor("exit", 0),
     m_rRunState(*(CReadoutMain::getInstance()->getRunState()))
{

} 


/*!
    Executes the exit command:
    -  It is an error to attempt to exit when the run is active.
    -  The shutdown C++ actions are performed.
    -  The shutdown script action is performed.
    -  The application exits.

	\param rInterp   - Interpreter under which the command is running.
	\param rResult   - Result string returned to the interpreter.
	\param argc,argv - Parameters in the command (argv[0] is command).

     Syntax:

        exit

       \returns TCL_OK - if everything worked.

       \returns TCL_ERROR - if there was some sort of error.

\note If TCL_ERROR is returned, rResult has a string containing the
      reason for the error.
*/
int 
CExitCommand::operator()(CTCLInterpreter& rInterp, CTCLResult& rResult, 
			 int argc, char** argv)  
{
  argv++; argc--;
  if(argc) {
    rResult += "Usage:\n";
    rResult += "   exit\n";
    return TCL_ERROR;
  }
  // Exit is only allowed when the run is halted:

  if(m_rRunState.getState() != CRunState::Inactive) {
    rResult = "A run is active and must be halted before exiting\n";
    return TCL_ERROR;
  }

  // Now exit:

  Shutdown(rInterp);		// Perform user tailored pre-exit functions.
  CReadoutMain::getInstance()->Exit();  // Request exit.
  return TCL_OK;		// That'll happen in a bit.
}  

/*!
    Performs the finalization user actions:
    - Invokes the C++ shutdown operations.
    - Invokes the Tcl shutdown script.
    

	\param rInterp - Script under which the exit command is running.

*/
int 
CExitCommand::Shutdown(CTCLInterpreter& rInterp)  
{
  // this function provides latent (unimplemented) support.

  return TCL_OK;
}
