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
#include "EventTooSmallException.h"
#include <stdio.h>

using namespace std;

/*!
  \param actualSize   - words in the event.
  \param requiredSize - Minimum number of words required in the size.
  \param doing - Context information.
*/
EventTooSmallException::EventTooSmallException(int actualSize,
					       int requiredSize,
					       std::string doing) :
  CException(doing),
  m_actualSize(actualSize),
  m_requestedSize(requiredSize)
{}

/*!
   \return int
   \retval The actual size of the mis-formatted event.
*/
int
EventTooSmallException::getActualSize() const
{
  return m_actualSize;
}

/*!
  \return int
  \return The requested minimum size for the event.
*/
int
EventTooSmallException::getRequestedSize() const
{
  return m_requestedSize;
}
/*!
  Return the reason code.. always -1, because I feel like it.
*/
Int_t 
EventTooSmallException::ReasonCode() const
{
  return -1;
}
/*!
   Build up a string that describes the reason for the exception.
*/
const char*
EventTooSmallException::ReasonText() const
{
  char integerBuffer[512];
  m_reasonText = "Event fragment size was too small. Actual size: ";
  sprintf(integerBuffer, "%d", m_actualSize);
  m_reasonText += integerBuffer;
  m_reasonText += " required minimum size: ";
  sprintf(integerBuffer, "%d", m_requestedSize);
  m_reasonText += integerBuffer;
  m_reasonText += " while: ";
  m_reasonText += WasDoing();

  return m_reasonText.c_str();

}
