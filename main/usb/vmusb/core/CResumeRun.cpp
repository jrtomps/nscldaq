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
<<<<<<< HEAD
#include "CPreResumeCommand.h"
#include <stdexcept>
#include <Exception.h>

/////////////////// cannonicals //////////////////////
//////////////////////////////////////////////////////////////////


CResumeRun::CResumeRun(CTCLInterpreter& interp, CPreResumeCommand* pre) :
  CTCLObjectProcessor(interp, "resume"),
  m_pre(pre)
=======

using std::string;
using std::vector;

static const string usage(
"Usage:\n\
   resume");
//////////////////////////////////////////////////////////////////
/////////////////////////////// cannonicals //////////////////////
//////////////////////////////////////////////////////////////////


CResumeRun::CResumeRun(CTCLInterpreter& interp) :
  CTCLObjectProcessor(interp, "resume")
>>>>>>> master
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

<<<<<<< HEAD
  bindAll(interp, objv);
  try {
    requireExactly(objv, 1, "resume command requires no parameters");
    CRunState* pState = CRunState::getInstance();
    CRunState::RunState state = pState->getState();
    
    if ((state!= CRunState::Paused) && (state != CRunState::Resuming)) {
      throw std::logic_error("resume but state is neither paused nor resuming");
    }
    // If needed preresume:
    
    if(state == CRunState::Paused) {
      m_pre->perform();
    }
    
    // resume the run:
  
    CControlQueues* pRequest = CControlQueues::getInstance();
    pRequest->ResumeRun();
    pState->setState(CRunState::Active);
    
    
  }
  catch (std::string msg) {
      interp.setResult(msg);
      return TCL_ERROR;
  }
  catch (const char* msg) {
      interp.setResult(msg);
      return TCL_ERROR;        
  }
  catch (CException& e) {
      interp.setResult(e.ReasonText());
      return TCL_ERROR;
  }
  catch (std::exception& e) {
      interp.setResult(e.what());
      return TCL_ERROR;        
  }
  catch (...) {
      interp.setResult("prebegin - unexpected exception type");
      return TCL_ERROR;
  }
=======
  if (objv.size() != 1) {
    tclUtil::Usage(interp,
		   "Invalid parameter count",
		   objv,
		   usage);
    return TCL_ERROR;
  }
  CRunState* pState = CRunState::getInstance();
  if (pState->getState() != CRunState::Paused) {
    tclUtil::Usage(interp,
		   "Invalid run state, to resume must be paused",
		   objv, usage);
    return TCL_ERROR;
  }
  // resume the run:

  CControlQueues* pRequest = CControlQueues::getInstance();
  pRequest->ResumeRun();
  
>>>>>>> master

  return TCL_OK;
}
