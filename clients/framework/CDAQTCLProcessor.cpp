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
    Called just prior to operator() .. lock the mutex.
*/
void
CDAQTCLProcessor::preCommand()
{
  lock();
}
/*!
   Called just after operator() .. unlock the mutex.
*/
void
CDAQTCLProcessor::postCommand()
{
  unlock();
}
void
CDAQTCLProcessor::preDelete()
{
  lock();
}
void
CDAQTCLProcessor::postDelete()
{
  unlock();
}

void
CDAQTCLProcessor::lock()
{
  CApplicationSerializer* pSync = CApplicationSerializer::getInstance();
  pSync->Lock();
}
void CDAQTCLProcessor::unlock()
{
  CApplicationSerializer* pSync = CApplicationSerializer::getInstance();
  pSync->UnLock();

}

