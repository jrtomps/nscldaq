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

#ifndef __STRINGLISTINPUTBUFFER_H
#define __STRINGLISTINPUTBUFFER_H

#ifndef

class StringListInputBuffer  : public InputBuffer
{
public:
	StringListInputBuffer(void* pBuffer);
	StringListInputBuffer(const StringListInputBuffer& rhs);
	StringListInputBuffer& operator=(const StringListInputBuffer& rhs);
	int operator==(const StringListInputBuffer& rhs) const;
	int operator!=(const StringListInputBuffer& rhs) const;
	
	// Member functions of the class:
	
	virtual InputBufferIterator* begin()    const;
};


/*!
 *   The StringListInputBufferIterator class provides a mechanism
 * to get the event fragments in an string list buffer. 
 * Buffers of this type are only supposed to have a single fragment...
 * one that consists of all the strings in the buffer body.
 */
class StringListInputBufferIterator : public InputBufferIterator 
{
private:
	const StringListInputBuffer&   m_buffer;
	bool                     m_haveFragment;
	

public:
	StringListInputBufferIterator(const StringListInputBuffer& myBuffer);
	StringListInputBufferIterator(const StringListInputBufferIterator& rhs);
	int operator==(const StringListInputBufferIterator& rhs) const;
	int operator!=(const StringListInputBufferIterator& rhs) const;

private:
	StringListInputBufferIterator& operator=(const StringListInputBufferIterator& rhs);
public:
	
	
	virtual void Next();   					//!< Advance to next event fragment.
	virtual bool End();   					//!< True if there is no next fragment.
	virtual EventFragment*  operator*();	//!< Produce next fragment (NULL If none).

};
#endif /*STRINGLISTINPUTBUFFER_H_*/
