#ifndef __FRAGMENTQUEUE_H
#define __FRAGMENTQUEUE_H

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


#ifndef __STL_DEQUE
#include <deque>
#ifndef __STL_DEQUE
#define __STL_DEQUE
#endif
#endif


class EventFragment;

/*!
  Fragment queue is a queue of pointers to event fragments.
  the data sources put fragments in this queue and assembler stage
  inspects and pulls fragments out of this queue.
*/
class FragmentQueue {
public:
  typedef std::deque<EventFragment*>::iterator iterator;
private:
  std::deque<EventFragment*>  m_queue;
public:
  void            insert(EventFragment& fragment);
  EventFragment*  peek();
  EventFragment*  remove();
  size_t          size() const;
  iterator begin();
  iterator end();
};

#endif
