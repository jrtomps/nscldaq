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
#include "CExperiment.h"
#include <CRingBuffer.h>
#include <CRingStateChangeItem.h>
#include <CRingScalerItem.h>
#include <CRingItem.h>
#include <CRingTextItem.h>
#include <CRingPhysicsEventCountItem.h>

#include <RunState.h>
#include <StateException.h>
#include <string.h>
#include <CCompoundEventSegment.h>
#include <CScalerBank.h>
#include <CTriggerLoop.h>
#include <CDocumentedPacketManager.h>
#include <CVariableBuffers.h>

#include <TCLApplication.h>

#include <vector>
#include <string>

using namespace std;


//
// This struct is used to the event used to schedule an end run with the interpreter.
//
typedef struct _EndRunEvent {
  Tcl_Event     tclEvent;
  bool         pause;
  CExperiment* pExperiment;	// Pointer to the experiment object.

} EndRunEvent, *pEndRunEvent;

extern CTCLApplication* gpTCLApplication; // We need this to get the tcl interp.

///////////////////////////////////////////////////////////////////////////////////////////

/*!
   Construction. 
   - Initialize all the pointers and things to zero.
   - If necessary create/format the ring.
   - Create the ring buffer object in which data will be placed.
   - Save the initial event buffer size.
*/
CExperiment::CExperiment(string ringName,
			 size_t eventBufferSize) :
  m_pRing(0),
  m_pRunState(0),
  m_pScalers(0),
  m_pReadout(0),
  m_pBusy(0),
  m_pEventTrigger(0),
  m_pScalerTrigger(0),
  m_pTriggerLoop(0),
  m_nDataBufferSize(eventBufferSize)


{
  if (!CRingBuffer::isRing(ringName)) {
    CRingBuffer::create(ringName);
  }

  m_pRing = new CRingBuffer(ringName, CRingBuffer::producer);

  m_pRunState = RunState::getInstance();

}

/*!
   Destruction goes back around deleting all this stuff.  In C++, a delete ona null
   pointer is a no-op and therefore not a problem.
*/
CExperiment::~CExperiment()
{
  delete m_pRing;
  delete m_pScalers;
  delete m_pReadout;
#if 0				// These types/classes are not yet defined :-(
  delete m_pBusy;
  delete m_pEventTrigger;
  delete m_pScalerTrigger;
#endif

}
/////////////////////////////////////////////////////////////////////////////////////////

/*!
  Sets a new size for the event buffer.  This must be at least big enough to hold a
  single event.

*/
void
CExperiment::setBufferSize(size_t newSize)
{
  m_nDataBufferSize = newSize;
}
/*!
   Returns the size of the data buffer.
*/
size_t
CExperiment::getBufferSize() const
{
  return m_nDataBufferSize;
}

///////////////////////////////////////////////////////////////////////////////////////

/*!
  Start a new run. 
  - This is illegal if a run is active.
  - Writes a begin to the ring.
  - This creates and starts a new trigger thread, which can dispatch triggers to us.

  \param resume - true if this is actually a resume run.
*/
void
CExperiment::Start(bool resume)
{
  // The run must be in the correct state:

  if (resume && (m_pRunState->m_state != RunState::paused)) {
    throw CStateException(m_pRunState->stateName().c_str(),
			  RunState::stateName(RunState::paused).c_str(),
			  "Starting data taking");
  }
  if (!resume && (m_pRunState->m_state != RunState::inactive)) {
    throw CStateException(m_pRunState->stateName().c_str(),
			  RunState::stateName(RunState::inactive).c_str(),
			  "Starting data taking");
  }
  // Initialize/clear the hardware:

  if (m_pReadout) {
    m_pReadout->initialize();
    m_pReadout->clear();
  }
  if (m_pScalers) {
    m_pScalers->initialize();
    m_pScalers->clear();
  }
  // Begin run zeroes the previous scaler time, and ring buffer item sequence.
  // 
  //
  if (m_pRunState->m_state != RunState::paused) {
    m_nEventsEmitted  = 0;
    m_nLastScalerTime = 0;

  }
      
  time_t  now;
  time(&now);
  CRingStateChangeItem item(resume ? PAUSE_RUN : BEGIN_RUN, 
			    m_pRunState->m_runNumber,
			    m_pRunState->m_timeOffset = (resume ? m_pRunState->m_timeOffset : 0),
			    now,
			    std::string(m_pRunState->m_pTitle));
  item.commitToRing(*m_pRing);
  
  DocumentPackets();		// output a documentation packet.

  // emit a physics event count item that indicates no events have been sent yet
  // if this is a start run:

  if (!resume) {
    CRingPhysicsEventCountItem count;
    count.commitToRing(*m_pRing);
  }

  // Start the trigger loop:

  if (!m_pTriggerLoop) {
    m_pTriggerLoop = new CTriggerLoop(*this);
  }

  // Can only start it if the triggers have been established:
  
  if (m_pEventTrigger && m_pScalerTrigger) {
    m_pTriggerLoop->start();
  }

  // The run is now active if looked at by the outside world:

  m_pRunState->m_state = RunState::active;

}
/*!
  End an existing run.  
  - This is illegal if the run is halted already.
  - This marks the event trigger thread for exit and 
    joins with it to ensure that it does actually exit.
  - Writes an end run record to the ring.

  \param pause - true if this is actually a pause run.
*/
void
CExperiment::Stop(bool pause)
{
  if (pause && (m_pRunState->m_state != RunState::active)) {
    throw CStateException(m_pRunState->stateName().c_str(),
			  RunState::stateName(RunState::active).c_str(),
			  "Stopping data taking");
  }
  if (!pause && (m_pRunState->m_state == RunState::inactive)) {
    string validStates;
    validStates  = RunState::stateName(RunState::active);
    validStates += ", ";
    validStates += RunState::stateName(RunState::paused);

    throw CStateException(m_pRunState->stateName().c_str(),
			  validStates.c_str(), 
			  "Stopping data taking");
  }
  // Schedule the trigger thread to stop.

  if (m_pTriggerLoop) {
    if (m_pRunState->m_state == RunState::active) {
      m_pTriggerLoop->stop(pause); // run is active
    }
    else {
      ScheduleEndRunBuffer(false); // for ending paused we need to schedule.
      
    }

  }
  // Everything else is scheduled by Tcl events from the trigger loop.

}
/*
** Called (indirectly) by the Tcl interpreter event dispatcher
** as a result of the trigger loops scheduling the end of run action.
*/
void
CExperiment::syncEndRun(bool pause)
{

  readScalers();		// last scaler read.

  // Disable the hardware:

  if (m_pScalers) {
    m_pScalers->disable();
  }
  if (m_pReadout) {
    m_pReadout->disable();
  }
    
  // 

  // Create the run state item and commit it to the ring.

  time_t now;
  time(&now);
  uint16_t        itemType;
  RunState::State finalState;
  if (pause) {
    itemType   = PAUSE_RUN;
    finalState = RunState::paused;
  }
  else {
    itemType   = END_RUN;
    finalState = RunState::inactive;
  }

  CRingStateChangeItem item(itemType, 
			    m_pRunState->m_runNumber,
			    m_pRunState->m_timeOffset,
			    now,
			    std::string(m_pRunState->m_pTitle));
  item.commitToRing(*m_pRing);


  // The run is in the appropriate inactive state if looked at by the outside world

  m_pRunState->m_state = finalState;
}

//////////////////////////////////////////////////////////////////////////////////////

/*!
   Append an event segment to the event readout list
   - If the root event segment does not yet exist, create it.
   \param pSegment - Pointer to the new segment to install.  Note this could in turn
                     be a compound event segment.
*/
void
CExperiment::AddEventSegment(CEventSegment* pSegment)
{
  if (! m_pReadout) {
    m_pReadout = new CCompoundEventSegment;
  }
  m_pReadout->AddEventSegment(pSegment);
}
/*!
  Remove an existing event segment from the readout.  Note that if the event 
  segment is actually a compound, its members are also removed.
  The removed event segment is not deleted. Storage management is up to the
  higher levels of the world.
  \param pSegment - Pointer to the segment to remove.  It is not an error to remove
                   a segment that is not in the readout.. that's just a no-op.
*/
void
CExperiment::RemoveEventSegment(CEventSegment* pSegment)
{
  // If the root event segment does not exist, there's nothing to delete:

  if (m_pReadout) {
    m_pReadout->DeleteEventSegment(pSegment);
  }
}
/*!
   Add a scaler module to the end of the root scaler bank.  
   - If the root bank does not yet exist it will be created.
   - the module added could itself be a scaler bank.
   
   \param pScalerModule - pointer to the module to add.
*/
void
CExperiment::AddScalerModule(CScaler* pScalerModule)
{
  // if the root scaler module has not yet been created create it:

  if (!m_pScalers) {
    m_pScalers = new CScalerBank;
  }
  // Add the new module:

  m_pScalers->AddScalerModule(pScalerModule);
}
/*!
   Remove a scaler module from the scaler hierarchy.  
   - If the module is not in the hierarhcy this is a no-op.
   - If the module is a composite, all of the elements it contains
     will also be removed.
   - The removed module (and any children) won't be deleted.  Storage
     management is the job of higher level code.
*/
void
CExperiment::RemoveScalerModule(CScaler* pScalerModule)
{
  // remove the module from the root level scaler bank, if it exists.

  if (m_pScalers) {
    m_pScalers->DeleteScaler(pScalerModule);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////

/*!
   Establishes the trigger module. The trigger module is consulted by the
   trigger thread to know when to invoke ReadEvent.
   \param pEventTrigger - pointer to the event trigger.
*/
void
CExperiment::EstablishTrigger(CEventTrigger* pEventTrigger) 
{
  m_pEventTrigger = pEventTrigger;
}

/*!
  Establishes the scaler trigger module.  This is periodically consulted to 
  know when it's time for the trigger thread to invoke TriggerScalerReadout
  \param pScalerTrigger - pointer to the scaler trigger object.
*/
void
CExperiment::setScalerTrigger(CEventTrigger* pScalerTrigger)
{
  m_pScalerTrigger = pScalerTrigger;
}

/*!

  Establishes the busy module. The busy module represents hardware that 
  describes to the outside world when the system is unable to react to any triggers.

  \param pBusyModule

*/
void
CExperiment::EstablishBusy(CBusy* pBusyModule)
{
  m_pBusy = pBusyModule;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/*!
   Reads an event. If the root event segment exists it is asked to read its
   data into the a physics ring item.
   The resulting event is placed in the ring buffer.

*/
void
CExperiment::ReadEvent()
{

  

  // If the root event segment exists, read it into the data buffer
  // and put the resulting event in the ring buffer:
  //
  if (m_pReadout) {
    CRingItem item(PHYSICS_EVENT, m_nDataBufferSize);
    uint16_t* pBuffer = reinterpret_cast<uint16_t*>(item.getBodyPointer());
    size_t nWords = m_pReadout->read(pBuffer +2 , m_nDataBufferSize);
    *(reinterpret_cast<uint32_t*>(pBuffer)) = nWords +2;
    m_pReadout->clear();	// do any post event clears.
    item.setBodyCursor(pBuffer + nWords+2);
    item.commitToRing(*m_pRing);
    m_nEventsEmitted++;

  }
  // TODO: Need to keep track of trigger counts and from time to time emit a 
  //      trigger count item.
}

/*
 * Read the scalers.
 */
void
CExperiment::readScalers()
{
  time_t           now     = time(&now);

  // can only do scaler readout if we have a root scaler bank:

  if (m_pScalers) {
    vector<uint32_t> scalers = m_pScalers->read();
    

    CRingScalerItem  item(m_nLastScalerTime,
			  m_pRunState->m_timeOffset,
			  now,
			  scalers);
    item.commitToRing(*m_pRing);
    m_nLastScalerTime = m_pRunState->m_timeOffset;
			  
  }
  // Regardless, let's emit a physics event count stamp:

  CRingPhysicsEventCountItem item(m_nEventsEmitted,
				  m_pRunState->m_timeOffset,
				  now);
  item.commitToRing(*m_pRing);
}

/*!
  Reads scaler data.  If the root scaler bank exists, it is asked to read its
  data and return a vector of uint32_t.  That vector is used as the scalers in a scaler
  event that is submitted to the ring buffer.

*/

void CExperiment::TriggerScalerReadout()
{


  readScalers();

  // For now documented variables are tied to this trigger too:

  ScheduleRunVariableDump();

}

/*!
   Dumps a documentation event that describes the documented packets to the ring
   buffer.  This usually happens as the run becomes active again.
   Implicit inputs are the documented packet manager which can produce the information we need.


*/
void
CExperiment::DocumentPackets()
{
  CDocumentedPacketManager* pMgr = CDocumentedPacketManager::getInstance();
  vector<string> packetDefs = pMgr->Format();

  // only emit a record if there are documented packets:

  if (packetDefs.size()) {
    time_t           now     = time(&now);
    CRingTextItem item(PACKET_TYPES, packetDefs,
		       m_pRunState->m_timeOffset,
		       now);
    item.commitToRing(*m_pRing);
  }
}

/*!
  Schedules a dump of the run variables.  Since this is invoked from the trigger thread,
  but references Tcl variables in the Tcl interpreter thread.  This is scheduled via
  a ThreadRequest that blocks us until the operation completes in the Tcl thread.
  Blocking also serializes access to the ring buffer.

*/
void
CExperiment::ScheduleRunVariableDump()
{
  CVariableBuffers* pVars = CVariableBuffers::getInstance();
  pVars->triggerRunVariableBuffer(m_pRing);
  pVars->triggerStateVariableBuffer(m_pRing);
}


/*!
  Return the current event trigger:
*/
CEventTrigger*
CExperiment::getEventTrigger()
{
  return m_pEventTrigger;
}
/*!
  Returnthe current scaler trigger:
*/
CEventTrigger*
CExperiment::getScalerTrigger()
{
  return m_pScalerTrigger;
}
/*!
   Schedule the end run buffer read out:
   @param pause  - true if this is a pause run.
*/
void
CExperiment::ScheduleEndRunBuffer(bool pause)
{

  pEndRunEvent pEvent = reinterpret_cast<pEndRunEvent>(Tcl_Alloc(sizeof(EndRunEvent)));
  pEvent->tclEvent.proc = CExperiment::HandleEndRunEvent;
  pEvent->pause         = pause;
  pEvent->pExperiment   = this;

  CTCLInterpreter* pInterp = gpTCLApplication->getInterpreter();
  Tcl_ThreadId     thread  = gpTCLApplication->getThread();

  Tcl_ThreadQueueEvent(thread, 
		       reinterpret_cast<Tcl_Event*>(pEvent),
		       TCL_QUEUE_TAIL);

}
/*!
  Handle the end run event by getting object context and 
 calling syncEndRun
*/
int CExperiment::HandleEndRunEvent(Tcl_Event* evPtr, int flags)
{
  pEndRunEvent pEvent = reinterpret_cast<pEndRunEvent>(evPtr);
  CExperiment* pExperiment  = pEvent->pExperiment;
  pExperiment->syncEndRun(pEvent->pause);
}

