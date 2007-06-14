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
#include "AssembledStateTransitionEvent.h"
#include <RangeError.h>
#include <string.h>

using namespace std;

/////////////// Constructors and explicit cannonicals ///////////////////

/*!
  Construct the state transition event.  
  
  \param run   The number of the run involved in the transition.
  \param type  The type of transition being attempted.  There is no default
               type.
  \throws CRangeError if type is not a valid state transition event type.
*/

AssembledStateTransitionEvent::AssembledStateTransitionEvent(
						 unsigned short run,
						 AssembledEvent::BufferType type) :
  AssembledEvent(0, type),
  m_runNumber(run)
{
  // Throw a range error if the type warrants it:  

  if ((type != AssembledEvent::BeginRun)            &&
      (type != AssembledEvent::EndRun)              &&
      (type != AssembledEvent::PauseRun)            &&
      (type != AssembledEvent::ResumeRun)) {
    throw CRangeError(AssembledEvent::BeginRun,
		      AssembledEvent::ResumeRun,
		      type,
      "Bad state transition type constructing an AssembledStateTransitionEvent");
  }
}

////////////////// Access to the title string: ///////////////

/*!
  Set a new title:

  \param title  - The new title string.

*/
void
AssembledStateTransitionEvent::setTitle(string title)
{
  m_title = title;
}
/*!
  Retrieve the title:
*/
string
AssembledStateTransitionEvent::getTitle() const
{
  return m_title;
}
//////////////////  Access to the elapsed run time ////////////////

/*!
  Set the time at which the run state transition occured relative
  to the start of the rum:
  \param elapsedTime  Elapsed time of transition.
*/
void
AssembledStateTransitionEvent::setElapsedTime(unsigned long elapsedTime)
{
  m_elapsedTime = elapsedTime;
}

/*!
  Get the elapsed time of the run in seconds:
*/
unsigned long
AssembledStateTransitionEvent::getElapsedTime() const
{
  return m_elapsedTime;
}
/////////////////// Access to the absolute timestamp /////////////

/*!
   Set the  timestamp:

   \param timestamp the timestamp to use.
*/
void
AssembledStateTransitionEvent::setTimestamp(struct tm timestamp)
{
  memcpy(&m_absoluteTime, &timestamp, sizeof(timestamp));
}

/*!
  Retrive the timestamp
*/
struct tm
AssembledStateTransitionEvent::getTimestamp() const
{
  return m_absoluteTime;
}
///////////////////////////   Access the run number///

/*!
   \return unsigned short
   \retval the run number.
*/
unsigned short
AssembledStateTransitionEvent::getRunNumber() const
{
  return m_runNumber;
}
