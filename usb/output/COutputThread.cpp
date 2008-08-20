

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
#include "COutputThread.h"
#include "CRunState.h"
#include "DataBuffer.h"
#include <string>
#include <Exception.h>


#include <assert.h>
#include <buftypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <CRingBuffer.h>
#include <CRingItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CRingScalerItem.h>
#include <CRingStateChangeItem.h>
#include <DataFormat.h>


#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>


static const uint64_t eventCountRate(2000);


using namespace std;



////////////////////////////////////////////////////////////////////////
///////////////////// Construction and destruction /////////////////////
////////////////////////////////////////////////////////////////////////

/*!
   Create an outupt thread.  Should only create 1 however if the
   following were parameters could create multiple:
   - Event buffer queue.
   - Free buffer queue.
   - Run state.
   - Open the ring.

   In future applications this could be done to manage multiple VM-USB
   controlled VME crates in 'singles mode'... or with a subsequent chunk of
   software on the end of spectrodaq assembling data.

*/
COutputThread::COutputThread() : 
  m_pRing(0)
{
 openRing();			// Open ring named after the user in localhost.
                                // the ring is created if necessary.
}
/*!
   Disconnect from the ring.
*/
COutputThread::~COutputThread()
{
  delete m_pRing;
}

////////////////////////////////////////////////////////////////////////
////////////////////// Thread entry point... ///////////////////////////
////////////////////////////////////////////////////////////////////////

/*
   Thread entry point.  This is just an infinite buffer processing loop.
*/
void
COutputThread::run()
{
  // Main loop is pretty simple.
  try {
    while(1) {
      
      DataBuffer& buffer(getBuffer());
      processBuffer(buffer);
      freeBuffer(buffer);
      
    }

  }
  catch (string msg) {
    cerr << "COutput thread caught a string exception: " << msg << endl;
    throw;
  }
  catch (char* msg) {
    cerr << "COutput thread caught a char* exception: " << msg << endl;
    throw;
  }
  catch (CException& err) {
    cerr << "COutputThread thread caught a daq exception: "
	 << err.ReasonText() << " while " << err.WasDoing() << endl;
    throw;
  }
  catch (...) {
    cerr << "COutput thread caught some other exception type.\n";
    throw;
  }

}


/*
   Get a buffer from the event queue.  A reference to the buffer will
   be returned.

   Note that this will block if needed to wait for a buffer.
   note as well that between runs, we'll wind up blocking in here.

*/
DataBuffer&
COutputThread::getBuffer()
{
  DataBuffer* pBuffer = gFilledBuffers.get(); // Will block if needed.
  return *pBuffer;

}
/*
   Free a buffer that has been completely processed.
   This will return the buffer to the gFreeBuffers queue.
   Parameters:
      DataBuffer& buffer   - Reference to the buffer to return.

*/
void
COutputThread::freeBuffer(DataBuffer& buffer)
{
  gFreeBuffers.queue(&buffer);
}
/*
   Process a buffer from the reader.  At this time we are just going
   to figure out what type of buffer we have and dispatch accordingly.
   Buffers are as follows:
   Begin run is indicated by the type in the DataBuffer, all others look like
      data buffers.
   End run is indicated by the last bit set in the vmusb header.
   scaler is indicated by the scaler bit in the vm usb header.
   All others are event data.
   Parameters:
     DataBuffer&   buffer   - The buffer from the readout thread.
*/
void
COutputThread::processBuffer(DataBuffer& buffer)
{
  if (buffer.s_bufferType == 1) {
    startRun(buffer.s_timeStamp);
  }
  else if (buffer.s_bufferType == 2) {
    endRun(buffer.s_timeStamp);
  }
  else {
    processUSBData(buffer);	// will callback.
  }
}


/*
  Create a begin run item on behalf of the concrete subclass/thread.
  \param when - Time the run began.
  \note Much of the information about the run is implicit input from the
       runstat object.
*/
void
COutputThread::startRun(time_t when)
{
  // Update our concept of run state, and buffer size:

  CRunState* pState      = CRunState::getInstance();
  m_runNumber            = pState->getRunNumber();
  m_title                = pState->getTitle();
  m_startTimestamp       = when;
  m_lastStampedBuffer    = 0;
  m_eventCount           = 0;

  CRingStateChangeItem item(BEGIN_RUN,
			    m_runNumber,
			    0,
			    when,
			    m_title);
  item.commitToRing(*m_pRing);
			    
       
}
/*
  Called to emit an end run item for the 
  caller.
  \param when - timestamp of the end run.
*/
void
COutputThread::endRun(time_t when)
{

  CRingStateChangeItem item(END_RUN,
			    m_runNumber,
			    when - m_startTimestamp,
			    when,
			    m_title);
  item.commitToRing(*m_pRing);

}

/*
  Create a scaler item for the caller.
  \param when  - Absolute time the scaler buffer is being emitted.
  \param number- Number of scalers in the item.
  \param pScalers - Pointer to the scalers.

*/
void
COutputThread::scaler(time_t when, int number, uint32_t* pScalers)
{
  
  uint32_t startTime = m_lastStampedBuffer - m_startTimestamp;
  uint32_t endTime   = when - m_startTimestamp;;
  m_lastStampedBuffer= when;

  CRingScalerItem  item(number);
  item.setStartTime(startTime);
  item.setEndTime(endTime);
  item.setTimestamp(when);
  for (int i = 0; i < number; i++) {
    item.setScaler(i, *pScalers++);
  }
  item.commitToRing(*m_pRing);

}
/*!
   Create an event item.
   \param size - number of uint16_t's in the event.
   \param pData- Pointer to the data words.

*/

void
COutputThread::event(uint32_t size, void* pData)
{
  uint32_t bytes = size*sizeof(uint16_t); // USB device sizes are not self inclusive.
  CRingItem item(PHYSICS_EVENT, bytes);

  uint8_t* pDest = reinterpret_cast<uint8_t*>(item.getBodyCursor());
  memcpy(pDest, pData, bytes);

  pDest += bytes;
  item.setBodyCursor(pDest);
  
  item.commitToRing(*m_pRing);

  m_eventCount++;
}	     
/*!
   Produce an event count item on behalf of the client.
   We've been maintaining a count of the events in a run for the caller.
   \param when -time when this should be emitted.

*/
void
COutputThread::eventCount(time_t when)
{
  CRingPhysicsEventCountItem item(m_eventCount,
				  when - m_startTimestamp,
				  when);
  item.commitToRing(*m_pRing);

}

/*!
   Process data from a VM/CC-USB event buffer.  We're going to collect
   events and pass them on to the event function until we run out of data.
   For buffers that don't match, this can be overridden.
   \param buffer -Reference to the data buffer struct.

*/
void 
COutputThread::processUSBData(DataBuffer& buffer)
{
  uint32_t  nWordsLeft = bodySize(buffer);
  uint16_t* pData      = buffer.s_rawData;
  unsigned  nEvents    = eventCount(buffer);
  unsigned  hWords     = headerSize(buffer);
  nWordsLeft -= hWords;
  pData      += hWords;
  time_t    timestamp  = buffer.s_timeStamp;


  while (nWordsLeft && nEvents) { // Exit either if out of events or out of data;

    uint32_t nWords = eventSize(pData);
    processEvent(timestamp, nWords, pData);

    pData += nWords;
    nWordsLeft -= nWords;
    nEvents--;
  }
  // Note that both nWordsLeft and nEvents should hit zero at the same time!!

  if (nWordsLeft != nEvents) {
    cerr << "Words in buffer not consistent with events in buffer:\n";
    cerr << "Both should now be zero but WordsLeft = " << nWordsLeft << endl;
    cerr << "                            EventsLeft= " << nEvents << endl;
    cerr << "Continuing with the next buffer\n";
  }

  // After the buffer, put out an event count record:

  eventCount(timestamp);

}

//////////////////////////////////////////////////////////////////////////////////
//
// Utilities.
//

/*
** Open the ring.  The ring name is just the user name.
*/
void
COutputThread::openRing()
{
  uid_t uid = getuid();
  passwd* pPass = getpwuid(uid);
  string name(pPass->pw_name);

  // If necessary create the ring:.. for now with default characteristics.

  if (!CRingBuffer::isRing(name)) {
    CRingBuffer::create(name);
  }

  m_pRing = new CRingBuffer(name, CRingBuffer::producer);
}
