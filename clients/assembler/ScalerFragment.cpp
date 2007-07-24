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
#include "ScalerFragment.h"
#include <buffer.h>



using namespace std;

//////////////////////////////////////////////////////////////////////////
/*!
   Construct a scaler fragment using just a raw data buffer.
   (with the help of the various helper functions.
   \param rawBuffer  - Pointer to a raw NSCLDAQ buffer.
*/
ScalerFragment::ScalerFragment(uint16_t* rawBuffer) :
  EventFragment(extractNode(rawBuffer),
		extractType(rawBufffer),
		bodyPointer(rawBuffer),
		extractSize(rawBuffer) - sizeof(bheader)/sizeof(uint16_t)),
  m_ssig(extractSsig(rawBuffer)),
  m_lsig(extractLsig(rawBuffer))
{
}
///////////////////////////////////////////////////////////////////////
/*!
   Consruct a scaler fragment from some pieces of extracted buffer
   and the body.
   \param node    - the node that created the fragment.
   \param type    - event type (need this 'cause we could be a snapshot e.g.).
   \param body    - Body of the event.
   \param bodyWords - The size of the body.

   \note Since the caller has broken this down already, the only
        values we can give the signatures are native byte ordering.
*/
ScalerFragment::ScalerFragment(uint16_t  node,
			       uint16_t  type
			       uint16_t* body,
			       size_t    bodyWords) :
  EventFragment(node, type, body, bodyWords),
  m_ssig(0x0102),
  m_lsig(0x01020304)
{
}
////////////////////////////////////////////////////////////////////
/*!
    Get the buffer start time.
*/
uint32_t 
ScalerFragment::startTime() const
{
  return getLongword(5);
}
///////////////////////////////////////////////////////////////////
/*!
  Get buffer stop time:
*/
uint32_t
ScalerFragment::endTime()   const
{
  return getLongWord(0);
}
/////////////////////////////////////////////////////////////////////
/*!
   Get the scalers as a vector.
*/
vector<uint32_t>
ScalerFragment::scalers() const
{
  vector<uint32_t> result;
  size_t offset = 10;
  while(offset < size()) {
    result.push_back(getLongword(offset));
    offset+= 2;
  }
  return result;
}
///////////////////////////////////////////////////////////////////
/*!
   Get the number of scalers:
*/
size_t
ScalerFragment::size()     const
{
  return (ScalerFragment::size() - 10)*sizeof(uint16_t)/sizeof(uint32_t);
}
///////////////////////////////////////////////////////////////////
/*!
   Get a specific scaler
   \param index  - selects the scaler to get (scaler number).
*/
uint32_t 
ScalerFragment:: operator[](size_t index) const
{
  index = index*sizeof(uint32_t)/sizeof(uint16_t) + 10;	// Short index into body.
  return getLongword(index);
  
}

////////////////////////////////////////////////////////////////////
//////////////////////// nonstatic member utilities ///////////////
///////////////////////////////////////////////////////////////////

/*
   convert a longword in the buffer body  given a word offset
   to a longword.  The longword is in the buffer native byte ordering.
   The result will be in the native byte ordering.

*/
uint32_t
ScalerFragment::getLongword(size_t wordOffset) 
{
  uint32_t* pLong = static_cast<uint32_t*>(&((*this)[wordOffset]));
  return tohl(*pLong, m_lsig);

}
/*
  Convert a word in the buffer body to a host ordered word given
  its offset
*/

uint16_t
ScalerFragment::getWord(size_t wordOffet)
{
  uint16_t* pWord = &((*this)[wordOffset]);
  return tohw(*pWord, m_ssig);
}
