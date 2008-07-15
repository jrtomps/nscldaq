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
#include <CGaurdedObject.h>

/*!
   Enter a critical region. This is just locking the mutex.  Our thread
   will block until the mutex is released.
*/
void
CGaurdedObject::Enter()
{
  m_Mutex.lock();
}
/*!
   Leave a critical region.  THis is just unlocking the mutex.
   Most likely this causes a scheduler pass if there are threads that
   are queud in the Enter... 
*/
void
CGaurdedObject::Leave()
{
  m_Mutex.unlock();
}
/*!
   Returns a reference to the mutex.  This is required for objects that
   may also want to implement a condition variable on top of this.
   For example the CBufferQueue class.
*/
CMutex&
CGaurdedObject::mutex()
{
  return m_Mutex;
}
