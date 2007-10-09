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
#include "FragmentQueue.h"

using namespace std;

/*!
  Insert an item at the back of the fragment queue.
*/
void
FragmentQueue::insert(EventFragment& fragment)
{
  m_queue.push_back(&fragment);
}
/*
   Return the first element of the queue without removing it:
*/
EventFragment*
FragmentQueue::peek()
{
  if (m_queue.empty()) {
    return static_cast<EventFragment*>(NULL);
  }
  return m_queue[0];
}
/*!
   Remove the first element of the queue and return it:
*/
EventFragment*
FragmentQueue::remove()
{
  if (m_queue.empty()) {
    return static_cast<EventFragment*>(NULL);
  }
  EventFragment* result = peek();
  m_queue.pop_front();
  return result;
}
/*!
   Return the number of elements in the queue:
*/
size_t
FragmentQueue::size() const
{
  return m_queue.size();
}
/*!
 Get the begin iterator.
*/
FragmentQueue::iterator
FragmentQueue::begin()
{
  return m_queue.begin();
}
/*!
  Get the end iterator:
*/
FragmentQueue::iterator
FragmentQueue::end()
{
  return m_queue.end();
}
