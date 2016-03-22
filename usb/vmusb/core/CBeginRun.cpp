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
#include "CBeginRun.h"
#include <TCLInterpreter.h>
#include <TCLInterpreterObject.h>
#include <TCLVariable.h>
#include <Globals.h>
#include "tclUtil.h"
#include <CAcquisitionThread.h>
#include <CRunState.h>
#include <CConfiguration.h>
#include <iostream>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <CReadoutModule.h>
#include <TclServer.h>
#include <CPreBeginCommand.h>

using std::vector;
using std::string;
using std::cerr;
using std::endl;

static const string usage(
"Usage:\n\
   begin");

static const size_t MAX_STACK_STORAGE(4096/sizeof(uint32_t));

/////////////////////////////////////////////////////////////////////////
//////////////////////////////// Canonicals /////////////////////////////
/////////////////////////////////////////////////////////////////////////

/*!
  Construct the begin command 
  \param interp : CTCLInterpreter&
     Interpreter on which this command will be registered.
  @param pre - pointer to the pre-begin command we use for our pre-begin run actions.
*/
CBeginRun::CBeginRun(CTCLInterpreter& interp, CPreBeginCommand* pre)) :
  CTCLObjectProcessor(interp, "begin"), m_preBegin(0)
{
    m_preBegin = pre;
}
/*!
   Destructor does nothing important.
*/
CBeginRun::~CBeginRun()
{}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////// Command execution /////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*!
   Process the begin run.
   - Ensure that the preconditions for starting the run are met these are:
     - The begin command has no additional command line parameters.
     - The current run state is Idle
   - Create the Adc/Scaler configuration.
     - Configure it from the file stored in Globals::configurationFilename.
     - Store that configuration in Globals::configuration
   - Start up the readout thread to take data.

   \param interp : CTCLInterpreter&
        Interpreter that is exeuting this command.
   \param objv : std::vector<CTCLObject>&
        Reference to an object vector that contains the command parameters.
*/
int
CBeginRun::operator()(CTCLInterpreter& interp,
		      vector<CTCLObject>& objv)
{
  // Make sured all precoditions are met.

  if (objv.size() != 1) {
    tclUtil::Usage(interp, 
		   "Incorrect number of command parameters",
		   objv,
		   usage);
    return TCL_ERROR;
  }

  CRunState* pState = CRunState::getInstance();
  CRunState::RunState state = pState->getState();
  if ((state != CRunState::Idle) && (state != CRunState::Starting)) {
    tclUtil::Usage(interp,
		   "Invalid run state for begin be sure to stop the run",
		   objv,
		   usage);
    return TCL_ERROR;
  }
 
  Globals::pConfig = new CConfiguration;
  
  // If we are idle -- need to prebegin:
  
  if (state == CRunState::Idle)  {
    m_preBegin->perform();                      // Pre begin operations.
  }
  // Now the state is 'starting'.
  
  // Set the state to match the appropriate set of variables:
  //
  CTCLVariable run(&interp, "run", false);
  const char* runNumberString = run.Get(TCL_GLOBAL_ONLY);
  if (!runNumberString) {
    runNumberString = "0";	// If no run variable, default run number-> 0.
  }
  uint16_t runNumber;
  sscanf(runNumberString, "%hu", &runNumber);
  pState->setRunNumber(runNumber);

  CTCLVariable title(&interp, "title", false);
  const char *titleString = title.Get(TCL_GLOBAL_ONLY);
  if (!titleString) {
    titleString = "No Title Set"; // If no title variable default it.
  }
  pState->setTitle(string(titleString));
  
  // Now we can start the run.


  CAcquisitionThread* pReadout = CAcquisitionThread::getInstance();
  pReadout->start(Globals::pUSBController);

  tclUtil::setResult(interp, string("Begin - Run started"));
  return TCL_OK;
}

