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
#include <CFragmentSource.h>
#include <assert.h>
#include <unistd.h>

static const useconds_t STALL_TIME(1000); // Microseconds to stall the data source.

// Implementation of the non-null methods of the CFragmentSource abstract base class.

/*!
 * Construction
 */
CFragmentSource::CFragmentSource()
{}

/*!
  Destruction supports base class destructor chaining:
*/
CFragmentSource::~CFragmentSource()
{
  m_signal.broadcast();		// Free any 'stuck' threads before total destruction.
}

/////////////////////////////////////////////////////////////////////////////////////////

/*!
  Signal and event is available.  This will broadcast the condition variable.
  Note that callers that do this must own the mutex.  For threads receiving the signal
  to be schedulable, the caller must at some point relinquish the mutex.
*/
void
CFragmentSource::signalEvent()
{
  m_signal.signal();

}
/*!
   Enter the mutex's mutual exclusion zone.  There's no deadlock/multi-lock protection
   so users better know what they're doing, or play it safe and use this framework.
*/
void
CFragmentSource::enter()
{
  m_lock.lock();
}
/*!
   Leave the mutex's mutual exclusion zone.  See enter() above.
*/
void
CFragmentSource::leave()
{
  m_lock.unlock();
}

/////////////////////////////////////////////////////////////////////////////////////

//
// Pure virtual methods can have implementations according to ISO C++.
// in this case they don't do anything but serve to provide place holders
// that describe the expectations that should be met by a concrete class.
// invoking one of these directly will cause an assert(0) to execute.

/*!
   Function call operator.  This is the entry point for the thread that's supposed
   to check for and if necessary buffer up data from the source.  It will be called
   with the mutex held.  Therefore, it should do one of the following:
   - Run for a while and then return true to indicate that it
     should be called some later time with the mutex set.
  - Run for a while and then release/reacquire the mutex before continuing.

  @return bool
  @retval true - Call this function again after giving up the mutex and yielding for a bit.
  @retval false - Exit the thread.

*/
bool
CFragmentSource::operator()()
{
  assert(0);			// Concrete classes must implement this.
}
/*!
   This should return the type of the nexte event available from the data source.
   This should be one of the values of teh CBuilderConstant::EventType enum.
   The method should be entered holding the mutex.  See, however, getNextType()
   below.

   @return CBuilderConstant::EventType
   @retval NOEVENT - no event is available.
   @retval other   - Type of the next event you'd put into a fragment with
                     addNext
*/
CBuilderConstant::EventType
CFragmentSource::provideType()
{
  assert(0);
}
/*!
   This entry should discard the next fragment from the data source.  The function should be
   entered with the mutex held.  See, however dicardNext below.
*/
void
CFragmentSource::discardFragment()
{
  assert(0);
}
/*!
   This method should add the next fragment from the data source to the event fragment.
   - This method should be entered with the mutex held, see addNext below.
   - On return, the discardFragment method will be called to remove the fragment
     from the event source.

     @param   pData - Pointer to the buffer in which to store the event.
     @return void*
     @retval - Points past the data inserted by this method.

*/
void*
CFragmentSource::addNextFragment(void* pData)
{
  assert(0);
  return reinterpret_cast<void*>(0);
}
/*!

  Returns the timestamp of the next fragment.  Tocall this, the caller should have some evidence
  that there is a next fragment as the return value is not defined if it does not.  The
  method should be called with the mutex held by the calling thread.
  @return uint64_t (64 bit unsigned).
  @retval Timestamp encoded in the nexte vent.

*/
uint64_t
CFragmentSource::provideTimestamp()
{
  assert(0);
}

/*!
  This driver function should return true if the data source has data available.
  This should be called with the mutex held.
  @return bool
  @retval true - Data source has data available.
  @retval false - Data source has no data available.
*/
bool
CFragmentSource::dataPresent()
{
  assert(0);
}

//////////////////////////////////////////////////////////////////////////////////

/*!
   Run the acquisition thread.  This repeatedly calls operator() until
   it returns a false.  Between each call:
   - A short sleep is done of a few milliseconds
   - The mutex is released/regotten.
*/
void
CFragmentSource::run()
{
  bool keepRunning = true;
  while (keepRunning) {
    usleep(STALL_TIME);
    enter();
    keepRunning = (*this)();
    leave();
  }
}


////////////////////////////////////////////////////////////////////////////////////

/*!
   Provides a callback handler that deals with data when signaled.
   The callback handler will execute with the mutex held.
   @param handler - Reference to the handler object.

   @note the handler object is passed *this.

*/
void
CFragmentSource::handleData(CFragmentSource::CFragmentHandler& handler)
{
  enter();
  m_signal.wait(m_lock);
  handler(*this);
  leave();
}
/*!
  Requests the next data type from the data source.
  @param lock - If true, the call to the driver function is bracketed with
                an enter()/leave() pair.  By default this is false under the
		assumption a handler as in handleData above may be calling this
		in which case  it already has the mutex.
  @return CBuilderConstant::EventType
  @retval CBuilderConstant::NOEVENT - no event is present.
  @retval other - The type of the next available event.

*/
CBuilderConstant::EventType
CFragmentSource::getNextType(bool lock)
{
  if (lock) enter();
  CBuilderConstant::EventType type = provideType();
  if (lock) leave();

  return type;
  
}
/*!
   Discard the next event from the source.
   @param lock If true, the mutex is acquired prior to invoking the driver
               discardFragment method, otherwise not.
*/
void
CFragmentSource::discardNext(bool lock)
{
  if (lock) enter();
  discardFragment();
  if (lock) leave();
}

/*!
  Add the next fragment from this data source to an event being built.
  @param pData - Pointer to where the fragment should be added.
  @param lock  - If true, the call to the driver addNextFragment method
                 is bracketed with an enter/leave pair.
  @return void*
  @retval      - Points past the data inserted by the driver.

*/
void*
CFragmentSource::addNext(void* pData, bool lock)
{
  if (lock) enter();
  void* pNext = addNextFragment(pData);
  if (lock) leave();

  return pNext;
}

/*!
  Return the timestamp from the next fragment.
  Note that if there is no next fragment, the return value is undefined...and
  the funtion may in fact not even return.
  @param lock - If true the call to the driver provideTimestamp method is bracketed
                in mutex lock/unlock.
  @return uint64_t
  @retval The 64 bit timestamp for the next event from this data source.
*/
uint64_t
CFragmentSource::getNextTimestamp(bool lock)
{
  if (lock) enter();
  uint64_t stamp = provideTimestamp();
  if (lock) leave();

  return stamp;
}
/*!
   @param lock If true the call to the driver's dataPresent method is 
          bracketed in mutex lock/unlock.
   @return bool
   @retval true  - the data source has at least one fragment.
   @retval false - The data source has no fragments.
*/
bool
CFragmentSource::hasData(bool lock)
{
  if (lock) enter();
  bool fragments = dataPresent();
  if (lock) leave();

  return fragments;
}
