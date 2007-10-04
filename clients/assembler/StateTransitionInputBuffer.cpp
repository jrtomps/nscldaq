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
#include "StateTransitionInputBuffer.h"
#include "StateTransitionFragment.h"

// Once more the canonicals are trivial.

StateTransitionInputBuffer::StateTransitionInputBuffer(void* pBuffer) :
	InputBuffer(pBuffer)
{
}
StateTransitionInputBuffer::StateTransitionInputBuffer(const StateTransitionInputBuffer& rhs) :
	InputBuffer(rhs)
{

}

const StateTransitionInputBuffer& operator=(const StateTransitionInputBuffer& rhs) 
{
	InputBuffer::operator=(rhs);
	
	return *this;
}

int 
StateTransitionInputBuffer::operator==(const StateTransitionInputBuffer& rhs) const
{
	return InputBuffer::operator==(rhs);
}
int 
StateTransitionBuffer::operator!=(const StateTransitionInputBuffer& rhs) const
{
	return !(*this == rhs);
}

///////////////////////////////////////////////////////////////////////
/*!
 *   Produce the iterator:
 */
InputBufferIterator*
begin() const
{
	return StateTransitionInputBufferIterator(*this);
}
////////////////////////////////////////////////////////////////////
///////// StateTransitionBufferIterator implementation /////////////
////////////////////////////////////////////////////////////////////

// The canonicals are not tough at all:

StateTransitionInputBufferIterator::StateTransitionInputBufferIterator(const StateTransitionInputBuffer& buf) :
	m_Buffer(buf),
	m_haveFragment(true)
{
}
StateTransitionInputIterator::StateTransitionInputBufferIterator(const StateTransitionInputBufferIterator& rhs) :
	m_Buffer(rhs.m_Buffer),
	m_haveFragment(rhs.m_haveFragment)
{

}
int 
StateTransitionInputIterator::operator==(const StateTranstionInputBufferIterator& rhs) const
{
	return (&m_Buffer  == &(rhs.m_Buffer)          &&
			(m_haveFragment == rhs.m_haveFragment);
}

int operator!=(const StateTransitionInputBufferIterator& rhs) const
{
	return !(*this == rhs)
}
/////////////////////////////////////////////////////////////////////
/*!   
 * On to next fragment (just marks no more fragments.
 */
void
StateTransitionInputBufferIterator::Next()
{
	m_haveFragment = false;
}
/////////////////////////////////////////////////////////////////////
/*!
 *   Report if the operator* will return a null fragment.
 */
bool
StateTransitionInputBufferIterator::End() 
{
	return m_haveFragment;
}
///////////////////////////////////////////////////////////////////////
/*!
 * return the next event fragment or null if there isn't one.\
 */
EventFragment*
StateTransitionInputBufferIterator::operator*()
{
	EventFragment* pResult(0);
	
	if (m_haveFragment) {
		pResult = new StateTransitionFragment(m_Buffer.Pointer());
	}
	return m_haveFragment
	}
}