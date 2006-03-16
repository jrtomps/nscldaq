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


static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";/*!
  Provides a synchronized TCL command.
           Inheriting from this class allows you to produce
           a TCL command which is synchronized to the
           application through the application mutex.
             This member essentially just replaces the
           TCLProcessor's registration procedures
           and static callback relay.  The static callback
           relay will now lock the application mutex prior
           to calling operator() and unlock on return
           (exception or normal).
           
 */

////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include <TCLInterpreter.h>
#include <TCLResult.h>
#include <TCLProcessor.h>
#include "CDAQTCLProcessor.h"    				
#include "CApplicationSerializer.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


/*!  Destructor:  The base class will take care of everything we
   need.  The fact that register registered our DeleteRelay will
   take care of ensuring that OnDelete is executed with application
   synchronization.
 */
CDAQTCLProcessor::~CDAQTCLProcessor ( )  //Destructor - Delete dynamic objects
{
}

/*! Constructor.  Builds a new command which will execute synchronized
  with all other threads in the application.
 */

CDAQTCLProcessor::CDAQTCLProcessor(const string&    rCommand,
				   CTCLInterpreter* pInterp) : 
  CTCLProcessor(rCommand, pInterp)
{
}

/*!  Constructor, Builds a new command which will execute synchronized
  with all the other threads in the application.

 */
CDAQTCLProcessor::CDAQTCLProcessor(const char* pCommand, 
				   CTCLInterpreter* pInterp) :
  CTCLProcessor(pCommand, pInterp)
{
  
}
// Functions for class CDAQTCLProcessor

/*!
  Registers the processor on the current interpreter. This reimplements code
from the base class because I need to specify my own Eval and Delete relay 
functions (there's naturally no way for static functions to be virtual).

 */
void 
CDAQTCLProcessor::Register()  
{
  CTCLInterpreter* pInterp = AssertIfNotBound();
  

  pInterp->AddCommand(getCommandName(), EvalRelay, (ClientData)this,
		      DeleteRelay);
  //  AddRegisteredOnCurrent();
}  

/*!

Locks the application mutex, calls
operator() and the unlocks the resource.

*/
int 
CDAQTCLProcessor::EvalRelay(ClientData pData, Tcl_Interp* pInterp, int Argc, 
#if (TCL_MAJOR_VERSION > 8) || ((TCL_MAJOR_VERSION ==8) && (TCL_MINOR_VERSION > 3))
			 const char** Argv)
#else
			 char** Argv)
#endif
{ 
   CApplicationSerializer* pSynch = CApplicationSerializer::getInstance();

   pSynch->Lock(); {		//<-- Begin critical region.
   try {
      CTCLProcessor*   pProcessor = (CTCLProcessor*)pData;
      CTCLInterpreter* pInterpObject = pProcessor->getInterpreter();
      CTCLResult       result(pInterpObject); 
      int status = (*pProcessor)(*pInterpObject, result, Argc, (char**)Argv);
      result.commit();
      return status;
// CTCLProcessor::EvalRelay(pData, pInterp, Argc, Argv);
    }
    catch(...)			// Ensure the mutex is released.
      {
      }
  }
  pSynch->UnLock();		//<-- End critical region.
}  

/*!
Locks the application mutex, call's
the object's OnDelete member function
(the object is pointed to by the client data
parameter), and unlocks the mutex.

*/
void 
CDAQTCLProcessor::DeleteRelay(ClientData pData)  
{
  CApplicationSerializer* pSynch = CApplicationSerializer::getInstance();
  pSynch->Lock(); {		//<-- Begin Critical region.
    try {
      CTCLProcessor* pProcesor = (CTCLProcessor*)pData;
      pProcesor->OnDelete();
      //      CTCLProcessor::DeleteRelay(pData);
    } 
    catch(...) {}		// Ensure we don't exception out before
				// releasing the lock.
  }
  pSynch->UnLock();		//<-- End Critical region.
}

