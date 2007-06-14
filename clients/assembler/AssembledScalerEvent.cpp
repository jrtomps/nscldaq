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
#include "AssembledScalerEvent.h"
#include <RangeError.h>

using namespace std;

///////////////////// Constructors and expclicit canonicals ////////////////////

/*!
  Construct the object:
  \param node       - The originating node.  Scaler events are not
                      assembled, so this is the actual originating node id.
  \param startTime  - The scaler interval start time.
  \param endTime    - The scaler interval end time.
  \param BufferType - Type of buffer.   This defaults to
                      AssembledEvent::Scaler if it is not this
		      value or AssembledEvent::SnapshotScaler,
		      An CRangeError Exception is thrown.
*/
AssembledScalerEvent::AssembledScalerEvent(unsigned short node,
					   unsigned long startTime,
					   unsigned long endTime,
					   AssembledEvent::BufferType type) :
  AssembledEvent(node, type),
  m_startTime(startTime),
  m_stopTime(endTime)
{
  if ((type != AssembledEvent::Scaler) && (type != AssembledEvent::SnapshotScaler)) {
    throw CRangeError(AssembledEvent::Scaler,
		      AssembledEvent::SnapshotScaler,
		      type,
	      "Constructing an AssembledScalerEvent - event type not a scaler");
  }
}


/////////////////////// Time accessors: /////////////////////////////////

/*!
  \return unsigned long
  \retval The elapsed time in seconds of the start of the interval over which
          these scalers were accumulated (remember NSCL scalers are incremental
          not absolute values).
*/
unsigned long
AssembledScalerEvent::getStartTime() const
{
  return m_startTime;
}
/*!
   \return unsigned long
   \retval The elapsed time in seconds of the end of the interval over which
           these scalers were accumulated.
*/
unsigned long
AssembledScalerEvent::getEndTime() const
{
  return m_stopTime;
}

////////////////////// Add scalers/get scalers /////////////////////////

/*!
  Add a set of scalers given a pointer to them.
  \param pScalers  - pointer to the scalers to add.
  \param count     - Number of 32 bit scalers to add.

*/
void
AssembledScalerEvent::addScalers(void* pScalers, size_t count)
{
  uint32_t *pSrc(static_cast<uint32_t*>(pScalers));
  m_scalers.insert(end(), pSrc, pSrc+count);
}
/*!
   Add a set of scalers given them as a vector:
   \param scalers  - The vector of scalers to append to the set we have.

*/
void
AssembledScalerEvent::addScalers(vector<uint32_t> scalers)
{
  m_scalers.insert(end(),
		   scalers.begin(),
		   scalers.end());
}
/////////////////////// Retrieve/access scalers /////////////////////

/*!
   Get a copy of the current scalers:
*/
vector<uint32_t>
AssembledScalerEvent::getScalers() const
{
  return m_scalers;
}
/*!
   Get a reference to a single scaler.. note this reference
   allows modification as well as retrieval.. use with care!!!
*/
uint32_t&
AssembledScalerEvent::operator[](unsigned int index)
{
  return m_scalers[index];
}

///////////////////////// Delegations to the m_scalers vector////////


///

size_t
AssembledScalerEvent::size() const
{
  return m_scalers.size();
}

///

vector<uint32_t>::iterator
AssembledScalerEvent::begin()
{
  return m_scalers.begin();
}

///

vector<uint32_t>::iterator
AssembledScalerEvent::end()
{
  return m_scalers.end();
}
