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
#include "CResumeRun.h"
#include <TCLObject.h>
#include <TCLInterpreter.h>
#include <CRunState.h>
#include <CControlQueues.h>
#include <tclUtil.h>
#include "CPreResumeCommand.h"

using std::string;
using std::vector;

static const string usage(
"Usage:\n\
   resume");
//////////////////////////////////////////////////////////////////
/////////////////////////////// cannonicals //////////////////////
//////////////////////////////////////////////////////////////////


CResumeRun::CResumeRun(CTCLInterpreter& interp, CPreResumeCommand* pre) :
  CTCLObjectProcessor(interp, "resume"),
  m_pre(pre)
{}

CResumeRun::~CResumeRun()
{}

/*!
   -To resume a run requies the followig prerequisites be made:
     - The command must contain only the resume keyword.
     - The run state must be paused.
   - The run is actually resumed via the control queues.

   \param interp : CTCLInterpreter& 
      Reference to the interpreter that is running this command.
   \param objv   : vector<CTCLObject>& 
      Reference to the command words.

*/
int
CResumeRun::operator()(CTCLInterpreter& interp,
		      vector<CTCLObject>& objv)
{
  // Check the prereqs:

  if (objv.size() != 1) {
    tclUtil::Usage(interp,
		   "Invalid parameter count",
		   objv,
		   usage);
    return TCL_ERROR;
  }
  CRunState* pState = CRunState::getInstance();
  CRunState::RunState state = pState->getState();
  if ((state != CRunState::Paused) && (state != CRunState::Resuming)) {
    tclUtil::Usage(interp,
		   "Invalid run state, to resume must be paused",
		   objv, usage);
    return TCL_ERROR;
  }
  
  // if needed, pre-resume:
  
  if (state == CRunState::Paused) {
    m_pre->perform();
  }
  
  // resume the run:

  CControlQueues* pRequest = CControlQueues::getInstance();
  pRequest->ResumeRun();
  

  return TCL_OK;
}
