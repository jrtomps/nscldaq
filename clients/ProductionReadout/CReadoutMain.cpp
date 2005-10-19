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
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
#include <spectrodaq.h>
#include "CReadoutMain.h"                  
#include <Exception.h>
#include "CTclInterpreterShell.h"
#include "CTkInterpreterShell.h"
#include "CInterpreterStartup.h"
#include "CInterpreterCore.h"
#include "CStateVariableCommand.h"
#include "CStateVariable.h"

#include "CVMETrigger.h"
#include "CCAMACTrigger.h"
#include "CVMEStatusModule.h"
#include "CCAMACStatusModule.h"
#include "CReaper.h"
#include "CTCLListener.h"

#include <CApplicationSerializer.h>

#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Iostream.h>
#include <unistd.h>

#include "cmdline.h"		// Hopefully last will stop redef issues.

static const long VMETRIGGERBASE(0x444400);


extern CReadoutMain MyApp;

/*!
   Default constructor.    This member is called for the single instance
   of the object.. which is statically declared.

*/


CReadoutMain::CReadoutMain () 

   : m_fWindowed(false),   
     m_fServer(false),   
     m_fExit(false),
     m_nPort(2701),   
     m_fVmeTrigger(true) ,
     m_pInterpreter(0) ,
     m_TimerQueue(*(new CTimer())),
     m_Experiment(*(new CExperiment))
{

}
/*!
   Destructor.  Race conditions make it safer to not destroy anything.

   */
CReadoutMain::~CReadoutMain()
{

}


// Selectors which are not inline implemented:



// Functions for class CReadoutMain

/*!
    Entry point to the program.  The flow is relatively simple:
    We parse the switches, Create the interpreter, Create the experiment
    and Create the timer manager, and then block to let the child threads 
    run.

	\param argc - Count of command line arguments.
	\param argv - Array o fpointers to command line arguments.  Note that
	             argv[0] is the path name to this program.

	\bug May need to replace sleep loop with wait on a condition which
             would be signalled by Tcl interpreter's exit.
*/
int 
CReadoutMain::operator()(int argc , char** argv)  
{
  try {
    // Parse command line parameters and make appropriate objects.
    
    CApplicationSerializer::getInstance()->Lock();
    ParseSwitches(argc, argv);
    
    CReaper::getInstance();                 // Forces creation of the grim reaper thread.
    
    CreateExperiment();
    CreateInterpreter();
    
    CApplicationSerializer::getInstance()->UnLock();
    
    // Just sleep loop.  The interpreter will exit us.
    
    
    while(!m_fExit) {
      sleep(1);			// Exit within a second of being asked.
    }
    exit(0);
  }
  catch (string msg) {
    cerr << "String exception caught initializing the program: " << msg << endl;
  }
  catch (char* msg) {
    cerr << "char* exception caught initializing the program: " << msg << endl;
  }
  catch (CException& exception) {
    cerr << "CException caught initializing the program : "
	 << exception.ReasonText() << " while " << exception.WasDoing() << endl;
  }
  catch (...) {
    cerr << "An unanticpated exception type was caught while initializing Readout\n";
  }
}  

/*!
    Parses the command line switches into
    the member variables.  Switches supported
    are described in the CReadoutMain class
    description.
    

	\param argc  - Number of command line parameters.
	\param argv  - Vector of command line pointers.  argv[0] is the
                       program name.

   We'll allow illegal switches since they may be directed at tcl/tk.
   we won't allow illegal formats of switches however.
 
   We make use of the gnu getgetopts cmdline parser.  Our
   parse 'grammer' is in options.ggo
*/
void 
CReadoutMain::ParseSwitches(int argc, char** argv)  
{
  gengetopt_args_info parse;

  if(cmdline_parser(argc, argv, &parse)) exit(-1); // Parse the line.
  if( m_fServer = parse.port_given) {
    m_nPort = parse.port_arg;

  }
  if(parse.window_flag) {
    cerr << "Due to some rather nasty to fix bugs, --window is not supported\n";
    exit(-1);
  }
  m_fWindowed = parse.window_flag;
  m_fVmeTrigger = ! parse.camac_trigger_flag;
}  


/*!
    Based on  the member data:
    
    m_fWindowed - true selects a Tk based interpreter,
    
   and schedules it to start up. The startup will register standard
   Readout Tcl extensions and variables, and then callback our members
   to register custom extensions.


*/
void 
CReadoutMain::CreateInterpreter()  
{
  if(m_fWindowed) {
    CTkInterpreterShell* pInterp = new CTkInterpreterShell;
    m_pInterpreter = pInterp;
    daq_dispatcher.Dispatch(*pInterp); // *pInterp has the right static type.
  }
  else {
    CTclInterpreterShell* pInterp = new CTclInterpreterShell;
    m_pInterpreter = pInterp;
    daq_dispatcher.Dispatch(*pInterp); // *pInterp has the right static type.
  }
}  

/*!
       Based on the member data m_fVmeTrigger creates an experiment
    object and configures it with the appropriate trigger/clear modules.
    The experiment object configures the rest of itself based on the
    experiment's Initialize callout which performs experiment specific 
    initialization including and not limited to configuring the EventReader
    component.


*/
void 
CReadoutMain::CreateExperiment()  
{
  if(m_fVmeTrigger) {
    m_Experiment.EstablishTrigger(new CVMETrigger(VMETRIGGERBASE));
    m_Experiment.EstablishBusy(new CVMEStatusModule(VMETRIGGERBASE));
  } 
  else {
    m_Experiment.EstablishTrigger(new CCAMACTrigger(0));
    m_Experiment.EstablishBusy(new CCAMACStatusModule(0,2, 19));
  }

  // Now invoke the experiment specific overrides:

  SetupReadout(m_Experiment);
  SetupScalers(m_Experiment);

}  

/*!

      This must be overridden to create the user's readout procedures

    \param rExperiment - reference to the experiment object.
    */
void
CReadoutMain::SetupReadout(CExperiment& rExperiment)
{
}

/*!
  This must be overridden to create the user's scaler readout procedure.
  \param rExperiment - reference to the experiment object.

  */
void
CReadoutMain::SetupScalers(CExperiment& rExperiment)
{
}
/*!
  This must be overridden if the experimenter wants to provide any 
  initial run variables. 
  \param rExperiment - reference to the experiment object.
  \param rStartup    - Reference to the Tcl interpreter startup thread.
  \param rCore       - Reference to the core readout Tcl extensions.

  The default implementation is a no-op and can be overridden.

  \note This member is called back from the interpreter core in the context
  of the interpreter startup thread.  Therefore, no syncrhonization is required
  with the interpreter.

  */
void
CReadoutMain::SetupRunVariables(CExperiment& rExperiment,
				CInterpreterStartup& rStartup,
				CInterpreterCore&    rCore)
{
}

/*!
   This must be overridden if the experimenter wants to provide any run
   state variables. 
    If you override this be sure to call the base class member e.g.:
      CReadoutMain::SetupStateVariables(rExperiment, Interp) as the
      first executable line.

   \param rExperiment - Reference to the experiment object.
   \param rStartup    - Reference to the interpreter startup thread.
   \param rCore       - Reference to the set of core readout extensions.

   The default implementation can be overridden and is an no-op.
   \note  This function is called in the context of the tcl interpreter
   thread and therefore does not need to synchronize with it in any way.

   */
void
CReadoutMain::SetupStateVariables(CExperiment&         rExperiment,
				  CInterpreterStartup& rStartup,
				  CInterpreterCore&    rCore)
{


}


/*!
  Must be overridden if you want to add additional readout specific commands:
  \param rExperiment - Refers to the experiment.
  \param rStartup    - Refers to the interpreter startup thread.
  \param rCore       - Refers to the core readout tcl extensions.

  The default implementation is a no-op and can be overridden.

  \note This member is called int he context of the interpreter thread 
        therefore noe synchronization is required with the interpreter.
  */
void
CReadoutMain::AddUserCommands(CExperiment&         rExperiment, 
			      CInterpreterStartup& rStartup,
			      CInterpreterCore&    rCore)
{
}


/*!
   Returns the current run title.  This is the value of the 
   title run state variable.  If that run state variable is not yet
   set we return -not set-.
*/
string
CReadoutMain::getTitle() const
{
  string unset("-not set=");

  // Get a pointer to the state variable command object...
  // it holds the dictionary of run state variables (including title).

  CInterpreterCore*      pCore    = m_pInterpreter->getInterpreterCore();
  CStateVariableCommand& rCommand(*(pCore->getStateVariables()));

  // Locate the title command:

  StateVariableIterator  i = rCommand.find(string("title"));
  if(i == rCommand.end()) {
    return unset;
  }
  CStateVariable* pTitle = i->second;
  if(!pTitle) {			// Not likely but let's be protective
    return unset;
  }
  // Now fetch the value of the command:

  const char* pValue = pTitle->Get(TCL_GLOBAL_ONLY);
  if(!pValue) {
    return unset;
  }

  return string(pValue);

}


/*!
   Returns the scaler readout period in seconds.
   \note Scaler timers require a period in ms so 
   be sure to scale this appropriately.
*/
unsigned int
CReadoutMain::getScalerPeriod() const
{
  const unsigned int    nDefaultPeriod(10);
  CInterpreterCore      *pCore = m_pInterpreter->getInterpreterCore();
  CStateVariableCommand& rState(*(pCore->getStateVariables()));

  // Try to locate the "period" variable.  If
  // not found return the default period.

  StateVariableIterator i = rState.find("frequency");
  if(i == rState.end()) {
    return nDefaultPeriod;
  }

  CStateVariable* pVar= i->second;
  if(!pVar) {
    return nDefaultPeriod;
  }

  // Now get the value of the state variable.
  // if it does not decode as an integer
  // greater than zero, return the default:

  const char* pValue = pVar->Get(TCL_GLOBAL_ONLY);
  if(!pValue) {
    return nDefaultPeriod;
  }
  int nValue = atoi(pValue);
  if(nValue <= 0) {
    return nDefaultPeriod;
  }
  return (unsigned int)nValue;

}


/*!
  Get a pointer to the single instance of a ReadoutMain object
  which can exist in the system.
  */
CReadoutMain*
CReadoutMain::getInstance() {
  return &MyApp;
}

void
CReadoutMain::Exit()
{
  CReaper::getInstance()->clear(); // Don't reap threads that are being
  m_fExit = true;		// destroyed.
}


// If spectrodaq main is separable, then I need to define main
// here to ensure that TCL++'s main is not pulled in by mistake.
//

#ifdef HAVE_SPECTRODAQ_MAIN
int
main(int argc, char** argv, char** envp) 
{
  return spectrodaq_main(argc, argv, envp);
}


#endif
