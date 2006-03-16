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
//////////////////////////CPauseCommand.cpp file////////////////////////////////////
#include <config.h>
#include "CPauseCommand.h"                  
#include "CReadoutMain.h"
#include "CRunState.h"
#include <Exception.h>
#include <TCLResult.h>
#include <TCLInterpreter.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
/*!
  Default constructor creates the "pause" command.
*/
CPauseCommand::CPauseCommand () :
  CStateTransitionCommand("pause")
{

} 


// Functions for class CPauseCommand

/*!
  Executes the pause command.  The pause command requires
  an active run.  The run is temporarily suspended.  paused runs can be 
  either resumed or directly stopped.  There are two threads of interest:
  This thread ( the Tcl interpreter), and the trigger thread.  Actions done are:
  - The prePause TCL script is executed.  Failures in that script can abort 
    the pause.
  - The C++ electronics shutdown software is run.
  - The final scaler buffer is emitted.
  - The pause buffer is emitted.
  - The PostPause script is executed. failures in it result in non-fatal 
    warnings.
  - The trigger thread, which is blocked since we hold the mutex, is requested 
    to stop.  It will either timeout out and stop or respond to the next 
    trigger by stopping rather than processing it.
  
  \param rInterp - CTCLInterpreter& [in] reference to the interpreter object
                   that is running this command.
  \param rResult - CTCLResult& [inout] Reference to the result string that will
                   be modified by the execution of this command.  The  result
		   string will either be:
		   - empty on success.
		   - an error message on failure.
  \param argc - int [in] Number of words in the command.
  \param argv - char** [in] Pointer to the list of words on the command. Note
                   that:
		   # One round of substitution has already occured.
		   # The command itself is a word.
  
  */
int 
CPauseCommand::operator()(CTCLInterpreter& rInterp, CTCLResult& rResult,
			  int argc, char** argv)  
{
  // Validate the parameters.

  argc--;
  if(argc) {
    Usage(rResult);
    return TCL_ERROR;
  }
  //  Attempt the state transition.

  CReadoutMain* pMain = CReadoutMain::getInstance();
  CRunState*    pState= pMain->getRunState();

  try {
    pState->Pause();
    pMain->getExperiment()->Stop(*this);
  }
  catch (CException& rExcept) {
    rResult = rExcept.ReasonText();
    return TCL_ERROR;
  }
  catch (...) {
    rResult = "Unrecognized exception caught while pausing run\n";
    return TCL_ERROR;    
  }
  return TCL_OK;
  //
}  

/*!
    Executes the PrePause script.
    The result of the script is returned as the 
    current interpreter's result string and the
    status is the return value of this function.



*/
int 
CPauseCommand::ExecutePreFunction()  
{
  
}  

/*!
    Invokes the PostPause script.  The result of that
    script is set in our interpreter's result field.
    The status of the script is returned.



*/
int 
CPauseCommand::ExecutePostFunction()  
{
 
}


/*!
   Add command usage information to the rResult string:
*/
void
CPauseCommand::Usage(CTCLResult& rResult)
{
  rResult += "Usage:\n";
  rResult += "   pause";
}
