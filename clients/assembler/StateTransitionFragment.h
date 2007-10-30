#ifndef __STATETRANSITIONFRAGMENT_H
#define __STATETRANSITIONFRAGMENT_H

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

#ifndef __EVENTFRAGMENT_H
#include <EventFragment.h>
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif


#ifndef __CRT_TIME_H
#include <time.h>		/*  For struct tm. */
#ifndef __CRT_TIME_H
#define __CRT_TIME_H
#endif
#endif


#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif


/*!
   This class describes and implements a state transition event fragment.
   These constitute assembly sentinells and are combined to a single
   state transition event.
*/
class StateTransitionFragment : public EventFragment
{
  uint16_t m_ssig;
  uint32_t m_lsig;
  uint16_t m_runNumber;
public:
  StateTransitionFragment(uint16_t* pBuffer);

  std::string title() const;
  struct tm   absoluteTime() const;
  uint32_t    elapsedTime() const;
  uint16_t    getRunNumber() const;
};


#endif
