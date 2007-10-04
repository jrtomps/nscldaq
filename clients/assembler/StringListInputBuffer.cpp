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
#include "StringListInputBuffer.h"
#include <StringListFragment.h>

//////////////////////////////////////////////////////////////////

// The canonicals are trivial and therefore don't have any commenting.
StringListInputBuffer::StringListInputBuffer(void* pBuffer) :
	InputBuffer(pBuffer)
{
}
StringListInputBuffer::StringListInputBuffer(const StringListInputBuffer& rhs) :
	InputBuffer(rhs)
{
	
}

StringListInputBuffer& 
StringListInputBuffer::operator=(const StringListInputBuffer& rhs)
{
	InputBuffer::operator=(rhs);
	return *this;
}
int 
StringListInputBuffer::operator==(const StringListInputBuffer& rhs) const
{
	return InputBuffer::operator==(rhs);
}
int 
StringListInputBuffer::operator!=(const StringListInputBuffer& rhs) const
{
	return !(*this == rhs);
}
///////////////////////////////////////////////////////////////////
/*!
 * \return InputBufferIterator*
 * \retval Actually a pointer to a StringListInputBufferIterator
 *         that can create the event fragments in the buffer.
 */
InputBufferIterator* 
StringListInputBuffer::begin()    const
{
	return new StringListInputBufferIterator(*this);
}
///////////////////////////////////////////////////////////////////
//////////// StringListInputBufferIterator implementation /////////
///////////////////////////////////////////////////////////////////

/*!
 *   Construct:
 */
StringListInputBufferIterator::StringListInputBufferIterator(const StringListInputBuffer& myBuffer) :
	m_buffer(myBuffer),
	m_haveFragment(true)
{
	
}
StringListInputBufferIterator::StringListInputBufferIterator(const StringListInputBufferIterator& rhs) :
	m_buffer(rhs.m_buffer),
	m_haveFragment(rhs.m_haveFragment)
{
}
int 
StringListInputBufferIterator::operator==(const StringListInputBufferIterator& rhs) const
{
	return (&m_buffer == &rhs.m_buffer)         &&
		   (m_haveFragment == rhs.m_haveFragment);
}
int
StringListInputBufferIterator::operator!=(const StringListInputBufferIterator& rhs) const
{
	return !(*this == rhs);
}
//////////////////////////////////////////////////////////////////////
/*!
 * Since the iterator can only produce a single fragment, this
 * just sets m_haveFragment to be false.
 */
void
StringListInputBufferIterator::Next()
{
	m_haveFragment = false;
}
/////////////////////////////////////////////////////////////////////
/*!
 *   \return bool
 *   \retval true  - operator* will return a non null event fragment.
 *   \retval false - operator* will return a null event fragment pointer.
 */
bool
StringListInputBufferIterator::End()
{
	return m_haveFragment;
}
/////////////////////////////////////////////////////////////////////
/*!
 * If there's an event fragment to return, constructs it and passes
 * it back. If not, returns null.
 */
EventFragment*
StringListInputBufferIterator::operator*()
{
	EventFragment* pResult(0);
	if (m_haveFragment) {
	  pResult  = new StringListFragment(m_buffer.Pointer());
	}
	return pResult;
}
