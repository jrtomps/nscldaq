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

#ifndef __SCALERINPUTBUFFER_H
#define __SCALERINPUTBUFFER_H

#ifndef __INPUTBUFFER_H
#include <InputBuffer.h>
#ifndef __INPUTBUFFER_H
#define __INPUTBUFFER_H
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

/*!
 *   Encapsulates an input buffer that is a scaler
 * buffer we can't/don't need to make the distinction at this level
 * between a scaler or a snapshot scaler buffer.
 */
class ScalerInputBuffer : public InputBuffer
{
public:
	
	// Constructors and other canonicals.
	
	ScalerInputBuffer(uint16_t* pBuffer);
	ScalerInputBuffer(const ScalerInputBuffer& rhs);
	ScalerInputBuffer& operator=(const ScalerInputBuffer& rhs);
	int operator==(const ScalerInputBuffer& rhs)  const;
	int operator!=(const ScalerInputBuffer& rhs)  const;

	// Object methods:
	
public:
	uint32_t startTime() const;
	uint32_t endTime()   const;
	
	std::vector<uint32_t> scalers() const;
	
	virtual InputBufferIterator* begin() const;
};


/*!
 *   A ScalerInputBufferIterator is the actual iterator type
 * produced by a scaler buffer.  It understands that each scaler
 * buffer can only produce a single event fragment... the scalers
 * it's related buffer holds.
 */

class ScalerInputBufferIterator : public InputBufferitertor
{
private:
	ScalerInputBuffer& m_buffer;
	bool               m_haveFragment;
	
public:
	ScalerInputBufferIterator(const ScalerInputBuffer& myBuffer);
	ScalerInputBufferIterator(const ScalerInputBufferIterator& rhs);

	int operator==(const ScalerInputBufferIterator& rhs) const;
	int operator!=(const ScalerInputBufferIterator& rhs) const;

	// Since we can't re-init a reference, assignment is illegal:
private:
	ScalerInputBufferIterator& operator=(const ScalerInputBufferIterator& rhs);	
public:


	// virtual methods (pure in base class)

	virtual void Next();                   //!< Advance to next event fragment.
	virtual bool End();                    //!< True if there is no next fragment.
	virtual EventFragment*  operator*();   //!< Produce next fragment (NULL If none).

};

#endif
