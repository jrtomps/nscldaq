
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
#include "CTriggerLoop.h"
#include "CExperiment.h"
#include "CEventTrigger.h"

#include <StateException.h>
#include <CInvalidArgumentException.h>
#include <string>
#include <stdlib.h>
#include <CVMEInterface.h>
#include <os.h>

#include <stdio.h>

#include <iostream>

using namespace std;

static const unsigned DWELL_COUNT(100);

/*!
  Construct the trigger loop object.  We have lazy binding to the
  triggers themselves.  The rule is that the triggers are not allowed
  to be changed during running, but they could be changed between running
  periods.

  \param experiment - Reference to the experiment object.

*/
CTriggerLoop::CTriggerLoop(CExperiment& experiment) :
  m_pExperiment(&experiment),
  m_running(false),
  m_stopping(false)
{}

/*!
   Destrution is only provided for chaining.
*/
CTriggerLoop::~CTriggerLoop()
{}

/*!
    Start - ensure that we are not running.
            initialize the running/stopping vars
            and chain to the base class which actually starts the thread.

    \note By the time this exits, the trigger thread will have been scheduled
         at least once.
*/
void 
CTriggerLoop::start()
{
  if (!m_running) {
    m_running = false;
    m_stopping = false;
    Thread::start();

    // Now wait here until we know the thread is running
    
    while (!m_running) {
      Os::usleep(500);
    }
  }
  else {
    throw CStateException("running", "stopped", "While starting trigger thread");
  }
}

/*!
  Stop the thread.. this is supposed to be invoked from a thread other than
  the executing thread.
  We ensure that's the case.

  The stop consists of setting the stopping member and joining the thread.
*/
void
CTriggerLoop::stop(bool pausing)
{
  if (runningThread() != getId()) {
    m_stopping = true;
    m_pausing  = pausing;
  }
  else {
    char mypid[1000];
    char whybad[1000];

    sprintf(mypid, "%x", runningThread());
    sprintf(whybad, "stopping thread must not have thread id: %lx", 
	    getId());
    throw CInvalidArgumentException(string(mypid), string(whybad),
				    string("Scheduling a stop of data taking"));
  }

}
/*!
   This is the entry point to the thread.  For now we set running to true,
   invoke the main thread and then when that returns, reset running and stopping:

*/
void
CTriggerLoop::run()
{
  m_running  = true;
  m_stopping = false;
  
  mainLoop();

  m_running  = false;
  m_stopping = false;
}

/*!
   Main loop looks at event triggers and scaler triggers... invoking the
   correct experiment callback when a trigger fires.. until we're told to exit:
*/
void
CTriggerLoop::mainLoop()
{
  CEventTrigger* pEvent = m_pExperiment->getEventTrigger();
  CEventTrigger* pScaler= m_pExperiment->getScalerTrigger();

  if (pEvent)  pEvent->setup();
  if (pScaler) pScaler->setup();

  do {
    try {
      CVMEInterface::Lock();
      for (int i =0; i < DWELL_COUNT; i++) {
	if ((*pEvent)()) {
	  m_pExperiment->ReadEvent();
	}
	if ((*pScaler)()) {
	  m_pExperiment->TriggerScalerReadout();
	}
      }
    }
    catch(...) {
      CVMEInterface::Unlock();
      throw;
    }
    CVMEInterface::Unlock();
  }
  while(!m_stopping);
  // End of run scaler:


  m_pExperiment->ScheduleEndRunBuffer(m_pausing);

  return;
}
