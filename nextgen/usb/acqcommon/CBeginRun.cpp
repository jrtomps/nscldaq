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
#include "Globals.h"
#include <TCLInterpreter.h>
#include <TCLInterpreterObject.h>
#include <TCLVariable.h>
#include <CAcquisitionThread.h>
#include <CRunState.h>
#include <CConfiguration.h>

using std::vector;
using std::string;

static const string usage(
"Usage:\n\
   begin");

/////////////////////////////////////////////////////////////////////////
//////////////////////////////// Canonicals /////////////////////////////
/////////////////////////////////////////////////////////////////////////

/*!
  Construct the begin command 
  \param interp : CTCLInterpreter&
     Interpreter on which this command will be registered.
*/
CBeginRun::CBeginRun(CTCLInterpreter& interp) :
  CTCLObjectProcessor(interp, "begin")
{}
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
  // Make sure all precoditions are met.

  if (objv.size() != 1) {
    string msg = "begin - incorrect number of command parameters\n";
    msg        += usage;
    interp.setResult(msg);
    return TCL_ERROR;
  }

  CRunState* pState = CRunState::getInstance();
  if (pState->getState() != CRunState::Idle) {
    string msg = "begin - Invalid run state for begin, be sure to stop the run\n";
    msg       += usage;
    interp.setResult(msg);
    return TCL_ERROR;
  }
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

  try {
    Globals::pConfig->processConfiguration(Globals::configurationFilename);
  }
  catch (...) {
    // Configuration file processing error of some sort...

    interp.setResult("Begin - configuration file processing failed");
    return TCL_ERROR;
		       
  }

  CAcquisitionThread* pReadout = CAcquisitionThread::getInstance();
  pReadout->Start();

  interp.setResult("Begin - Run started");
  return TCL_OK;
}
