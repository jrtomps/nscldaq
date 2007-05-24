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
#include <spectrodaq.h>
#include <string.h>

using namespace std;

/*!
  Implementation functions of the non-inline members
  of DAQWordBufferPtr..  a small wrapper for uint16_t*
  to be a facade for spectrodaq-lite.
*/

/*!
  Copy a block of data to some arbitrary place.
  \param pDest    - Copy destination.
  \param offset   - Offset from m_pData (in words)
  \param nwds     - Number of words to copy.
*/
void
DAQWordBufferPtr::CopyOut(void* pDest, off_t offset, size_t nwds)
{
  uint16_t* src = m_pData + offset;
  memcpy(pDest, src, nwds*sizeof(uint16_t));
}

/*!
  Copy a block of memory into the buffer from an arbitrary place.
  All the parameters are the same as CopyOut but:
  \param pSrc is where the data are copied to.

*/
void
DAQWordBufferPtr::CopyIn(void* pSrc, off_t offset, size_t nwds)
{
  uint16_t* dst = m_pData + offset;
  memcpy(dst, pSrc, nwds*sizeof(uint16_t));
}

//! Dereference the pointer:

uint16_t& 
DAQWordBufferPtr::operator*()
{
  return *m_pData;
}
//!  Prefix ++  (fastest):

DAQWordBufferPtr&
DAQWordBufferPtr::operator++()
{
  m_pData++;
  return *this;
}

//! Postfix ++

DAQWordBufferPtr
DAQWordBufferPtr::operator++(int)
{
  DAQWordBufferPtr result(m_pData++);
  return result;

}

//! Add an offset:

DAQWordBufferPtr&
DAQWordBufferPtr::operator+=(int offset) 
{
  m_pData += offset;
  return *this;
}

//! Indexing:

uint16_t&
DAQWordBufferPtr::operator[](int index) 
{
  return m_pData[index];
}

/*!
   Get the index of the current pointer (m_pData)
   relative to the pointer at construction time (m_pInitial)
*/
size_t
DAQWordBufferPtr::GetIndex()
{
  return (m_pData - m_pInitial);
}




//!  Simulate the & operator on daqwordbuffers:

DAQWordBufferPtr operator&(DAQWordBuffer& buffer) {
  return DAQWordBufferPtr(buffer.GetPtr());
}



