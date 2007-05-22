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
//////////////////////////CBeginCommand.cpp file////////////////////////////////////

#include <config.h>
#include <TCLResult.h>
#include "CBeginCommand.h"                  
#include "CExperiment.h"
#include "CReadoutMain.h"
#include "CBadTransitionException.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
/*!

  Construct a begin command.
*/
CBeginCommand::CBeginCommand () :
  CStateTransitionCommand("begin")
{

} 

// Functions for class CBeginCommand

/*!
    Executes the begin command.  For the most part,
    all we do is validate that the run can be started, and
    then ask the experiment to start itself.   The experiment
    will perform all initializations in our context:
    - PreBegin script which, if it fails, makes the begin fail in error.
    - Emission of the Begin Run Buffer.
    - C++ Initializations of hardware.
    - PostBegin script which, if it fails causes the begin to succeed
      but the result code warns of this fact.
    - Removal of computer busy signal.
    - Schedule the execution of the trigger thread.
    Readout will then occur in a separate thread, allowing the command
    interpreter to continue to 'run'.

	\param rInterp    - The interpreter on which the command is running.
	\param rResult    - The result of to be returned to the interpreter
	                    (empty string if ok error message otherwise).
	\param argc,argv  - The parameters for the command.

     Syntax:
  
     begin

     No parameters are allowed or supported.

*/
int 
CBeginCommand::operator()(CTCLInterpreter& rInterp, CTCLResult& rResult, 
			  int argc, char** argv)  
{
  // Validate the command arguments (there should be none).

  argc--; argv++;

  if(argc) {
    Usage(rResult);
    return TCL_ERROR;
  }
  
  // Determine if the run state allows this transition to occur:

  CReadoutMain* pRdo = CReadoutMain::getInstance();
  CRunState* pState = pRdo->getRunState();
  try {
    pState->Begin();		// Try the begin transition.
    pRdo->getExperiment()->Start(*this); // And start data taking.
  }
  catch (CException& rExcept) {
    rResult = rExcept.ReasonText();
  }
  catch (...) {
    rResult = "Unrecognized exception caught while starting run.\n";
    return TCL_ERROR;
  }

  return TCL_OK;

}  

/*!
    Executes the PreBegin script; returning the status and result string.
    \note This is a callback given to the Experiment begin run function so that
        it does not need to know how to execute Tcl scripts.

*/
int 
CBeginCommand::ExecutePreFunction()  
{
 
}  

/*!
    Called to execute the PostBegin script (if defined).
    \note This will be called back from the start function of CExperiment
    so that that object doesn't have to know much about scripts.
    etc.

       

*/
int 
CBeginCommand::ExecutePostFunction()  
{
 
}

/*!
   Provide command online help.

   \param CTCLResult& rResult:
       Reference to the result string.
 */
void
CBeginCommand::Usage(CTCLResult& rResult)
{
  rResult  = "Usage: \n";
  rResult += "   begin\n";
    
}
