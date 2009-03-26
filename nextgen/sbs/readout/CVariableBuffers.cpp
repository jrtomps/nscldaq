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
#include <CRingBuffer.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CDuplicateSingleton.h>
#include <CRingBuffer.h>
#include <CRingTextItem.h>
#include <RunState.h>


#include <vector>
#include <string>
#include <time.h>

using namespace std;

// Static class member data:

CVariableBuffers* CVariableBuffers::m_pInstance(0);


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
CVariableBuffers::triggerRunVariableBuffer(CRingBuffer* pRing) 
{
  triggerBuffer(pRing, HandleRunVarTrigger, createRunVariableEvent);
}

/*!
   Trigger the production of a state variable documentation event into the specified
   ring buffer.  This has all of the considerations of triggerRunVariableBuffer.
   the only difference is that different functions get called:
   @param pRing - Pointer to the ring buffer into which the event should be placed.


*/
void
CVariableBuffers::triggerStateVariableBuffer(CRingBuffer* pRing)
{
  triggerBuffer(pRing, HandleStateVarTrigger, createStateVariableEvent);
}





/*
 * Factor out the stuff needed to trigger a buffer of one type or another.
 *
 * Parameters:
 *   CRingBuffer* pRing      - Pointer to the ring into which the event is put.
 *   Tcl_EventProc* handler  - Function to handle tcl events should they need scheduling.
 *   Creator*      creator   - Function to create an event directly.
 */
void
CVariableBuffers::triggerBuffer(CRingBuffer* pRing,
				Tcl_EventProc* handler,
				CVariableBuffers::Creator* creator)
{
  // This whole thing is done holding the mutex so that we ensure
  // only one run variable buffer is in process at a time.. you might
  // think event dispatch takes care of that, however this is no longer the
  // case if the interpreter thread wants a dump while another thread does.
  //


  Tcl_ThreadId current = Tcl_GetCurrentThread();
  m_pTriggerGuard->lock();
  if (current == m_interpreterThread) {
    (*creator)(pRing);
  }
  else {
    pTriggerEvent pEvent = reinterpret_cast<pTriggerEvent>(Tcl_Alloc(sizeof(TriggerEvent)));
    pEvent->s_RawEvent.proc = handler;
    pEvent->s_pRing         = pRing;
    Tcl_ThreadQueueEvent(m_interpreterThread, 
			 reinterpret_cast<Tcl_Event*>(pEvent), TCL_QUEUE_TAIL);
    m_pTriggerSynchronize->wait(*m_pTriggerGuard);	// Wait until done... or tearing down.
  }
  m_pTriggerGuard->unlock();


}

/**
 * Create a run variable event forthe specified ring.
 * this just involves getting the run variables and, if there are any,
 * calling createDocEvent.
 * Parameters:
 *    CRingBuffer*  pRing  - Destination ringbuffer.
 */
void
CVariableBuffers::createRunVariableEvent(CRingBuffer* pRing)
{
  CDocumentedVars::NameValuePairs info = m_pInstance->m_pVars->getRunVars();
  if (info.size() > 0) {
    createDocEvent(pRing, MONITORED_VARIABLES, info);
  }
}
/*
 * Similarly create a state variable event for the specified ring.
 */
void
CVariableBuffers::createStateVariableEvent(CRingBuffer* pRing)
{
  CDocumentedVars::NameValuePairs info = m_pInstance->m_pVars->getStateVars();
  if (info.size() > 0) {
    createDocEvent(pRing, MONITORED_VARIABLES, info);
  }
}
/*
 * Create a documenation event from a set of name value pairs.
 * Doing this is a matter of generating set varname value strings
 * which is easily done by creating Tcl lists, and extracting their representation
 * [that gets quoting right]
 *
 * Parameters:
 *    CRingBuffer*   pRing     - Pointer to the ring buffer in which the event is made.
 *    uint16_t       eventType - Type of event
 *    CDocumentedVars::NameValuePairs& variabls - The variables to write.
 */
void
CVariableBuffers::createDocEvent(CRingBuffer* pRing,
				 uint16_t     eventType,
				 CDocumentedVars::NameValuePairs& variables)
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
  time_t timestamp;
  time(&timestamp);

  CRingTextItem item(eventType, elements,
		     pState->m_timeOffset,
		     timestamp);
  item.commitToRing(*pRing);

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
  CRingBuffer*  pRing  = pEvent->s_pRing;

  m_pInstance->m_pTriggerGuard->lock();
  CVariableBuffers::createRunVariableEvent(pRing);
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
  CRingBuffer*  pRing  = pEvent->s_pRing;

  m_pInstance->m_pTriggerGuard->lock();
  CVariableBuffers::createStateVariableEvent(pRing);
  m_pInstance->m_pTriggerSynchronize->signal();
  m_pInstance->m_pTriggerGuard->unlock();

  return 1;  
}
