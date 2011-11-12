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
#include <Globals.h>
#include <TclServer.h>

#include <assert.h>
#include <buffer.h>
#include <buftypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <iostream>



using namespace std;

static DataBuffer* lastBuffer(0);
static const unsigned ReadoutStack(0);
static const unsigned ScalerStack(1);


///////////////////////////////////////////////////////////////////////
///////////////////// Local data types ////////////////////////////////
//////////////////////////////////////////////////////////////////////

typedef struct _BeginRunBuffer {
  BHEADER         s_header;
  struct ctlbody  s_body;
} BeginRunBuffer, *pBeginRunBuffer;

typedef struct _ScalerBuffer {
  BHEADER         s_header;
  struct sclbody  s_body;
} ScalerBuffer, *pScalerBuffer;

typedef struct _EventBuffer {
  BHEADER      s_header;
  uint16_t     s_body[1];
} EventBuffer, *pEventBuffer;

////////////////////////////////////////////////////////////////////////
//   mytimersub - since BSD timeval is not the same as POSIX timespec:
////////////////////////////////////////////////////////////////////////
static inline void 
mytimersub(timespec* minuend, timespec* subtrahend, timespec* difference)
{
  // We'll cheat and map these to timevals, use timersub and convert back:
  // this means we're only good to a microsecond not a nanosecond _sigh_
  // if this is not good enough we'll do the subtraction manually later.

  timeval m,s,d;
  m.tv_sec   = minuend->tv_sec;
  m.tv_usec  = minuend->tv_nsec/1000;

  s.tv_sec   = subtrahend->tv_sec;
  s.tv_usec  = subtrahend->tv_nsec/1000;

  timersub(&m, &s, &d);

  difference->tv_sec  = d.tv_sec;
  difference->tv_nsec = d.tv_usec * 1000;
}

////////////////////////////////////////////////////////////////////////
///////////////////// Construction and destruction /////////////////////
////////////////////////////////////////////////////////////////////////

/*!
   Create an outupt thread.  Should only create 1 however if the
   following were parameters could create multiple:
   - Event buffer queue.
   - Free buffer queue.
   - Run state.

   In future applications this could be done to manage multiple VM-USB
   controlled VME crates in 'singles mode'... or with a subsequent chunk of
   software on the end of spectrodaq assembling data.

*/
COutputThread::COutputThread() : 
  m_sequence(0),
  m_nOutputBufferSize(0),		// Don't know yet.
  m_pBuffer(0),
  m_pCursor(0),
  m_pEventStart(0),
  m_nWordsInBuffer(0),
  m_nEventsInBuffer(0)
{
  
}
/*!
  Destruction is a no-op at this time.
*/
COutputThread::~COutputThread()
{
}

////////////////////////////////////////////////////////////////////////
////////////////////// Thread entry point... ///////////////////////////
////////////////////////////////////////////////////////////////////////

/*
   Thread entry point.  This is just an infinite buffer processing loop.
*/
int
COutputThread::operator()(int argc, char** argv)
{
  // Main loop is pretty simple.
#ifdef LAST_CHANCE
  try {
#endif
    while(1) {
      
      DataBuffer& buffer(getBuffer());
      processBuffer(buffer);
      freeBuffer(buffer);
      
    }
#ifdef LAST_CHANCE
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
#endif
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////// Utility functions ///////////////////////////
/////////////////////////////////////////////////////////////////////////


/*
   Send a formatted buffer to spectrodaq.
   Parameters:
     void*        pBuffer            - The formatted buffer.
     unsigned int sdaqTag            - How to tag the buffer.
     size_t       sdaqWords          - How big a Spectrodaq buffer to create.
     size_t       copInSize          - words from pBuffer to copy to the spectrodaq
                                       buffer.

    Note that the spectrodaq buffer will be created, filled and routed
    in this function.
*/
void
COutputThread::bufferToSpectrodaq(void*          pBuffer,
				  unsigned int   sdaqTag,
				  size_t         sdaqWords,
				  size_t         copyInSize)
{
  if (copyInSize > sdaqWords) {
    cerr << "COutputThread::bufferToSpectrodaq - copy in size too big: "
	 << "buffersize: " << sdaqWords << " copyInSize: " << copyInSize << endl;
    copyInSize = sdaqWords;	// Truncate and try to keep going.

  }
  DAQWordBuffer sdaqBuffer(sdaqWords);
  sdaqBuffer.SetTag(sdaqTag);
  sdaqBuffer.CopyIn(pBuffer, 0, copyInSize);
  sdaqBuffer.Route();

  // This will finalize the buffer.

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
    startRun(buffer);
  }
  else if (buffer.s_bufferType == 2) {
    endRun(buffer);
  }
  else {
    processEvents(buffer);
  }
}

/*
   Process a begin run pseudo buffer. I call this a psuedo buffer because
   there will not be any data from the VM_usb for this
   We must:
   - Update our concept of the run state.
   - Set the m_nOutputBufferSize accordingly.
   - Create an NSCL begin run buffer and
   - send it to spectrodaq.
   - Destroy the NSCL begin run buffer we created.
   
   Parameters:
     DataBuffer&  buffer   - the buffer from the buffer queue.
*/
void
COutputThread::startRun(DataBuffer& buffer)
{
  cerr << "Buffer multiplier (COutputThread) : " << Globals::bufferMultiplier << endl;

  // Figure out the buffers size:

  m_nOutputBufferSize = Globals::bufferMultiplier*26*1024 + 32;

  // Update our concept of run state, and buffer size:

  CRunState* pState = CRunState::getInstance();
  m_runNumber       = pState->getRunNumber();
  m_title           = pState->getTitle();
  m_sequence        = 0;

  clock_gettime(CLOCK_REALTIME, &m_startTimestamp);
  m_lastStampedBuffer = m_startTimestamp;
  m_elapsedSeconds = 0;


  // Allocate the Begin run buffer and fill it in.

  pBeginRunBuffer p = static_cast<pBeginRunBuffer>(malloc(sizeof(BeginRunBuffer)));

  formatControlBuffer(BEGRUNBF, p);
  

  // Submit the buffer to spectrodaq as tag 2 and free it.

  bufferToSpectrodaq(p, 3, m_nOutputBufferSize/sizeof(uint16_t), 
		     sizeof(BeginRunBuffer)/sizeof(uint16_t));
  free(p);
}
/*
  Called when an end of run has occured.  The end of run
  with a VMUSB is a data buffer.  We submit that data buffer.
  by calling events, and then we generate an end of run buffer.
  for the DAQ system to use to note the change of run state.



  Parameters:
  DataBuffer& buffer  - The data from the readout thread.


*/
void
COutputThread::endRun(DataBuffer& buffer)
{
  // Flush any existing output buffer:

  flush();

  // Process the data buffer:
  
  
  pBeginRunBuffer p = static_cast<pBeginRunBuffer>(malloc(sizeof(BeginRunBuffer)));
  formatControlBuffer(ENDRUNBF, p);
  
  // Submit to spectrodaq and free:
  
  bufferToSpectrodaq(p, 3, m_nOutputBufferSize/sizeof(uint16_t),
		     sizeof(BeginRunBuffer)/sizeof(uint16_t));
  free(p);
}

/**
 * Process events in a buffer creating output buffers as required.
 *  - Figure out the used words in the buffer
 *  - For each event in the buffer invoke either event or scaler depending on
 *    the stack number.  Stack 1 is always a scaler event while any other stack
 *    is considered a physics event.
 *
 * @param inBuffer  -  Reference to a raw input buffer.
 */
static uint32_t  bufferNumber = 0; 
void 
COutputThread::processEvents(DataBuffer& inBuffer)
{
  uint16_t* pContents = inBuffer.s_rawData;
  int16_t  nEvents   = (*pContents) & VMUSBNEventMask;
  bool     continuous = ((*pContents) & VMUSBContinuous) != 0;
  bool     multibuffer = ((*pContents) & VMUSBMultiBuffer) != 0;

  bufferNumber++;
  if(continuous) {
    cerr << "Buffer number " << bufferNumber << " is in continuous mode\n";
  }
  if (multibuffer) {
    cerr << "Buffer number " << bufferNumber << "Spans buffer boundaries. \n";
  }


  pContents++;			// Point to first event.
  ssize_t    nWords    = (inBuffer.s_bufferSize)/sizeof(uint16_t) - 1; // Remaining words read.


  while (nWords > 0) {
    if (nEvents <= 0) {
      // Next long should be 0xffffffff buffer terminator:

      uint32_t* pNextLong = reinterpret_cast<uint32_t*>(pContents);
      if (*pNextLong != 0xffffffff) {
	cerr << "Ran out of events but did not see buffer terminator\n";
	cerr << nWords << " remaining unprocessed\n";
      }

      break;			// trusting event count vs word count(?).
    }
    uint16_t header     = *pContents;
    size_t   eventLength = header & VMUSBEventLengthMask;
    uint8_t  stackNum   = (header & VMUSBStackIdMask) >> VMUSBStackIdShift;

    if (stackNum == ScalerStack) {
      scaler(pContents);
    }
    else if (stackNum == 7) {
      sendToTclServer(pContents);
    }
    else {
      event(pContents);
    }
    // Adjust pointer to the next event and decrement the size:
    
    pContents += eventLength + 1; // Event count is not self inclusive.
    nWords    -= (eventLength + 1);
    nEvents--;
  }
  if (nWords < 0) {
    cerr << "Warning used up more than the buffer  by " << (-nWords) << endl;
  }
}

/**
 * Process a scaler event:
 * - If there is an existing started output buffer it is flushed.
 * - A new buffer is allocated.
 * - The header is populated with the usual stuff for scalers
 * - The body is populated with the body of the event.
 * @note We assume scaler events won't consist of more than 2048
 *       scalers (e.g. the continuation bit is not set.
 * @param pData - Pointer to scaler data.
 */
void
COutputThread::scaler(void* pData)
{
  if (m_pBuffer) {
    fillEventHeader();
    flush();
  }
  m_pBuffer = newOutputBuffer();			// Scaler events stand alone in their own buffer.

  uint16_t* pHeader = reinterpret_cast<uint16_t*>(pData);
  uint16_t  header  = *pHeader;
  uint32_t* pBody   = reinterpret_cast<uint32_t*>(pHeader+1); // Poiner to the scalers.

  if (header & VMUSBContinuation) {
    cerr << "Warning - scaler events requiring multiple segments are not supported!\n";
  }
  size_t        nWords   =  header & VMUSBEventLengthMask;
  size_t        nScalers =  nWords/(sizeof(uint32_t)/sizeof(uint16_t));
  pScalerBuffer outbuf   = reinterpret_cast<pScalerBuffer>(m_pBuffer);

  // Format the buffer header:

  outbuf->s_header.nwds  = sizeof(ScalerBuffer)/sizeof(uint16_t) + nWords -1;
  outbuf->s_header.type   = 2;
  outbuf->s_header.cks    = 0;
  outbuf->s_header.run    = m_runNumber;
  outbuf->s_header.seq    = m_sequence;
  outbuf->s_header.nevt   = nScalers;
  outbuf->s_header.nlam   = 0;
  outbuf->s_header.nbit   = 0;
  outbuf->s_header.buffmt = BUFFER_REVISION;
  outbuf->s_header.ssignature = 0x0102;
  outbuf->s_header.lsignature = 0x01020304;

  // Format the body prior to the scalers:

  // We only have the VM-USB's word on how often scaler readout occurs
  // because of mixed buffer mode:

 

  outbuf->s_body.btime    = m_elapsedSeconds;
  m_elapsedSeconds       += Globals::scalerPeriod;
  outbuf->s_body.etime    = m_elapsedSeconds;

  // Copy the scaler data:

  memcpy(outbuf->s_body.scalers, pBody, nScalers*sizeof(uint32_t));

  // scaler buffers always only have one scaler event:

  m_nWordsInBuffer = outbuf->s_header.nwds;

  flush();

}

/**
 * Flush an output buffer to spectrodaq
 * This means invoking bufferToSpectrodaq, freeing the data buffer
 * and resetting all the pointers.
 * If there is no databuffer yet, this is a no-op.
 */
void
COutputThread::flush()
{
  if(m_pBuffer) {
    // Figureout the tag:

    BHEADER* pHead = reinterpret_cast<BHEADER*>(m_pBuffer);
    int tag;
    if(pHead->type == 1) {
      tag = 2;
    } 
    else {
      tag = 3;
    }
    bufferToSpectrodaq(m_pBuffer, tag, 
		       m_nOutputBufferSize/sizeof(uint16_t), m_nWordsInBuffer);

    free(m_pBuffer);
    m_pBuffer = m_pCursor = m_pEventStart = 0;
    m_nWordsInBuffer = m_nEventsInBuffer = 0;

  }
}

/**
 * Handle a buffer overflow.  Only event buffers can overflow therefore  
 *  - Fill in the header of the buffer with the stuff needed to 
 *    Make it an event buffer.
 *  - Get a new buffer.
 *  - If there is a partial event, copy it from the old buffer into the new buffer.
 *  - flush the old buffer
 *  - Set up all the book keeping values for the register to reflect whatever we've done
 *    with respect to the partial event we may or may not have copied into the next buffer.
 */
void 
COutputThread::overflowOutputBuffer()
{
  fillEventHeader();
  uint8_t*  pNewBuffer = newOutputBuffer();
  uint8_t* pNewCursor = (&pNewBuffer[sizeof(BHEADER)]); // buffer body.
  uint8_t* pNewEventStart = pNewCursor;
  size_t   newBufferWords = sizeof(BHEADER)/sizeof(int16_t);
  

  // Send the output buffer to spectrodaq:

  bufferToSpectrodaq(m_pBuffer,
		     2,
		     m_nOutputBufferSize/sizeof(uint16_t),
		     m_nWordsInBuffer);


  // Copy any partial event to the new buffer:
  // and adjust the new stuff accordingly.
  if (m_pCursor != m_pEventStart) {
    uint32_t partialEventSize = m_pCursor - m_pEventStart;
    memcpy(pNewCursor, m_pEventStart, partialEventSize);
    newBufferWords += partialEventSize/sizeof(uint16_t);
    pNewCursor    += partialEventSize;
  }
  free(m_pBuffer);

  // Adjust the book keeping data:

  m_pBuffer         = pNewBuffer;
  m_pCursor         = pNewCursor;
  m_pEventStart     = pNewEventStart;
  m_nWordsInBuffer  = newBufferWords;
  m_nEventsInBuffer = 0;


}

/**
 * Create a new output buffer.
 * for now this is trivial
 * @return uint8_t*
 * @retval Pointer to the output buffer.
 */
uint8_t* 
COutputThread::newOutputBuffer()
{
  return reinterpret_cast<uint8_t*>(malloc(m_nOutputBufferSize));
}
/**
 * Process a single event segment to the output buffer.
 * If the continuation bit is set, the book keeping data is managed to reflect we have
 * a partial event.  If, the segment would overflow the existing buffer, overflowOutputBuffer
 * is invoked to send the complete events to the data distribution software and
 * we load the data into the new buffer created by that function
 * If this is not a continuation, we can book keep the completion of an event.
 * @param pSource - Pointer to the event data segment.
 */
void 
COutputThread::event(void* pData)
{
  // If necessary make an new output buffer

  if (!m_pBuffer) {
    m_pBuffer     = newOutputBuffer();
    m_pEventStart = m_pBuffer + sizeof(BHEADER);
    m_pCursor     = m_pEventStart;
    m_nWordsInBuffer = sizeof(BHEADER)/sizeof(uint16_t);
    m_nEventsInBuffer = 0;
  }

  // Initialize the pointers to event bits and pieces.

  uint16_t* pSegment = reinterpret_cast<uint16_t*>(pData);
  uint16_t  header   = *pSegment;

  // Figure out the header:
 
  size_t segmentSize = header & VMUSBEventLengthMask;
  bool   haveMore    = (header & VMUSBContinuation) != 0;
  
  // first question is whether or not this segment will fit in 
  // the output buffer.  If not, overflow it  so we have a new buffer to work in:

  if ((segmentSize + m_nWordsInBuffer) >= m_nOutputBufferSize/sizeof(uint16_t)) {
    overflowOutputBuffer();
  }
  // Next we can copy our data to the output buffer and update the cursro
  // remembering that the size is not self inclusive:
  //
  segmentSize += 1;
  memcpy(m_pCursor, pData, segmentSize*sizeof(uint16_t));
  m_nWordsInBuffer += segmentSize;

  // Different handling of the cursor and event start/event count 
  // depending on continuation or not.
  // Event start is normally kept the same as the event cursor but
  // not so if the event is segmented:

  m_pCursor += segmentSize*sizeof(uint16_t); // advance the cursor
    
  if (!haveMore) {			    // Ending segment:

    m_nEventsInBuffer++;
    m_pEventStart = m_pCursor;

  }
  

}




/**
 * Fill the contents of the event buffer header for a physics data buffer
 * Space for this has already been reserved for at the start of m_pOutputBuffer.
 */

void
COutputThread::fillEventHeader()
{
  BHEADER* pHeader = reinterpret_cast<BHEADER*>(m_pBuffer);
  pHeader->nwds    = m_nWordsInBuffer;
  pHeader->type    = DATABF;
  pHeader->cks     = 0;
  pHeader->run     = m_runNumber;
  pHeader->seq     = m_sequence++;
  pHeader->nlam    = 0;
  pHeader->nbit    = 0;
  pHeader->buffmt   = BUFFER_REVISION;
  pHeader->ssignature = 0x0102;
  pHeader->lsignature = 0x01020304;
  pHeader->nevt       = m_nEventsInBuffer;
  
}



/*
   Format a control buffer.  A control buffer is a begin or end run buffer.
   Parameters:
     uint16_t   type     - Type of the buffer (e.g. BEGRUNBF).
     void*      buffer   - Pointer to the buffer to forrmat.

*/
void
COutputThread::formatControlBuffer(uint16_t type, void* buffer)
{
  pBeginRunBuffer p = static_cast<pBeginRunBuffer>(buffer);

  // Header:
  
  p->s_header.nwds  = sizeof(BeginRunBuffer)/sizeof(uint16_t);
  p->s_header.type  = type;
  p->s_header.cks   = 0;	// Not using the checksum!!!
  p->s_header.run   = m_runNumber;
  p->s_header.seq   = m_sequence;
  p->s_header.nevt  = 0;
  p->s_header.nlam  = 0;
  p->s_header.cpu   = 0;
  p->s_header.nbit  = 0;
  p->s_header.buffmt= BUFFER_REVISION;
  p->s_header.ssignature = 0x0102;
  p->s_header.lsignature = 0x01020304;


  // Body:
  
  memset(p->s_body.title, 0, 80);
  strncpy(p->s_body.title, m_title.c_str(), 79);

  time_t t;
  struct tm structuredtime;
  time(&t);
  timespec microtime;
  clock_gettime(CLOCK_REALTIME, &microtime);
  timespec microdiff;
  mytimersub(&microtime, &m_startTimestamp, &microdiff);
  p->s_body.sortim    = microdiff.tv_sec;
  if (microdiff.tv_nsec > 500000000){
    p->s_body.sortim++;
  }


  localtime_r(&t, &structuredtime);
  p->s_body.tod.month = structuredtime.tm_mon;
  p->s_body.tod.day   = structuredtime.tm_mday;
  p->s_body.tod.year  = structuredtime.tm_year;
  p->s_body.tod.hours = structuredtime.tm_hour;
  p->s_body.tod.min   = structuredtime.tm_min;
  p->s_body.tod.sec   = structuredtime.tm_sec;
  p->s_body.tod.tenths= 0;	// Always 0 in unix.

}
/**
 * Sends a control monitoring event (one from stack 7) to the 
 * control server. These events contain periodic monitoring data.
 * It's up to the Tcl server to assemble them into a single 
 * event (if they are split across a buffer boundary).
 * 
 * @param pEvent - Pointer to the event fragment.
 */
void
COutputThread::sendToTclServer(uint16_t* pEvent)
{


  // Locate the Tcl Server and queue the event:

  TclServer* pServer = Globals::pTclServer;
  pServer->QueueBuffer(pEvent);

  
}
