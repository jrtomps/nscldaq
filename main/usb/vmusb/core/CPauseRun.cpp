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
#include "CPauseRun.h"
#include <TCLObject.h>
#include <TCLInterpreter.h>
#include <CRunState.h>
#include <CControlQueues.h>
#include <tclUtil.h>
#include <CPrePauseCommand.h>


using std::string;
using std::vector;

static const string usage(
"Usage:\n\
   pause");

//////////////////////////////////////////////////////////////////
/////////////////////////////// cannonicals //////////////////////
//////////////////////////////////////////////////////////////////

CPauseRun::CPauseRun(CTCLInterpreter& interp, CPrePauseCommand* pre) :
  CTCLObjectProcessor(interp, "pause") ,
  m_pPre(pre)
{}
CPauseRun::~CPauseRun()
{}

//////////////////////////////////////////////////////////////////
//////////////////// Command processing //////////////////////////
//////////////////////////////////////////////////////////////////

/*!
   To pause a run requires that:
   - The following prerequisites be made:
     - The pause command must have no extra command line parameters.
     - The run state must be Active.
   - We request the pause through the CControlQueues.

   \param interp : CTCLInterpreter& 
      Reference to the interpreter that is running this command.
   \param objv   : vector<CTCLObject>& 
      Reference to the command words.

*/
int
CPauseRun::operator()(CTCLInterpreter& interp,
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
  // check the state:

  CRunState* pState = CRunState::getInstance();
  CRunState::RunState state = pState->getState();
  
  if ((state != CRunState::Active)  && (state != CRunState::Pausing)) {
    tclUtil::Usage(interp,
		   "To pause, the run must be active or Pausing",
		   objv,
		   usage);
    return TCL_ERROR;
  }
  if (state == CRunState::Active) {
    m_pPre->perform();                     // Perform the pre-pause action.
  }

  CControlQueues* pRequest = CControlQueues::getInstance();

  pRequest->PauseRun();

  return TCL_OK;
}
