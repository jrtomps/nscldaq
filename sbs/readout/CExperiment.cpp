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


using namespace std;

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
  m_nDataBufferSize(eventBufferSize),
  m_pDataBuffer(0)

{
  if (!CRingBuffer::isRing(ringName)) {
    CRingBuffer::create(ringName);
  }

  m_pRing = new CRingBuffer(ringName, CRingBuffer::producer);

}

/*!
   Destruction goes back around deleting all this stuff.  In C++, a delete ona null
   pointer is a no-op and therefore not a problem.
*/
CExperiment::~CExperiment()
{
  delete m_pRing;
#if 0				// These types/classes are not yet defined :-(
  delete m_pRunState;
  delete m_pScalers;
  delete m_pReadout;
  delete m_pBusy;
  delete m_pEventTrigger;
  delete m_pScalerTrigger;
#endif
  delete m_pDataBuffer;
}
/////////////////////////////////////////////////////////////////////////////////////////

/*!
  Sets a new size for the event buffer.  This must be at least big enough to hold a
  single event.

*/
void
CExperiment::setBufferSize(size_t newSize)
{
  delete []m_pDataBuffer;
  m_pDataBuffer     = 0;
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
*/
void
CExperiment::Start()
{
  // TODO:  Write the body of this stub function.
}
/*!
  End an existing run.  
  - This is illegal if the run is halted already.
  - This marks the event trigger thread for exit and 
    joins with it to ensure that it does actually exit.
  - Writes an end run record to the ring.
*/
void
CExperiment::Stop()
{
  // TODO:  Write the body of this stub function.
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
  // TODO:  Write the body of this stub fucntion
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
  // TODO: Write the body of this stub function.
}
/*!
   Add a scaler module to the end of the root scaler bank.  
   - If the root bank does not yet exist it will be created.
   - the module added could itself be a scaler bank.
   
   \param pScalerModule - pointer to the module to add.
*/
void
CExperiment::AddScalerModule(CScalerModule* pScalerModule)
{
  // TODO: Write the body of his stub function.
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
CExperiment::RemoveScalerModule(CScalerModule* pScalerModule)
{
  // TODO:  Write the body of this stub function.
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
   data into the m_pDataBuffer (which is created if not yet allocated).
   The resulting event is placed in the ring buffer.

*/
void
CExperiment::ReadEvent()
{
  // TODO:  Write the body of this stub function.
}

/*!
  Reads scaler data.  If the root scaler bank exists, it is asked to read its
  data and return a vector of uint32_t.  That vector is used as the scalers in a scaler
  event that is submitted to the ring buffer.

*/

void CExperiment::TriggerScalerReadout()
{
  // TODO: Write the body of this stub function.

}

/*!
   Dumps a documentation event that describes the documented packets to the ring
   buffer.  This usually happens as the run becomes active again.

*/
void
CExperiment::DocumentPackets()
{
  // TODO:  Write the body of this stub function.
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
  // TODO: Write the body of this stub function.
}
