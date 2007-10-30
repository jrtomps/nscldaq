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
#include "StateTransitionFragment.h"

#include <buffer.h>
#include <string.h>



using namespace std;

static const int RUNNUMOFFSET(3);

/*!
  Construct the buffer:

*/
StateTransitionFragment::StateTransitionFragment(uint16_t* pBuffer)  :
  EventFragment(extractNode(pBuffer),
		extractType(pBuffer),
		bodyPointer(pBuffer),
		extractSize(pBuffer) - sizeof(struct bheader)/sizeof(uint16_t)),
  m_ssig(extractSsig(pBuffer)),
  m_lsig(extractLsig(pBuffer)),
  m_runNumber(getWord(pBuffer, RUNNUMOFFSET, m_ssig))

{
}

/*!
    Return the title string:
*/
string
StateTransitionFragment::title() const
{

  // The buffer body starts with 80 bytes of title that
  // are gaurenteed to be null terminated.
  // We can therefore directly construct the title string from it.
  

  const char* pTitle = reinterpret_cast<const char*>(&((*this)[0]));
  return string(pTitle);
}
/*!
   Return the aboslute timestamp from the buffer, unused fields are
   zeroed.
*/
struct tm 
StateTransitionFragment::absoluteTime() const
{
  struct tm timestamp;
  memset(&timestamp, 0, sizeof(timestamp));

  timestamp.tm_mon = tohs((*this)[58-16], m_ssig);
  timestamp.tm_mday= tohs((*this)[59-16], m_ssig);
  timestamp.tm_year= tohs((*this)[60-16], m_ssig);
  timestamp.tm_hour= tohs((*this)[61-16], m_ssig);
  timestamp.tm_min = tohs((*this)[62-16], m_ssig);
  timestamp.tm_sec = tohs((*this)[63-16], m_ssig);


  return timestamp;

}
// Return the  run number:

uint16_t
StateTransitionFragment::getRunNumber() const
{
  return m_runNumber;
}
/*!
  Return the elapsed time into the run (host byte order).
*/
uint32_t
StateTransitionFragment::elapsedTime() const
{
  return getLongword(&((*this)[0]), 40,
		     m_lsig);
}
