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
#include <assert.h>
#include <buffer.h>
#include <buftypes.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


///////////////////////////////////////////////////////////////////////
///////////////////// Local constants. ////////////////////////////////
///////////////////////////////////////////////////////////////////////

// The VMUSB header:

static const int VMUSBLastBuffer(0x8000);
static const int VMUSBisScaler(0x4000);
static const int VMUSBContinuous(0x2000);
static const int VMUSBMultiBuffer(0x1000);
static const int VMUSBNEventMask(0x0fff);

///////////////////////////////////////////////////////////////////////
///////////////////// Local data types ////////////////////////////////
//////////////////////////////////////////////////////////////////////

typedef struct _BeginRunBuffer {
  BHEADER         s_header;
  struct ctlbody  s_body;
} BeginRunBuffer, *pBeginRunBuffer;

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
  m_outputBufferSize(0)		// Don't know yet.
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

  while(1) {

    DataBuffer& buffer(getBuffer());
    processBuffer(buffer);
    freeBuffer(buffer);

  }
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
  if (s_bufferType == 1) {
    startRun(buffer);
  } else {
    uint16_t header = buffer.s_rawData[0]; // VMUSB header.
    if (header & VMUSBLastBuffer) {
      endRun(buffer);
    }
    else if (header & VMUSBisScaler) {
      scaler(buffer);
    }
    else {
      events(buffer);
    }
  }
}
/*
   Process a begin run pseudo buffer. I call this a psuedo buffer because
   there will not be any data from the VM_usb for this
   We must:
   - Update our concept of the run state.
   - Set the m_outputBufferSize accordingly.
   - Create an NSCL begin run buffer and
   - send it to spectrodaq.
   - Destroy the NSCL begin run buffer we created.
   
   Parameters:
     DataBuffer&  buffer   - the buffer from the buffer queue.
*/
void
COutputThread::startRun(DataBuffer& buffer)
{
  // Update our concept of run state, and buffer size:

  CRunState* pState = getInstance();
  m_RunNumber       = pState->getRunNumber();
  m_title           = pState->getTitle();
  m_sequence        = 0;
  m_outputBufferSize= buffer.s_bufferSize;

  // Allocate the Begin run buffer and fill it in.

  pBeginRunBuffer p = static_cast<pBeginRunBuffer>(malloc(sizeof(BeginRunBuffer)));

  // Header:
  
  p->s_header.nwds  = sizeof(BeginRunBuffer/sizeof(uint16_t));
  p->s_header.type  = BEGRUNBF;
  p->s_header.cks   = 0;	// Not using the checksum!!!
  p->s_header.run   = m_runNuber;
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
  p->s_body.sortim    = 0;

  time_t t;
  struct tm structuredtime;
  time(&t);
  localtime_r(&t, &structuredtime);
  p->s_body.tod.month = structuredtime.tm_mon;
  p->s_body.tod.day   = structuredtime.tm_mday;
  p->s_body.tod.year  = structuredtime.tm_year;
  p->s_body.tod.hours = structuredtime.tm_hour;
  p->s_body.tod.min   = structuredtime.tm_min;
  p->s_body.tod.sec   = structuredtime.tm_sec;
  p->s_body.tod.tenths= 0;	// Always 0 in unix.
  

  // Submit the buffer to spectrodaq as tag 2 and free it.

  bufferToSpectrodaq(p, 2, m_outputBufferSize, 
		     sizeof(BeginRunBuffer/sizeof(uint16_t)));
  free(p);
}
