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

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";
/*!
  \file Reader.cpp
  Implements the functionality of the Reader class.  Reader objects
  provide the experiment specific part of the readout skeleton.
  See Reader.h for more iformation.
*/
#include <config.h>
#include "Reader.h"
#include "Trigger.h"
#include "Busy.h"
#include "ReadoutStateMachine.h"
#include <daqinterface.h>
#include <assert.h>
#include <skeleton.h>
#include <Iostream.h>
#include <string>
#include <buftypes.h>
#include <CVMEInterface.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

union longword {
  uint32_t   l;
  uint16_t   s[2];
};

/*!
   Construct a Reader object.  Note that tyipcally the Reader object is 
   subclassed to select the appropriate type of trigger or busy module.

*/
CReader::CReader(ReadoutStateMachine& rManager) :
  m_rManager(rManager),
  m_pBuffer(0),
  m_pRawBuffer(0),
  m_pRawCursor(0),
  m_nEvents(0),
  m_nWords(0),
  m_nBufferSize(daq_GetBufferSize()),
  m_pTrigger(0),
  m_pBusy(0)
{}

/*!
    Enable data taking.  This involves:
    - Clearing the user's event hardware.
    - Enabling the trigger.
    - Clearing the busy.

    It is assumed that:
    - The computer has busy set.
    - The m_pBusy is non null (assert guarded).
    - The m_pTrigger is non null (assert guarded).
*/
void
CReader::Enable()
{
  // Ensure we're properly assembled:

  assert(m_pTrigger);
  assert(m_pBusy);

  //  Initialize the user's code:

  ::initevt();
  ::clearevt();
  ::clrscl();

  // Enable the trigger:

  m_pTrigger->Initialize();
  m_pTrigger->Enable();

  // Clear the busy

  m_pBusy->Initialize();
  m_pBusy->Clear();


}

/*!
   Turns off data taking by disableing the trigger.  The busy is set as well.

*/
void
CReader::Disable()
{
  m_pBusy->Set();		// Set computer not accepting.
  m_pTrigger->Disable();	// Disable further event receipt.
}

/*!
   Manages the overall flow of checking for triggers and reading events.
   In order to amortize the call overhead and other checks that are performed
   over multiple potential trigger checks, nPasses through the trigger
   check and readout code will be performed.
   
   If within this function, the buffer fills, it will be flushed out to 
   the routing system.

   \param nPasses int [in] - Number of passes through the trigger check loop.
*/
void
CReader::ReadSomeEvents(unsigned int nPasses)
{
  bool jumbo = daq_isJumboBuffer();

  // The trigger and busy managers must have been installed:

  assert(m_pTrigger);
  assert(m_pBusy);


  // If necessary, allocate a new >empty<  buffer etc.
  //
  if(!m_pBuffer) {
    m_pBuffer     = m_rManager.GetBuffer(daq_GetBufferSize());
    m_BufferPtr  = m_rManager.GetBody(m_pBuffer);
    m_nEvents     = 0;
    m_nBufferSize = daq_GetBufferSize() - m_BufferPtr.GetIndex();
    m_pRawBuffer  = new unsigned short[daq_GetBufferSize()*2]; // Double sized buffer.
    m_pRawCursor  = m_pRawBuffer;
    m_nWords      = 0;
  }
  
  try {
    for (unsigned int i = 0; i < nPasses; i++) {
      unsigned int nEventSize;
      if(m_pTrigger->Check()) {	// Event fired.
	m_pTrigger->Clear();
	unsigned short*  hdr = m_pRawCursor;
	m_pRawCursor++;		// Reserve space for event size.
	if (jumbo) m_pRawCursor++;
	
	CVMEInterface::Lock();
	nEventSize = ::readevt(m_pRawCursor);
	CVMEInterface::Unlock();
	if(nEventSize > 0) {
	  if (jumbo) {
	    nEventSize += 2;
	    union longword lw;
	    lw.l = nEventSize;
	    hdr[0]   = lw.s[0];
	    hdr[1]   = lw.s[1];
	  }
	  else {
	    nEventSize += 1;
	    *hdr       = nEventSize; // Fill in the size header.
	  }
	  m_nWords  += nEventSize;   // Fill in the buffer index.
	  m_nEvents++;
	  m_pRawCursor += nEventSize;
	}
	else {			// Rejected (zero length) event.
	  m_pRawCursor  = hdr;	// Retract buffer ptr on rejected event.
	}
	//
	// Try to overlap buffer flush management behind inter-event time.

	::clearevt();
	m_pBusy->ModuleClear();
	m_pBusy->Clear();

	// If necessary, flush the buffer:

	if(m_nWords > m_nBufferSize) {
	  OverFlow(hdr);
	}
      }      	                 // Trigger present.
    }                            // Trigger check loop.
  }
  // The catch blocks below attempt to do our best to put out messages
  // for exceptions thrown by data taking.  The exceptions themselves are
  // propagated back up the call stack, but at least we'll give a message
  // before dying.

  catch(string& rsMessage) {
    cerr << __FILE__ << __LINE__ << 
           "A string exception was caught during readout: \n";
    cerr << rsMessage << endl;
    cerr << "Propagating exception back to caller\n";
    throw;

  }
  catch(char* pszMessage) {
    cerr << __FILE__ << __LINE__ << 
         "A C-string exception was caught during readout:\n";
    cerr << pszMessage << endl;
    cerr << "Propagating exception back to caller\n";
    throw;
  }
  catch(...) {
    cerr << __FILE__ << __LINE__ <<
            "An exception was caught during readout:\n";
    cerr << "Don't know how to convert this to an error message\n";
    cerr << "Propagating exception back to the caller\n";
    throw;
  }
}

/*!
   Flush an event buffer out to the routing system.  This involves:
   - Shrinking the buffer down to 4K words..
   - Filling in the buffer header. 
   - Routing the buffer.
   - Deleting the buffer.
   - Resetting the member variables to indicate that there is no current
     buffer.

  Note that the following are expected to be correct:
  - m_nEvents - Number of events (entities) in the shrunk buffer.
  - m_nWords  - Number of valid words in the shrunken buffer.

  They will be placed into the buffer header.
*/
void
CReader::FlushBuffer()
{
  if(!m_pBuffer) return;	// No buffer to flush.

  // Fill in the buffer heaer:

  m_rManager.NextSequence();	// Increment sequence
  m_rManager.FormatHeader(m_pBuffer, m_nWords, DATABF, m_nEvents);

  // Copyin data from the raw buffer into the spectrodaq buffer:

  m_BufferPtr.CopyIn(m_pRawBuffer, 0, m_nWords);

  // Route and delete the buffer.

  m_pBuffer->Route();
  delete m_pBuffer;

  // Reset the member variables to force a new allocation on the next call
  // to ReadSomeEvents:

  delete []m_pRawBuffer;
  m_pRawBuffer  = (unsigned short*)NULL;
  m_pRawCursor  = (unsigned short*)NULL;
  m_pBuffer     = (DAQWordBuffer*)NULL;
  m_nEvents     = 0;
  m_nWords      = 0;
  m_nBufferSize = 0;
}

/*!
   Handles buffer overflows into the safe zone.  All buffers are allocated
   to be twice the size of a daq buffer.  Therefore as long as the worst 
   case event is not larger than a buffer (required by the NSCL daq system), 
   event buffer overflows are allowed and expected.

   When a buffer overflow occurs:
   - A new buffer is allocated.
   - The event that caused the overflow is copied into the new buffer.
   - The old buffer is flushed.
   - Member data are modified so that the new buffer will have new data taken
     into it.
*/
void
CReader::OverFlow(unsigned short* rLastEventPtr)
{

  // Allocate a new raw buffer and copy the event that's hanging over the
  // end of the current raw buffer:

  unsigned short* pNextBuffer = new unsigned short[m_nBufferSize*2];
  bool            jumbo       =daq_isJumboBuffer();

  // Retract the last event from the buffer:

  unsigned int nWords;
  unsigned int nNewSize;

  if (jumbo) {
    union longword lw;
    lw.s[0]  = rLastEventPtr[0];
    lw.s[1]  = rLastEventPtr[1];

    nWords   = lw.l;
  }
  else {
    nWords   = *rLastEventPtr;
  }
  nNewSize   = nWords;


  m_nWords -= nWords;
  m_nEvents--;



  // Copy the last event in the old buffer (pointed to by rLastEventPtr)
  // into the new buffer.  The only assumption is that the first word of
  // the event is a self inclusive size.  Note that pre-increments are used
  // because in general they will be faster for objects since they avoid
  // copy construction.
  
  memcpy(pNextBuffer, rLastEventPtr, nWords*sizeof(unsigned short));

  // Flush the existing buffer:

  FlushBuffer();


  // Allocate a new DAQWordBuffer and so on, and reset the member data
  // to use all the new stuff.


  // The raw buffer:

  m_pRawBuffer = pNextBuffer;
  m_pRawCursor = pNextBuffer + *m_pRawBuffer;


  // The DAQWordBuffer:

  m_pBuffer     = m_rManager.GetBuffer(daq_GetBufferSize());
  m_BufferPtr   = m_rManager.GetBody(m_pBuffer);
  m_nEvents     = 1;		// The odd event.
  m_nWords       = *m_pRawBuffer;   // The size of the odd event.
  m_nBufferSize = daq_GetBufferSize() -
    m_BufferPtr.GetIndex();
      
}
