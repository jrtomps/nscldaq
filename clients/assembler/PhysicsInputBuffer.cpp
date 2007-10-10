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

#include "PhysicsInputBuffer.h"
#include "EventFragment.h"
#include "PhysicsFragment.h"
#include "EventTooSmallException.h"
#include <string>

using namespace std;
/*
 *  The canonicals are quite trivial.
 */
PhysicsInputBuffer::PhysicsInputBuffer(void* pBuffer) :
	InputBuffer(pBuffer)
{
}
PhysicsInputBuffer::PhysicsInputBuffer(const PhysicsInputBuffer&rhs) :
	InputBuffer(rhs)
{

}
PhysicsInputBuffer&
PhysicsInputBuffer::operator=(const PhysicsInputBuffer& rhs)
{
	InputBuffer::operator=(rhs);
	return *this;
}
int 
PhysicsInputBuffer::operator==(const PhysicsInputBuffer& rhs) const
{
	return InputBuffer::operator==(rhs);
}
int
PhysicsInputBuffer::operator!=(const PhysicsInputBuffer& rhs) const
{
		return !(*this == rhs);
}

/*!
    Create the iterator for this buffer:
*/
InputBufferIterator*
PhysicsInputBuffer::begin() const
{
  return new PhysicsInputBufferIterator(const_cast<PhysicsInputBuffer*>(this));
}
//////////////////////////////////////////////////////////////////////

/*!
 * Constructs an iterator for physics buffers.  In addition to 
 * retaining a reference to the buffer we'll set up the offset
 * and event count from the base buffer.
 */
PhysicsInputBufferIterator::PhysicsInputBufferIterator(PhysicsInputBuffer* pBuffer) :
	m_Buffer(*pBuffer),
	m_remaining(pBuffer->getEntityCount()),
	m_currentOffset(pBuffer->bodyPointer() - pBuffer->Pointer())
	{
	
	}
/*!
 *   Construct a copy of the rhs.
 */
PhysicsInputBufferIterator::PhysicsInputBufferIterator(const PhysicsInputBufferIterator& rhs) :
	m_Buffer(rhs.m_Buffer),
	m_remaining(rhs.m_remaining),
	m_currentOffset(rhs.m_currentOffset)
	{
	
	}
/*!
 * Equality implies the same buffer position in the same buffer:
 * Note that if the underlying buffers are the same, and the current
 * offset is the same, the number of remining events must be the same.
 */
int
PhysicsInputBufferIterator::operator==(const PhysicsInputBufferIterator& rhs) const
{
	return (&m_Buffer == &rhs.m_Buffer)  &&
			(m_currentOffset == rhs.m_currentOffset);
}
int
PhysicsInputBufferIterator::operator!=(const PhysicsInputBufferIterator& rhs) const
{
	return !(*this == rhs);
}
/////////////////////////////////////////////////////////////////////
/*!
 * iterate  to the next event.
 */
void
PhysicsInputBufferIterator::Next()
{
	// This is a no-op if we are already at the end:
	
	if(m_remaining) {
		size_t size = eventSize();
		m_remaining--;
		m_currentOffset += size;
	}
}
/////////////////////////////////////////////////////////////////////
/*!
 * \return bool
 * \retval true  - No more events.
 * \retval false - More events.
 */
bool
PhysicsInputBufferIterator::End()
{
	return (m_remaining == 0);
}
////////////////////////////////////////////////////////////////////
/*!
 * \return EventFragment*
 * \retval NULL  - No more fragments.
 * \retval other - Pointer to a dynamically allocated event fragment
 * \note It is up to the caller to free the event fragment.
 * \note We assume the structure of the event is:
 *        event size (16 or 32 bits)
 *        timestamp  (32 bits).
 */
EventFragment*
PhysicsInputBufferIterator::operator*()
{
  EventFragment* pResult(0);
  if(m_remaining) {
    size_t   size = eventSize();
    uint32_t timestamp;
    size_t   tsOffset;
    if (m_Buffer.isJumboBuffer()) {
      tsOffset = 2;
    } else {
      tsOffset =  1;
    }
    if (tsOffset >= size) {
      throw EventTooSmallException(size,
				   tsOffset,
				   string("PhysicsInputBufferIterator::operator()"));
    }
    timestamp = m_Buffer.getLongword(m_currentOffset + tsOffset);
    pResult = new PhysicsFragment(m_Buffer.getNode(),
				  m_Buffer.Pointer()  + m_currentOffset + tsOffset,
				  size - tsOffset,
				  timestamp);
    
  }
  return pResult;
}
/////////////////////////////////////////////////////////////////
/*
 * Return the size of the event at the current offset...taking into account
 * the jumbo state.
 */
size_t 
PhysicsInputBufferIterator::eventSize()
{
  if (m_Buffer.isJumboBuffer()) {
    return m_Buffer.getLongword(m_currentOffset);
  }
  else {
    return m_Buffer.getWord(m_currentOffset);
  }
}
