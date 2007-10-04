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

#ifndef __STATETRANSITIONINPUTBUFFER_H
#define __STATETRANSITIONINPUTBUFFER_H

#ifndef __INPUTBUFFER_H
#include <InputBuffer.h>
#endif

/*!
 * Encapsulate a class for state transition buffers on the
 * input stream.   The main purpose of the class is to produce
 * an iterator that can contribute the event fragment(s) this
 * buffer can produce.
 */

class StateTransitionInputBuffer : public InputBuffer
{
public:
	// canonicals.
	StateTransitionInputBuffer(void* pBuffer);
	StateTransitionInputBuffer(const StateTransitionInputBuffer& rhs);
	const StateTransitionInputBuffer& operator=(const StateTransitionInputBuffer& rhs);
	int operator==(const StateTransitionInputBuffer& rhs) const;
	int operator!=(const StateTransitionInputBuffer& rhs) const;
	
	
	virtual InputBufferIterator* begin()    const;
	
};


class StateTransitionInputBufferIterator : public InputBufferIterator
{
	const StateTransitionInputBuffer&	m_Buffer;
	bool  m_haveFragment;
public:
	StateTransitionInputBufferIterator(const StateTransitionInputBuffer& buf);
	StateTransitionInputBufferIterator(const StateTransitionInputBufferIterator& rhs);
	int operator==(const StateTransitionInputBufferIterator& rhs) const;
	int operator!=(const StateTransitionInputBufferIterator& rhs) const;
private:
	StateTransitionInputBufferIterator& operator=(const StateTransitionInputBufferIterator& rhs);
public:
	virtual void Next();	//!< Advance to next event fragment.
	virtual bool End();	//!< True if there is no next fragment.
	virtual EventFragment*  operator*(); //!< Produce next fragment (NULL If none).
	
};



#endif /*STATETRANSITIONINPUTBUFFER_H_*/
