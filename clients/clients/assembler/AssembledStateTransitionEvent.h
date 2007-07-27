#ifndef __ASSEMBLEDSTATETRANSITIONEVENT_H
#define __ASSEMBLEDSTATETRANSITIONEVENT_h
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


#ifndef __ASSEMBLEDEVENT_H
#include "AssembledEvent.h"
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __CRT_TIME_H
#include <time.h>
#ifndef __CRT_TIME_H
#define __CRT_TIME_H
#endif
#endif

/*!
  State transition events mark run state transitions,
  e.g. from halted to active, active to paused and so on.
  These events are aggregated, and therefore always will
  come from node 0... The type, however must be specified at
  construction time.  State transition events also mark
  'assembly boundaries'  that is assembly of events will not
  occur across them...as all nodes are assumed to attempt to do
  state transitions close in time and hold the new state for some
  macroscopic length of time.
*/
class AssembledStateTransitionEvent : public AssembledEvent
{
private:
  std::string       m_title;
  unsigned long     m_elapsedTime;
  struct tm         m_absoluteTime;
  unsigned short    m_runNumber;

public:
  AssembledStateTransitionEvent(unsigned short run,
				AssembledEvent::BufferType type);

  void setTitle(std::string title);
  std::string getTitle() const;

  void setElapsedTime(unsigned long elapsedTime);
  unsigned long getElapsedTime() const;

  void setTimestamp(struct tm  timestamp);
  struct tm getTimestamp() const;

  unsigned short getRunNumber() const;
};


#endif
