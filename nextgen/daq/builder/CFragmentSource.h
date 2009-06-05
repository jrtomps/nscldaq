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

#ifndef __CFRAGMENTSOURCE_H
#define __CFRAGMENTSOURCE_H

#ifndef  DAQHWYAPI_THREAD_H
#include <Thread.h>
#endif

#ifndef __CMUTEX_H
#include <CMutex.h>
#endif

#ifndef __CCONDITION_H
#include <CCondition.h>
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif


#ifndef __CCONFIGURABLEOBJECT_H
#include <CConfigurableObject.h>
#endif

/*!
   TODO: This does not belong here but in a separate file along with other
   constants etc:
*/
class CBuilderConstant {
public:
  typedef enum _EventType_ {
    PHYSICS,
    SCALER,
    BEGIN,
    END,
    PAUSE,
    RESUME,
    PACKETDOCS,
    VARIABLES,
    TRIGGERCOUNT,
    NOEVENT
  } EventType;
};

/*!
   Abstract base class for a source of event fragments.  Event fragment
   sources are responsible for getting event fragments from somewhere and
   making them available to the builder in a controlled way.
   The acquisition part of the class is a thread (we are derived from 
   a thread).  A mutex and a condition variable is included in the class 
   along with methods locking/unlocking the mutex and signalling/waiting
   on  the condition.


*/
class CFragmentSource : public Thread
{
private:
  CMutex               m_lock;
  CConditionVariable   m_signal;

public:
  class CFragmentHandler 
  {
  public:
    virtual void operator()(CFragmentSource& source) = 0;
  };

public:
  CFragmentSource();
  virtual ~CFragmentSource( );
private:
  CFragmentSource(const CFragmentSource&);
  CFragmentSource& operator=(const CFragmentSource&);
  int operator==(const CFragmentSource& rhs) const;
  int operator!=(const CFragmentSource& rhs) const;
  
  // 'Internal' methods:

protected:
  void signalEvent();		// Input thread calls this to signal data availability.
  void enter();			// locks m_lock.
  void leave();			// unlocks m_lock.

  // Driver pure virtuals fragments must implement these.

  virtual bool     operator()()                 = 0;
  virtual CBuilderConstant::EventType      provideType()                = 0;
  virtual void     discardFragment()            = 0;
  virtual void*    addNextFragment(void *pData) = 0;
  virtual uint64_t provideTimestamp()           = 0;
  virtual bool     dataPresent()                = 0;

  // Final function for running the acquisition thread of the data source.

  virtual void run();                     // Final.
  
  // Public entries:

public:
  void handleData(CFragmentHandler& handler);   // Invoke handler when event signalled.
 
  CBuilderConstant::EventType getNextType(bool lock=false);
  void         discardNext(bool lock=false);
  void*        addNext(void* pData, bool lock=false);
  uint64_t     getNextTimestamp(bool lock=false);
  bool         hasData(bool lock=false);
  
};


#endif
