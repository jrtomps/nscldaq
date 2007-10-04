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

#ifndef __INPUTBUFFER_H
#define __INPUTBUFFER_H


#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __CRT_STDLIB_H
#include <stdlib.h>
#ifndef __CRT_STDLIB_H
#define __CRT_STDLIB_H
#endif
#endif



class EventFragment;
class InputBufferIterator;
/*!
 *   This class is the base class functionality for an input buffer.
 *   Input buffers have the ability to fetch words from themselves in the
 *   host system byte ordering as well as to break themselves up into 
 *   fragments.  The fragment break up is separated from where the fragements
 *   go via the ability of each buffer type to produce an iterator that
 *   is descended from the InputBufferIterator abstract base class.
 *   \note
 *      The classes construct on a raw buffer and maintain a pointer to it to
 *    reduce copy overhead.  Therfore the buffer itself must survive for the lifetime
 *    of input buffer objects.
 */  
class InputBuffer {
private:
	uint16_t*   m_pBuffer;           // Copy-free buffer access.
	uint16_t    m_ssig;              // Cached short signature.
	uint32_t    m_lsig;              // cached long signature
	uint16_t    m_type;              // cached buffer type.
	
public:
	// Constructors and canonicals.
	
	InputBuffer(void* pBuffer);
	InputBuffer(const InputBuffer& rhs);
	InputBuffer& operator=(const InputBuffer& rhs);
	int operator==(const InputBuffer& rhs) const;
	int operator!=(const InputBuffer& rhs) const;
	
	// The selectors people care about;
	// These access fields in the header (cached or not).
	
public:
	int getType()                           const;
	int getSize()                           const;
	int getNode()                           const;
	int getEntityCount()                    const;
	bool isJumboBuffer()                    const;
	uint32_t getLongword(size_t wordOffset) const;
	uint16_t getWord(size_t wordOffset)     const;
	
	uint16_t*       Pointer()             const;
	const uint16_t* bodyPointer()          const;
	int   bodyOffset()                     const;

	
	virtual InputBufferIterator* begin()    const =0;


	
	// These are utilities that probably are note needed
	// by the derived classes but are provided 'just in case'.
protected:
	
	uint32_t tohl(uint32_t datum)          const;
	uint16_t tohs(uint16_t datum)          const;
	

};

/*!
 *   This class is an abstract base class that can traverse the contents
 *   of a data buffer, producing sequential event fragments.
 */
class InputBufferIterator {

public:
	virtual void Next()                 = 0;   //!< Advance to next event fragment.
	virtual bool End()                  = 0;   //!< True if there is no next fragment.
	virtual EventFragment*  operator*() = 0;   //!< Produce next fragment (NULL If none).
	
};

#endif
