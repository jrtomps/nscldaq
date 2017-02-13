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
#include "CVariableBuffers.h"
#include <CCondition.h>
#include <CMutex.h>
#include <CDataSink.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CDuplicateSingleton.h>
#include <RunState.h>
#include <V12/DataFormat.h>
#include <V12/CRawRingItem.h>
#include <V12/CRingTextItem.h>
#include <RingIOV12.h>

#include <vector>
#include <string>
#include <time.h>
#include <iostream>

using namespace std;
using namespace DAQ::V12;

// Static class member data:

CVariableBuffers* CVariableBuffers::m_pInstance(0);
uint32_t CVariableBuffers::m_nSourceId(0);

/*!
   Construction requires that the instance variable be
   null.  Otherwise a duplicate singleton is thrown.
   this is why there are no initializers. I don't want to take
   any action until I know I'm a singleton.

   @param interp - Reference to the encapsulated Tcl interpreter
                   on which the commands will be registered.
*/
CVariableBuffers::CVariableBuffers(CTCLInterpreter& interp)
{
  if (!m_pInstance) {
    m_pVars               = new CDocumentedVars(interp);
    m_pTriggerGuard       = new CMutex;
    m_pTriggerSynchronize = new CConditionVariable;
    m_pInstance           = this;
    m_interpreterThread   = Tcl_GetCurrentThread();
  }
  else {
    throw CDuplicateSingleton("Construction", "CVariableBuffer");
  }
}
/*!
  Destruction will destroy the components and also null out the
  instance variable.  The condition variable is signalled first
  so anyone waiting on it is released.
*/
CVariableBuffers::~CVariableBuffers()
{
  m_pTriggerGuard->lock();
  m_pTriggerSynchronize->broadcast();
  m_pTriggerGuard->unlock();

  delete m_pVars;
  
  // Unfortunately I must let the mutex and condition variable leak
  // as there may be folks in the process of synchronizing with triggers
  // using them.  If I wanted to be cute I could use some reference counting
  // scheme.. but in practice since this destructor only gets called
  // as the program exits (if at all), that's overkill.

  m_pInstance = reinterpret_cast<CVariableBuffers*>(0);
}


/*!
  Return the instance variable.. If this is null the instance has not
yet been created.
*/
CVariableBuffers* 
CVariableBuffers::getInstance()
{
  return m_pInstance;
}


/*!
   Trigger the production of a run variable documentation event into the
   specified ring buffer object.  We have two cases to consider:
   - The triggering thread is the same as the interpreter thread.  In that
     case, createRunVaribleEvent can be directly called.
   - The triggering thread is not the same as the interpreter thread.  In that
     case the interpreter must be sent an event (in the Tcl sense of the word)
     requesting a synchronized dump of the run varibles.. using the 
     condition variable to make the caller block until this is done.

     @param pRing - Pointer to the ring buffer into which the event is placed:
*/
void
CVariableBuffers::triggerRunVariableBuffer(CDataSink* pRing,
					   uint64_t timeOffset) 
{
  triggerBuffer(pRing, HandleRunVarTrigger, createRunVariableEvent,
		timeOffset);
}

/*!
   Trigger the production of a state variable documentation event into the specified
   ring buffer.  This has all of the considerations of triggerRunVariableBuffer.
   the only difference is that different functions get called:
   @param pRing - Pointer to the ring buffer into which the event should be placed.


*/
void
CVariableBuffers::triggerStateVariableBuffer(CDataSink* pRing,
					     uint64_t timeOffset)
{
  triggerBuffer(pRing, HandleStateVarTrigger, createStateVariableEvent,
		timeOffset);
}





/*
 * Factor out the stuff needed to trigger a buffer of one type or another.
 *
 * Parameters:
 *   CDataSink* pRing      - Pointer to the ring into which the event is put.
 *   Tcl_EventProc* handler  - Function to handle tcl events should they need scheduling.
 *   Creator*      creator   - Function to create an event directly.
 */
void
CVariableBuffers::triggerBuffer(CDataSink* pRing,
				Tcl_EventProc* handler,
				CVariableBuffers::Creator* creator,
				uint64_t timeOffset)
{
  // This whole thing is done holding the mutex so that we ensure
  // only one run variable buffer is in process at a time.. you might
  // think event dispatch takes care of that, however this is no longer the
  // case if the interpreter thread wants a dump while another thread does.
  //


  Tcl_ThreadId current = Tcl_GetCurrentThread();
  m_pTriggerGuard->lock();
  if (current == m_interpreterThread) {
    (*creator)(pRing, timeOffset);
  }
  else {
    pTriggerEvent pEvent = reinterpret_cast<pTriggerEvent>(Tcl_Alloc(sizeof(TriggerEvent)));
    pEvent->s_RawEvent.proc = handler;
    pEvent->s_pRing         = pRing;
    pEvent->s_TimeOffset    = timeOffset;
    Tcl_ThreadQueueEvent(m_interpreterThread, 
			 reinterpret_cast<Tcl_Event*>(pEvent), TCL_QUEUE_TAIL);
    Tcl_ThreadAlert(m_interpreterThread);
    m_pTriggerSynchronize->wait(*m_pTriggerGuard);	// Wait until done... or tearing down.
  }
  m_pTriggerGuard->unlock();


}

/**
 * Create a run variable event forthe specified ring.
 * this just involves getting the run variables and, if there are any,
 * calling createDocEvent.
 * Parameters:
 *    CDataSink*  pRing  - Destination ringbuffer.
 */
void
CVariableBuffers::createRunVariableEvent(CDataSink* pRing, uint64_t tbase)
{
  CDocumentedVars::NameValuePairs info = m_pInstance->m_pVars->getRunVars();
  if (info.size() > 0) {
    createDocEvent(pRing, MONITORED_VARIABLES, info, tbase);
  }
}
/*
 * Similarly create a state variable event for the specified ring.
 */
void
CVariableBuffers::createStateVariableEvent(CDataSink* pRing, uint64_t tbase)
{
  CDocumentedVars::NameValuePairs info = m_pInstance->m_pVars->getStateVars();
  if (info.size() > 0) {
    createDocEvent(pRing, MONITORED_VARIABLES, info, tbase);
  }
}
/*
 * Create a documenation event from a set of name value pairs.
 * Doing this is a matter of generating set varname value strings
 * which is easily done by creating Tcl lists, and extracting their representation
 * [that gets quoting right] In keeping with the scaler behavior, we will add 
 * a source id and a NULL_TIMESTAMP in the body header. This ensures that there is always 
 * a body header. This won't create strange behavior. Not having the body header
 * does create strange behavior because you cannot easily figure out where the doc item
 * originated from after the event builder if it is missing.
 *
 * Parameters:
 *    CDataSink*   pRing     - Pointer to the ring buffer in which the event is made.
 *    uint16_t       eventType - Type of event
 *    CDocumentedVars::NameValuePairs& variabls - The variables to write.
 */
void
CVariableBuffers::createDocEvent(CDataSink* pRing,
				 uint16_t     eventType,
				 CDocumentedVars::NameValuePairs& variables,
				 uint64_t     tbase)
{
  vector<string> elements;
  CTCLInterpreter* pInterp = m_pInstance->m_pVars->getInterpreter();  
  for (int i=0; i < variables.size(); i++) {
    string name  = variables[i].first;
    string value = variables[i].second;
    

    CTCLObject command;
    command.Bind(*pInterp);
    command += string("set");
    command += name;
    command += value;

    elements.push_back(string(command));
  }
  RunState* pState = RunState::getInstance();
  time_t timestamp = time(NULL);

  timespec currentTime;
  uint64_t msTime;
  clock_gettime(CLOCK_REALTIME, &currentTime);
  msTime = currentTime.tv_sec;
  msTime *= 1000;
  msTime += currentTime.tv_nsec/(1000*1000);

  CRingTextItem item(eventType, 
      NULL_TIMESTAMP, 
      m_nSourceId, 
      elements,
      (msTime - tbase)/1000,
      timestamp);
  CRawRingItem serialItem(item);
  *pRing << serialItem;

}

/*
 * Services run variable packet event requests.  This requires:
 * - Getting the ring buffer from the event.
 * - calling createRunVariableEvent to create the event.
 * - Returining 1 to indicate the event can be de-queued.
 */
int
CVariableBuffers::HandleRunVarTrigger(Tcl_Event* evPtr, int flags)
{
  pTriggerEvent pEvent = reinterpret_cast<pTriggerEvent>(evPtr);
  CDataSink*    pRing  = pEvent->s_pRing;
  uint64_t      tBase  = pEvent->s_TimeOffset;

  m_pInstance->m_pTriggerGuard->lock();
  CVariableBuffers::createRunVariableEvent(pRing, tBase);
  m_pInstance->m_pTriggerSynchronize->signal();
  m_pInstance->m_pTriggerGuard->unlock();

  return 1;
}
/*
 *  Pretty much all the same stuff as HandleRunVarTrigger but
 *  call createStateVariableBuffer.
 */
int
CVariableBuffers::HandleStateVarTrigger(Tcl_Event* evPtr,
					int flags)
{
  pTriggerEvent pEvent = reinterpret_cast<pTriggerEvent>(evPtr);
  CDataSink*  pRing  = pEvent->s_pRing;
  uint64_t      tBase  = pEvent->s_TimeOffset;

  m_pInstance->m_pTriggerGuard->lock();
  CVariableBuffers::createStateVariableEvent(pRing, tBase);
  m_pInstance->m_pTriggerSynchronize->signal();
  m_pInstance->m_pTriggerGuard->unlock();

  return 1;  
}


void 
CVariableBuffers::setSourceId(uint32_t sourceId) 
{
  m_nSourceId = sourceId;
}

uint32_t CVariableBuffers::getSourceId() const
{
  return m_nSourceId;
}
