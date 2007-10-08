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
#ifndef __PHYSICSINPUTBUFFER_H
#define __PHYSICSINPUTBUFFER_H

#ifndef __INPUTBUFFER_H
#include "InputBuffer.h"

/*!
 *   Encapsulates a physics input buffer so that
 *   the correct type of iterator can be created 
 */

class PhysicsInputBuffer : public InputBuffer;
{
public:
	PhysicsInputBuffer(void* pBuffer);
	PhysicsInputBuffer(const PhysicsInputBuffer& rhs);
	PhysicInputBuffer& operator=(const PhysicsInputBuffer& rhs);
	int operator==(const PhysicsInputBuffer& rhs) const;
	int operator!=(const PhysicsInputBuffer& rhs) const;
	
	
	// Create the iterator:
	
	virtual InputBufferIterator* begin() const;
	
};

/*!
 * Iterates through a physics input buffer, creating
 * the event fragments it contains.
 */
class PhysicInputBufferIterator : public InputBufferIterator
{
private:
	PhysicsInputBuffer&  m_Buffer;
	size_t               m_remaining;     // Events remaining.
	off_t                m_currentOffset; // Offset of current event.
	
public:
	PhysicsInputBufferIterator(PhysicsInputBuffer* pBuffer);
	PhysicsInputBufferIterator(const PhysicsInputBufferIterator& rhs);
	int operator==(const PhysicsInputBufferIterator& rhs) const;
	int operator!=(const PhysicsInputBufferIterator& rhs) const;
private:
	PhysicsInputBufferIterator& operator=(const PhysicsInputBufferIterator& rhs);
public:
	
	virtual void Next();
	virtual bool End();
	virtual EventFragment* operator*();
	
private:
	size_t eventSize(); 
};

#endif /*PHYSICSINPUTBUFFER_H_*/
