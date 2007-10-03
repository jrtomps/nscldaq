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
#include "EventFragment.h"
#include <buffer.h>
#include <buftypes.h>

using namespace std;

/////////////////////////////////////////////////////////////////////
/*!
    Construct an event fragment given a body as a pointer to stuff.
    \param node   - originating node.
    \param type   - type of event.
    \param body   - Pointer to the body.
    \param words  - Count of words in the body.
*/
EventFragment::EventFragment(uint16_t node,
			      uint16_t type,
			      const void*    body,
			      size_t   words) :
  m_node(node),
  m_type(type)
{
  const uint16_t* pBegin= static_cast<const uint16_t*>(body);
  const uint16_t* pEnd  = pBegin + words;

  m_body.insert(m_body.begin(),
		pBegin, pEnd);
}
////////////////////////////////////////////////////////////////////
/*!
   Construct an event fragment given a body that is itself a vector

    \param node   - originating node.
    \param type   - type of event.
    \param body   - vector that is the body.
*/
EventFragment::EventFragment(uint16_t node,
			     uint16_t type,
			     std::vector<uint16_t> body) :
  m_node(node),
  m_type(type),
  m_body(body)
{
}

//////////////////////////////////////////////////////////////////
/*!
   Return the node from which this event originated:
   
*/
uint16_t
EventFragment::node() const
{
  return m_node;
}
//////////////////////////////////////////////////////////////////
/*!
   Return the type of the event fragment.  The event fragment
   type determines how the body should be interpreted.
*/
uint16_t
EventFragment::type() const
{
  return m_type;
}
//////////////////////////////////////////////////////////////////
/*!
   Return a copy of the event body. For efficient access,
   it is recommended that you use size() and operator[].
*/
vector<uint16_t>
EventFragment::body() const
{
  return m_body;
}
//////////////////////////////////////////////////////////////////
/*!
   Returns the number of elements in the body.
   Body indices then run from [0,size()).
*/
size_t
EventFragment:: size() const
{
  return m_body.size();
}
//////////////////////////////////////////////////////////////////
/*!
   Returns a reference to an element of the body array.
   Attempts to reference off the end of the vector are ill defined
   in this implementation.
*/
uint16_t&
EventFragment::operator[](size_t index) 
{
  return m_body[index];
}
const uint16_t&
EventFragment::operator[](size_t index) const
{
  return m_body[index];
}



/////////////////////////////////////////////////////////////////////
/////////////////////// static member utilities /////////////////////
/////////////////////////////////////////////////////////////////////


//  Extract the buffer type in host byte order from the raw buffer:

uint16_t
EventFragment::extractType(const uint16_t* rawbuffer) 
{
  const struct bheader* pHeader = reinterpret_cast<const struct bheader*>(rawbuffer);

  return tohs(pHeader->type, pHeader->ssignature);
}
// Extract the buffers size in howt byte order from the raw buffer.
// This function can also deal with the case that
// the buffer is a jumbo buffer.
// in that case, the low order part of the size is in
// nwds, the high order in unused[0]..and
// buffmt is 6 or greater.
//

uint32_t
EventFragment::extractSize(const uint16_t* rawBuffer)
{
  const struct bheader* pHeader = reinterpret_cast<const struct bheader*>(rawBuffer);

  uint32_t low = tohs(pHeader->nwds, pHeader->ssignature);
  uint32_t rev = tohs(pHeader->buffmt, pHeader->ssignature);
  if (rev < 6) {
    // 16 bit size...
    //
    return low;
  }
  uint32_t high = tohs(pHeader->unused[0], pHeader->ssignature);
  return (high << 16) | low;
}
//
// Extract the node id from the buffer return it in host byte order.
//
uint16_t
EventFragment::extractNode(const uint16_t* rawBuffer)
{
  const struct bheader* pHeader = reinterpret_cast<const struct bheader*>(rawBuffer);
  
  return tohs(pHeader->cpu, pHeader->ssignature);
}
//
// Extract the short signature from the buffer and return it
// in the buffer byte order so it can be used to construct a 
// conversion.
//
uint16_t
EventFragment::extractSsig(const uint16_t* rawBuffer)
{
  const struct bheader* pHeader = reinterpret_cast<const struct bheader*>(rawBuffer);
  return pHeader->ssignature;
}
//
// Extract the longword signature from the buffer and return it
// in the buffer byte order so it can be used to construct a conversion,.
//

uint32_t
EventFragment::extractLsig(const uint16_t* rawBuffer)
{
  const struct bheader *pHeader = reinterpret_cast<const struct bheader*>(rawBuffer);
  return pHeader->lsignature;
}
//
// Return a pointer to the body of the buffer.
//
const uint16_t* 
EventFragment::bodyPointer(const uint16_t* rawBuffer)
{
  const char* pBuffer = reinterpret_cast<const char*>(rawBuffer);
  pBuffer += sizeof(struct bheader);
  return reinterpret_cast<const uint16_t*> (pBuffer);
}
//
// Given a longword in buffer byte order, and the buffer's
// longword signature, return the longword in host byte order.
// 
uint32_t
EventFragment::tohl(uint32_t datum, uint32_t lsig)
{
  // If byte ordering matches just return datum:

  if (lsig == 0x01020304)  return datum;

  // else swap bytes and words:

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
// Given a words in buffer byte order, and the buffer's
// short signature, return the word in host byte order.
//
uint16_t
EventFragment::tohs(uint16_t datum, uint16_t ssig)
{
  if (ssig == 0x0102) return datum; // Same order as host.

  // swap bytes:

  union Word {
    uint16_t s;
    uint8_t  b[2];
  } source, dest;

  source.s = datum;
  dest.b[0]= source.b[1];
  dest.b[1]= source.b[0];

  return dest.s;
}
//  
// Return the entity count from a raw buffer in local byte ordering.
// 
uint16_t 
EventFragment::extractEntityCount(const uint16_t* rawBuffer)
{

  const struct bheader* pHeader = reinterpret_cast<const struct bheader*>(rawBuffer);

  return tohs(pHeader->nevt, pHeader->ssignature);

}

//
// Get a longword at a word offset.
//
uint32_t 
EventFragment::getLongword(const uint16_t* buffer,
			   size_t wordOffset,
			   uint32_t lsig)
{
  const uint32_t* pDatum = reinterpret_cast<const uint32_t*>(buffer + wordOffset);
  return tohl(*pDatum, lsig);
}
//
// Get a word at a specific offset:
//

uint16_t
EventFragment::getWord(const uint16_t* buffer,
			  size_t wordOffset,
			  uint16_t ssig)
{
  return tohs(*(buffer+wordOffset), ssig);
}
