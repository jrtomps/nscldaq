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
//////////////////////////CEndCommand.cpp file////////////////////////////////////
#include <config.h>
#include <TCLResult.h>
#include "CEndCommand.h"                  
#include "CReadoutMain.h"
#include "CRunState.h"
#include <Exception.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
/*!
  Create the command ("end"):
*/
CEndCommand::CEndCommand () :
  CStateTransitionCommand("end")
{

} 

/*!
    Executes the end command.  At this time, since the
    global mutex is held, the trigger response loop is frozen.
    Therefore in this context we ask the experiment to end the run which:
    - Calls ExecutePreFunction in our thread context.
    - Does the C++ shutdown on the electronics in our thread context
    - Closes off any partial event buffer,
    - Emits a scaler buffer
    - Emits the end run buffer in our thread context,
    - Calls the ExecutePostFunction in our thread context and finally
    - Asks the trigger thread to exit.

	\param rInterp - Reference to the interpreter on which this command is
	                 running.
	\param rResult - Reference to the object encapsulation of the 
	                result string which will be returned by 
			command.
	\param argc,argv - The parameters of the command, argv[0] is the
	               command itself.

     \note No parameters are expected or allowed.

*/
int 
CEndCommand::operator()(CTCLInterpreter& rInterp, CTCLResult& rResult, 
			int argc, char** argv)  
{
  // validate the parameters (none expected or allowed);

  argc--; argv++;		// Skip the command name.
  if(argc) {
    Usage(rResult);
    return TCL_ERROR;
  }
  // Attempt the transition.. catch exceptions which may indicate an
  // invalid transition.

  CReadoutMain* pMain = CReadoutMain::getInstance();
  CRunState*    pState= pMain->getRunState();
  try {
    pState->End();
    pMain->getExperiment()->Stop(*this);
  }
  catch (CException& rExcept) {
    rResult = rExcept.ReasonText();
    return TCL_ERROR;
  }
  catch (...) {
    rResult = "Unrecognized exception caught while ending run\n";
    return TCL_ERROR;    
  }
  return TCL_OK;
}  

/*!
    Executes the PostEnd procedure; returns the return value from
    that script as well as the return status code from that script.

      NO-OP at present.
*/
int 
CEndCommand::ExecutePreFunction()  
{
 
}  

/*!
    Executest the PostEnd script.  Returns the result and status
    code of that script.


*/
int 
CEndCommand::ExecutePostFunction()  
{
 
}
/*!
   Append usage information to the result string
   \param rResult - The result string to append to.

   */

void
CEndCommand::Usage(CTCLResult& rResult)
{
  rResult  = "Usage:\n";
  rResult += "   end\n";
}
