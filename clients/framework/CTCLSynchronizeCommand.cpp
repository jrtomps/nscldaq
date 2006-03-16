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

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";/*!
   \file  CTCLSynchronizeCommand.cpp 

           Implements a Tcl command extension
           sync {script}
           
           The script parameter is simply evaluated.
           Since this class is derived from CDAQTCLProcessor, however
           the script is executed syncrhonized to the application's global mutex.
*/

////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "CTCLSynchronizeCommand.h"    				
#include <TCLInterpreter.h>
#include <TCLException.h>


#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

/*!

Executes the script passed as argv[1]  synchronized
to the appliication's global mutex.  Note that the Lack of an Argv[1] is
not an error, no action will be performed, but the mutex will have been
locked and unlocked.

\param rInterp is the interpreter on which the command is to execute.
\param rResult is the result string to be filled in by the script.
\param nArguments is the number of command line arguments.
\param pArguments is a char** command parameter list.

 */
int
CTCLSynchronizeCommand::operator()(CTCLInterpreter& rInterp, CTCLResult &rResult, int nArguments, char* pArguments[])  
{ 
  if(nArguments < 2) {
    return TCL_OK;
  }
  if(nArguments > 2) {
    rResult  = "Usage:\n";
    rResult += "   sync [script]\n";
    rResult += "      script is executed synchronized to the application.\n";
    return TCL_ERROR;
  }
  // From here on, execution is in the hands of the script:

  char* pScript = pArguments[1];
  try {
    string result = rInterp.Eval(pScript);
    rResult = result;
    return TCL_OK;
  }
  catch (CTCLException& rExcept) {
    rResult = rExcept.ReasonText();
    return TCL_ERROR;
  }
  return TCL_ERROR;
}
