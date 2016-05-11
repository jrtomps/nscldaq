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
#include "CEndRun.h"
#include <TCLInterpreter.h>
#include <TCLInterpreterObject.h>
#include <Globals.h>
#include <CAcquisitionThread.h>
#include "tclUtil.h"

#include <CControlQueues.h>
#include <CRunState.h>
#include "CPreEndCommand.h"

using std::vector;
using std::string;

static string usage(
"Usage\n\
   end");

/////////////////////////////////////////////////////////////////
////////////////////////////// Canonicals. //////////////////////
/////////////////////////////////////////////////////////////////


CEndRun::CEndRun(CTCLInterpreter& interp, CPreEndCommand* preEnd) :
  CTCLObjectProcessor(interp, string("end")),
  m_preEnd(preEnd)
{}
CEndRun::~CEndRun()
{}

///////////////////////////////////////////////////////////////////
///////////////// Command processing //////////////////////////////
///////////////////////////////////////////////////////////////////

/*!
  Process the end command. 
  - Ensure that the prerequisites are met:
    - The end command has no extraneous stuff on the back end of it.
    - The current state is not Idle.
  - Request the readout thread to exit using CControlQueues.
  - Kill off the old configuration for next time.
*/
int
CEndRun::operator()(CTCLInterpreter& interp, 
		    vector<CTCLObject>& objv)
{
  // Check pre-requisites:

  if (objv.size() != 1) {
    tclUtil::Usage(interp,
		   "Incorrect number of command parameters",
		   objv,usage);
    return TCL_ERROR;
  }
  CRunState* pState = CRunState::getInstance();
  CRunState::RunState state = pState->getState();
  
  if (
    (state != CRunState::Active) && (state != CRunState::Paused) &&
    (state != CRunState::Ending)
  ) {
    
  
    tclUtil::Usage(interp,
		   "Invalid state for end run we must be in one of Active, Paused or Ending states",
		   objv, usage);
    return TCL_ERROR;
  }
  // If not ending we need to fire off the preend operations.
  
  if (state != CRunState::Ending) {
    m_preEnd->perform();              // Now in ending.
  }


  // Now stop the run... if the thread has not already exited:

  if(CAcquisitionThread::getInstance()->isRunning()) {
    
    CControlQueues* pRequest = CControlQueues::getInstance();
    pRequest->EndRun();
 

    CAcquisitionThread::waitExit();
  }
  //  delete Globals::pConfig;	// Delete the old configuration database.

  pState->setState(CRunState::Idle);

  return TCL_OK;
}
