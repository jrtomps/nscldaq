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
#include "ScalerInputBuffer.h"
#include "ScalerFragment.h"

using namespace std;
// Format of scaler buffer body:
// Buffer word offset 	words 	contents
//  17 	                   2 	Interval end time in seconds
//  19                 	   3 	Unused words
//  22 	                   2 	Interval start time in seconds
//  24 	                   3 	Unused words
//  27 	                   varies 	longword scaler values

// The constants below describe this:

static const size_t EndTime(17);
static const size_t StartTime(22);
static const size_t Scalers(27);

////////////////////////////////////////////////////
// All of the canonical operations can be done by
// the base class methods since we have no additiona.
// internal state.

ScalerInputBuffer::ScalerInputBuffer(void* pBuffer) : 
	InputBuffer(pBuffer)
{
	
}
ScalerInputBuffer::ScalerInputBuffer(const ScalerInputBuffer& rhs) :
	InputBuffer(rhs)
{
}
ScalerInputBuffer&
ScalerInputBuffer::operator=(const ScalerInputBuffer& rhs)
{
	InputBuffer::operator=(rhs);
	return *this;
}
int
ScalerInputBuffer::operator==(const ScalerInputBuffer& rhs)  const
{
	return InputBuffer::operator==(rhs);
}
int 
ScalerInputBuffer::operator!=(const ScalerInputBuffer& rhs)  const
{
	return InputBuffer::operator!=(rhs);
}
/////////////////////////////////////////////////////////////////
/*!
 * \return uint32_t
 * \retval The interval start time.
 * 
 */
uint32_t 
ScalerInputBuffer::startTime() const
{
	return getLongword(StartTime);
}
////////////////////////////////////////////////////////////////
/*!
 *   \return uint32_t
 *   \retval The scaler interval end time.
 */
uint32_t 
ScalerInputBuffer::endTime()   const
{
	return getLongword(EndTime);
}
//////////////////////////////////////////////////////////////////
/*!
 * \return std::vector<uint32_t>
 * \retval the vector of scalers.
 * \note the buffer entity count tells us how many scalers are present.
 */

vector<uint32_t> 
ScalerInputBuffer::scalers() const
{
	size_t offset      = Scalers;
	int    scalerCount = getEntityCount();
	vector<uint32_t>     scalerValues;
	
	for (int i = 0; i < scalerCount; i++) {
		scalerValues.push_back(getLongword(offset));
		offset    += sizeof(uint32_t)/sizeof(uint16_t);
	}
	return scalerValues;
	
}
//////////////////////////////////////////////////////////////////
/*!
 * \return InputBufferIterator*
 * \retval A scaler buffer iterator for this buffer, initialized so 
 *         that it can return the event fragment for this buffer
 *         and only one fragment.
 * \note  The iterator is dynamically allocated and must be deleted by the 
 *        caller at some point to prevent memory leaks.
 */
InputBufferIterator* 
ScalerInputBuffer::begin() const
{
	return new ScalerInputBufferIterator(*this);
}
//////////////////////////////////////////////////////////////////
//////////// ScalerInputBufferIterator implementation ////////////
//////////////////////////////////////////////////////////////////

/*
 * Constructors and canonicals are trivial.
 */
ScalerInputBufferIterator::ScalerInputBufferIterator(const ScalerInputBuffer& myBuffer) :
	m_buffer(myBuffer),
	m_haveFragment(true)
{
}
ScalerInputBufferIterator::ScalerInputBufferIterator(const ScalerInputBufferIterator& rhs) :
	m_buffer(rhs.m_buffer),
	m_haveFragment(rhs.m_haveFragment)
{
	
}
int
ScalerInputBufferIterator::operator==(const ScalerInputBufferIterator& rhs) const
{
	return (&m_buffer == &rhs.m_buffer) &&
	       (m_haveFragment == rhs.m_haveFragment);
}
int 
ScalerInputBufferIterator::operator!=(const ScalerInputBufferIterator& rhs) const
{
	return !(*this == rhs);
}
/////////////////////////////////////////////////////////////////////
/*!
 *  Advance to the next fragment.  note that there is no second fragment
 */
void 
ScalerInputBufferIterator::Next()
{
	m_haveFragment = false;
}
/////////////////////////////////////////////////////////////////////
/*!
 *   \return bool
 *   \retval true - If Next() was never called.
 *   \retval false - If Next() was called at least once.
 */
bool
ScalerInputBufferIterator::End()
{
	return !m_haveFragment;
}
//////////////////////////////////////////////////////////////////////
/*!
 *  Interact with our buffer to return a scaler event fragment if one
 *  exists.. the event fragment will be the host byte order. 
 * 
 * \return EventFragment*  (really pointing to a ScalerFragment)
 * \retval NULL   - We have no more event fragments to give.
 * \retval other  - Pointer to a dynamically allocated event fragment
 *                  that is a ScalerFragment for our buffer
 * \note The event fragment is dynamically allocated and must be deleted
 *       by the caller.
 */
EventFragment*  
ScalerInputBufferIterator::operator*()
{
	EventFragment* pReturnValue(0);
	if(m_haveFragment) {
		// Since we want it in our byte order, we take
		// the stupid approach of creating a local copy of the
		// body in our byte order and constructing the scaler event
		// around that.  Since scalers are infrequent this is probably ok.
		int totalSize =  m_buffer.getSize();
		int bodyOffset=  m_buffer.bodyOffset();
		uint16_t type =  m_buffer.getType();
		uint16_t node =  m_buffer.getNode();
		uint16_t bodyLongs =  (totalSize - bodyOffset)* sizeof(uint16_t)/sizeof(uint32_t); 
		
		uint32_t* bodyCopy = new uint32_t[bodyLongs];
		
		for (int i=0; i < bodyLongs; i++) {
			bodyCopy[i] = m_buffer.getLongword(bodyOffset + 
							   i*sizeof(uint32_t)/sizeof(uint16_t));
		}
		pReturnValue = new ScalerFragment(type, node, 
						  reinterpret_cast<uint16_t*>(bodyCopy), 
						  bodyLongs * sizeof(uint32_t)/sizeof(uint16_t));
		delete []bodyCopy;
	}
	return pReturnValue;
}
