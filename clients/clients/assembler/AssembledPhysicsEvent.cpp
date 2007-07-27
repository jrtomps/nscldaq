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
#include "AssembledPhysicsEvent.h"

#include <string.h>


using namespace std;

///////////////////////// Constructors: /////////////////////

AssembledPhysicsEvent::AssembledPhysicsEvent() :
  AssembledEvent()
{}

/////////////////////// Set/get data ///////////////////////


/*!
    Add some data to the end of the event body.
    \param pData   - Pointer to the data to add.
    \param wordCount - Number of words to add.

*/
void
AssembledPhysicsEvent::addData(void*  pData,
			       size_t wordCount)
{
  unsigned short *pSource(static_cast<unsigned short*>(pData));
  for (int i=0; i < wordCount; i++) {
    m_body.push_back(*pSource++);
  }
  
}

/*!
    Copies the data from the vector to arbitrary
    storage.  Here there is an assumption of packed contiguity in the
    elements.
*/
void
AssembledPhysicsEvent::copyOut(void* pTarget) const
{
  const  void* pSrc(&(m_body.front()));

  memcpy(pTarget, pSrc, m_body.size()*sizeof(unsigned short));
}


////////////////////// Access to underlying data vector /////

/*!
   \return size_t
   \retval number of unsigned shorts in the body.
*/
size_t
AssembledPhysicsEvent::size() const
{
  return m_body.size();
}
/*!
   \return std::vector<unsigned short>::iterator
   \retval Begin of iteration iterator to the body.

*/
vector<unsigned short>::iterator
AssembledPhysicsEvent::begin()
{
  return m_body.begin();
}
/*!
   \return std::vector<unsigned short>::iterator
   \retval End of iteratino iterator in the body array.
*/
vector<unsigned short>::iterator
AssembledPhysicsEvent::end()
{
  return m_body.end();
}
