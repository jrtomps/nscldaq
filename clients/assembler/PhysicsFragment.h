#ifndef __PHYSICSFRAGMENT_H
#define __PHYSICSFRAGMENT_H

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
#include "EventFragment.h"
#endif

/*!
   Class to hold a physics event fragment.  As far as
   the event asembler is concerned, the body of a physics event
   fragment is just an undifferentiated soup of uint16_t.
   Therefore , this class defines no new methods.  Just
   simplifies construction.
*/
class PhysicsFragment : public EventFragment
{
private:

  uint32_t       m_timeStamp;
public:
  PhysicsFragment(uint16_t node,
		  void*    body,
		  size_t   words,
		  off_t    offset=0,
		  uint32_t timestamp=0);
  PhysicsFragment(uint16_t node,
		  std::vector<uint16_t> body,
		  uint32_t timestamp=0);

  uint32_t getTimestamp() const;
};



#endif
