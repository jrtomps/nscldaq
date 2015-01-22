/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

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
#include "CTriggerCommand.h"
#include "CCAENModule.h"
#include <string>
#include <TCLInterpreter.h>
#include <TCLResult.h>
#include "CDigitizerDictionary.h"
#include <Active.h>
#include <StateMachine.h>

using namespace std;


extern StateMachine* gpStateMachine;
/*!

   Construct a trigger command object.  The trigger
   command object responds to the \em trigger
   command which is used to select a module to
   serve as the event trigger.

   \param rCommand (const string& [in]):
       Name of the command ... "trigger" usually.
   \param rInterp (CTCLInterpreter& [in]):
       The interpreter on which this command will
       be entered.
   \param pDictionary (CModuleDictionary* [in]):
       Pointer to the module dictionary that will
       have information about modules that are
       elligible to be triggers.
*/
CTriggerCommand::CTriggerCommand(const string& rCommand,
				 CTCLInterpreter& rInterp,
				 CDigitizerDictionary* pDictionary) :
  CTCLProcessor(rCommand, &rInterp),
  m_pDictionary(pDictionary),
  m_pModule(0),
  m_pTrigger(0)
{
  Register();
}
/*!
  Destroy the command processor.  This involves 
  deleting the  trigger.. note that the
  dictionary and trigger belong to external clients. 
*/
CTriggerCommand::~CTriggerCommand()
{
  delete m_pTrigger;
}
/*!
  Process the trigger command.  The trigger command
  expects a sigle parameter, the name of a digitizer module
  that will be used to trigger the readout.
  At the time of this command, the dictionary must have
  an entry for this digiitzer.
  \param rInterp (CTCLINterpreter& rInterp[in]):
     The interpreter that's executing this command. // 
  \param rResult (CTCLResult& rResult [out]):  
     The result of the command for successs, this is
     just the name of the trigger module.  For failure,
     a reason for the failure.
  \param int argc, char**argv:
     The parameters (argv[0] is our command name.
 
   \return One of:
   - TCL_OK    - If success.
   - TCL_ERROR - If failure.
*/
int
CTriggerCommand::operator()(CTCLInterpreter& rInterp,
			    CTCLResult&      rResult,
			    int argc, char** argv)
{
  argc--; argv++;		// Ignore the command name.
  if(argc != 1) {		// Need exactly 1 parameter.
    Usage(rResult);
    return TCL_ERROR;
  }
  char* pTriggerName = *argv;	// And that's the tyrigger.
  CDigitizerDictionary::ModuleIterator i = 
    m_pDictionary->DigitizerFind(string(pTriggerName));
  if(i == m_pDictionary->DigitizerEnd()) {
    rResult = "Module not found: ";
    rResult+= pTriggerName;
    return TCL_ERROR;
  }
  m_pModule = dynamic_cast<CCAENModule*>(i->second);
  rResult   = pTriggerName;
  return TCL_OK;
}
/*!
  Initialize the trigger module.  This involves
  creating a CAENTrigger from the trigger module.
*/
void
CTriggerCommand::Initialize()
{
  m_pTrigger = new CCAENTrigger(m_pModule->getCard());
  Active* pActive = (Active*)gpStateMachine->GetCurrentStatePtr();
  pActive->SetTrigger(m_pTrigger);
  
}
/*!
   Put the command usage into a result string.
*/
void
CTriggerCommand::Usage(CTCLResult& rResult)
{
  string cmd = getCommandName();
  rResult += "Usage: \n";
  rResult += "   ";
  rResult += cmd;
  rResult += " trigger module\n";
  rResult += "Where:\n";
  rResult += "   module is the name of the trigger module\n";
    
}
