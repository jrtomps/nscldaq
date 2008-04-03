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

static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";
//////////////////////////CNSCLPhysicsBuffer.cpp file////////////////////////////////////
#include <config.h>
#include "CNSCLPhysicsBuffer.h"                  
#include <buffer.h>
#include "buftypes.h"
#include <RangeError.h>
#include <unistd.h>

extern bool daq_isJumboBuffer();

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
/*!
   Default constructor.  This is called when declarations of the form e.g.:
   -  CNSCLPhysicsBuffer  object;
   are performed.
*/
CNSCLPhysicsBuffer::CNSCLPhysicsBuffer(unsigned nWords)
#ifndef HIGH_PERFORMANCE
   : CNSCLOutputBuffer(nWords)
#else /* HIGH_PERFORMANCE */
  : CNSCLOutputBuffer(nWords),
    m_pBuffer(new unsigned short[nWords]),
    m_nEntityCount(0),
    m_nBufferSize(nWords)
#endif /* HIGH_PERFORMANCE */
{
#ifdef HIGH_PERFORMANCE
  m_pBufferCursor = m_pBuffer;

#endif /* HIGH_PERFORMANCE */
  SetType(DATABF);
  getBuffer().SetTag(CNSCLOutputBuffer::m_EventTag);
} 

#ifdef HIGH_PERFORMANCE
//! Destructor:

CNSCLPhysicsBuffer::~CNSCLPhysicsBuffer()
{
  delete []m_pBuffer;
}
#endif /* HIGH_PERFORMANCE */

/*!
    Creates a buffer pointer,  reserves space
    for a word count and returns it to the caller.
    

*/
#ifndef HIGH_PERFORMANCE
DAQWordBufferPtr 
#else /* HIGH_PERFORMANCE */
unsigned short*
#endif /* HIGH_PERFORMANCE */
CNSCLPhysicsBuffer::StartEvent()  
{
  bool jumbo = daq_isJumboBuffer();
#ifndef HIGH_PERFORMANCE
  m_EventStartPtr = StartEntity(); // Start the buffer entity.
  DAQWordBufferPtr p = m_EventStartPtr;
  ++p;
  if (jumbo) ++p;
  return p;
#else /* HIGH_PERFORMANCE */
  return jumbo ? (m_pBufferCursor + 2) : 
    (m_pBufferCursor + 1);	// Hold space for the word count long or short as case may be.
#endif /* HIGH_PERFORMANCE */
  
}  

/*!
    Determines the word count from the
    difference between the input pointer and
    m_EventStartPtr.  This word count is
    inserted in the buffer at *m_EventStartPtr
    and m_EventStartPtr is set to be the same as
    the input pointer.  The entity count is incremented.
 

*/
void 
#ifndef HIGH_PERFORMANCE
CNSCLPhysicsBuffer::EndEvent(DAQWordBufferPtr& rPtr)  
#else /* HIGH_PERFORMANCE */
CNSCLPhysicsBuffer::EndEvent(unsigned short* pEnd)
#endif /* HIGH_PERFORMANCE */
{

  bool jumbo = daq_isJumboBuffer();
  union {
    unsigned int l;
    unsigned short w[2];
  } lw;


#ifndef HIGH_PERFORMANCE
  if(rPtr.GetIndex() >= getWords()) {
    throw CRangeError(0, getWords(), rPtr.GetIndex(),
	     "CNSCLPhysicsBuffer::EndEvent - Off the end of the buffer");
#else /* HIGH_PERFORMANCE */

  if ((pEnd - m_pBuffer) > m_nBufferSize) {
    throw CRangeError(0, m_nBufferSize,
		      (pEnd - m_pBuffer),
		      "CNSCLPhysicsBuffer::EndEvent - off the end of the buffer");
#endif /* HIGH_PERFORMANCE */
  }
#ifndef HIGH_PERFORMANCE
  unsigned nSize = rPtr.GetIndex() - m_EventStartPtr.GetIndex();
  if (jumbo) {
    lw.l = nSize;
    *m_EventStartPtr = lw.w[0];
    m_EventStartPtr[1] = lw.w[1];
  }
  else {
    *m_EventStartPtr = nSize;
  }
#else /* HIGH_PERFORMANCE */
  unsigned nSize = (pEnd - m_pBufferCursor); // Event size.
  if (jumbo) {
    lw.l = nSize;
    *m_pBufferCursor = lw.w[0];
    m_pBufferCursor[1] = lw.w[1];
  }
  else {
    *m_pBufferCursor = nSize;
  }
  m_pBufferCursor  = pEnd;
  m_nEntityCount++;
#endif /* HIGH_PERFORMANCE */

#ifndef HIGH_PERFORMANCE
  EndEntity(rPtr);		// Add to entity count and update ptr.
#endif /* ! HIGH_PERFORMANCE */

}  

/*!
    Take the necessary steps to ensure that
    data put in the buffer for this event does not
    get committed to the buffer.  At present this is
    a no-op. Since neiter the base class's buffer pointer
    nor the entity count get modified until EndEvent is called.

	\param DAQWordBufferPtr& p

	\bug There is unfortunately no known way to invalidate
	      m_EventStartPtr else we would at this point.

*/
void 
#ifndef HIGH_PERFORMANCE
CNSCLPhysicsBuffer::RetractEvent(DAQWordBufferPtr& p)  
#else /* HIGH_PERFORMANCE */
CNSCLPhysicsBuffer::RetractEvent(unsigned short* p)  
#endif /* HIGH_PERFORMANCE */
{
 
}
#ifdef HIGH_PERFORMANCE
/*!
   Route the event.  This involves doing a copyin to the 
   base class buffer and asking it to route:

*/
void
CNSCLPhysicsBuffer::Route()
{
  setEntityCount(m_nEntityCount);
  PutWords(m_pBuffer, m_pBufferCursor - m_pBuffer);
  CNSCLOutputBuffer::Route();

}
#endif /* HIGH_PERFORMANCE */
/*!
   Return the number of words in the buffer at this point
   (where the cursor is).
*/
int
CNSCLPhysicsBuffer::WordsInBody() const
{
#ifdef HIGH_PERFORMANCE
  return (m_pBufferCursor - m_pBuffer);
#else
  return m_BufferPtr.GetIndex() - sizeof(struct bheader)/sizeof(unsigned short);
#endif
}
