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
#include "InputBuffer.h"
#include <buffer.h>


/////////////////////////////////////////////////////////////////////
/*!
 *    Create an input buffer base class.  In addition to storing
 * a pointer to the raw buffer, the signatures and buffer types
 * are pulled out and cached.  All other items will need to be 
 * converted from the underlying buffer as needed.
 * 
 * \param pBuffer - Pointer to the raw data on which this object will
 *                  constructed.  Note that only the pointer is saved 
 *                  so the lifetime of the underlying buffer must be at
 *                  least as long as that of this object.
 */
InputBuffer::InputBuffer(void* pBuffer) :
	m_pBuffer(reinterpret_cast<uint16_t*>(pBuffer))
{
	// Extract the cached items from the buffer:
	// in the case of the signatures no conversion is needed.  The type
	// is converted to the local representation:
	
	BHEADER* pHeader(reinterpret_cast<BHEADER*>(m_pBuffer));
	m_lsig = pHeader->lsignature;
	m_ssig = pHeader->ssignature;
	
	m_type = tohs(pHeader->type);
}
//////////////////////////////////////////////////////////////////////////
/*!
 *   Copy construction is just member by member copy.  
 * Coding practices at the NSCL require explicit copy constructors rather
 * than allowing the default copy constructor to do the work...so that
 * there's no lack of clarity about what's intended.  Copy constructors are
 * either explicit or disallowed.
 * 
 * \param rhs   - Reference to the object that will be copied to construct us.
 * 
 * \note Remember that the underlying buffer (now referenced by two objects)
 *       must be alive for the lifetime of both objects.
 */
InputBuffer::InputBuffer(const InputBuffer& rhs) :
	m_pBuffer(rhs.m_pBuffer),
	m_ssig(rhs.m_ssig),
	m_lsig(rhs.m_lsig),
	m_type(rhs.m_type)
{
	
}
////////////////////////////////////////////////////////////////////////////
/*!
 *  Assignment is just member by member.  There's no need to protect
 * against self assignment.
 *  
 * \param rhs  - Reference to the object that will be assigned to us.
 * 
 * \return InputBuffer&
 * \retval *this
 * 
 * \note Remember that the underlying buffer (now referenced by two objects)
 *       must be alive for the lifetime of both objects.
 */
InputBuffer& 
InputBuffer::operator=(const InputBuffer& rhs)
{
	m_pBuffer   = rhs.m_pBuffer;
	m_ssig      = rhs.m_ssig;
	m_lsig      = rhs.m_lsig;
	m_type      = rhs.m_type;
	
	return *this;
}
////////////////////////////////////////////////////////////////////////////
/*!
 *   Equality comparison.  If the underlying buffer of both objects
 * is the same, equality holds since all the other member data are just
 * cached from that buffer.
 * 
 * \param rhs  - The object *this is being compared to.
 * 
 * \return int
 * \retval FALSE - Objects are not equal
 * \retval TRUE  - Objects are equal.
 */
int 
InputBuffer::operator==(const InputBuffer& rhs) const
{
	return (m_pBuffer == rhs.m_pBuffer);
}
///////////////////////////////////////////////////////////////////////////
/*!
 *   Under NSCL coding standards, != is always defined as the boolean
 *   negation of operator==.  This makes intuitive sense and factors
 *   all the comparison code into one location.
 * \param rhs  - The object *this is being compared to.
 * 
 * \return int
 * \retval TRUE   - Objects are not equal
 * \retval FALSE- Objects are equal.
 */
int 
InputBuffer::operator!=(const InputBuffer& rhs) const
{
	return !(*this == rhs);
}
////////////////////////////////////////////////////////////////////////////
/*!
 *  
 * \return int
 * \retval The type of buffer held by this object.
 */
int 
InputBuffer::getType() const
{
	return m_type;
}
///////////////////////////////////////////////////////////////////////////////
/*!
 * \return int
 * \retval Size of the buffer in words.  
 * \note This member is smart enough to know about jumbo buffers.
 */
int 
InputBuffer::getSize() const
{
	BHEADER* pHeader(reinterpret_cast<BHEADER*>(m_pBuffer));
	int      nWords  = tohs(pHeader->nwds);
	
	// If this is a jumbo buffer, we need to fetch the high 16 bits...
	

	if (isJumboBuffer()) {
		int nWordsHigh = tohs(pHeader->nwdsHigh);
		nWords        |= nWordsHigh << 16;
	}
	return nWords;
}
//////////////////////////////////////////////////////////////////////////////
/*!
 * \return int
 * \retval the node number of the CPU that produced this buffer.
 */
int 
InputBuffer::getNode() const
{
	BHEADER* pHeader(reinterpret_cast<BHEADER*>(m_pBuffer));
	return tohs(pHeader->cpu);
}
/////////////////////////////////////////////////////////////////////////////
/*!
 * \return int
 * \retval The number of entities in the buffer.  The meaning of this
 *         depends on the type of the buffer.
 */
int 
InputBuffer::getEntityCount() const
{
	BHEADER* pHeader(reinterpret_cast<BHEADER*>(m_pBuffer));
	
	return tohs(pHeader->nevt);
}
////////////////////////////////////////////////////////////////////////////
/*!
 *  \return bool
 *  \retval True if the buffer revision level implies this is a jumbo buffer.
 */
bool 
InputBuffer::isJumboBuffer() const
{
	BHEADER* pHeader(reinterpret_cast<BHEADER*>(m_pBuffer));
	uint16_t revlevel = tohs(pHeader->buffmt);
	return (revlevel >= JumboBufferRevLevel);
}
//////////////////////////////////////////////////////////////////////////
/*!
 * Get a longword(32 bits) from a specific offset in the buffer.
 * 
 * \param wordOffset  - The offset in words from the start of the buffer
 * \return uint32_t
 * \note  In this implementation no range checking is performed.
 * 
 * TODO: Throw an exception  if the offset is past the used buffer size.
 */
uint32_t 
InputBuffer::getLongword(size_t wordOffset) const
{
	return tohl(m_pBuffer[wordOffset]);
}
/////////////////////////////////////////////////////////////////////////
/*!
 * Get a word (16 bits) from a specified offset in the buffer.
 * \param wordOffset - The offset in words from the start of the buffer.
 * \return uint16_t
 * \note  NO range checking is performed.
 * 
 * TODO: Throw an exception  if the offset is past the used buffer size.
 */
uint16_t 
InputBuffer::getWord(size_t wordOffset) const
{
	return tohs(m_pBuffer[wordOffset]);
}

///////////////////////////////////////////////////////////////////////
/////////////////////// Protected members /////////////////////////////
///////////////////////////////////////////////////////////////////////

/*
 *  Return a pointer to the buffer.
 */
uint16_t*
InputBuffer::Pointer() const
{
	return m_pBuffer;
}

/*
 *  Return a pointer to the body of the buffer.
 */
const uint16_t* 
InputBuffer::bodyPointer() const
{
	const BHEADER* pHeader(reinterpret_cast<BHEADER*>(m_pBuffer));
	return reinterpret_cast<const uint16_t*>(&(pHeader[1])); 
}
/*
 * Return the offset in words to the body of the buffer (more practically
 * useful
 */
int 
InputBuffer::bodyOffset() const
{
	return sizeof(BHEADER)/sizeof(uint16_t);
}
/*
 *   Convert a longword in buffer byte order to
 *   host byte order.
 */
uint32_t 
InputBuffer::tohl(uint32_t datum) const
{
	if (m_lsig == 0x01020304) return datum;   // Our byte order.
	
	  union Long {
	    uint32_t l;
	    uint8_t  b[4];
	  } source, dest;

	  source.l = datum;
	  dest.b[0] = source.b[3];
	  dest.b[1] = source.b[2];

	  dest.b[2] = source.b[1];
	  dest.b[3] = source.b[0];

	  return dest.l;	
}
/*
 * Convert a short in buffer byte order to host byte order.
 */
uint16_t
InputBuffer::tohs(uint16_t datum) const
{
	if (m_ssig == 0x0102) return datum; // Our byte order.
	
	  union Word {
	    uint16_t s;
	    uint8_t  b[2];
	  } source, dest;

	  source.s = datum;
	  dest.b[0]= source.b[1];
	  dest.b[1]= source.b[0];

	  return dest.s;
}

