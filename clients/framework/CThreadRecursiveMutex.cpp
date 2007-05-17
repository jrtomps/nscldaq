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

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";/*! \file  CThreadRecursiveMutex.cpp 
           Provides a mutex which can be locked in a nested manner by a thread.
           The "nested ness'  is managed through the m_nLockLevel and the 
           m_OwningThread member.  Atomicity of the otherwise non-atomic 
           function is handled by bracketing calls with locks of the mutexe's
           own m_MonitorMutex
           
*/

////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>

// Need to do this here to be compatible with spectrodaq.h.

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include "CThreadRecursiveMutex.h"    				
#include <spectrodaq.h>
#include <errno.h>

#include <assert.h>

#ifdef __DEBUG__
#include <Iostream.h>
#include <Iomanip.h>
#endif

//!
//  Constructor for CThreadRecursiveMutex:
//    Initializes as follows:
//
//    m_nLockLevel    0
//   
//    All others initialized through default constructors.
CThreadRecursiveMutex::CThreadRecursiveMutex ()  : 
  m_nLockLevel(0)
{   

}
//!
// Destructor:
//   Releases the mutexes and that's about it.
//  Note that the monitor mutex is locked and then unlocked just
//  prior to completion, this ensures onle a single thread destruction.
//
CThreadRecursiveMutex::~CThreadRecursiveMutex()
{
  m_MonitorMutex.Lock();
  if(m_nLockLevel > 0) {
    m_Mutex.UnLock();
  }
  m_MonitorMutex.UnLock();

}

// Functions for class CThreadRecursiveMutex

/*!
  \fn int CThreadRecursiveMutex::Lock() 
 Operation Type:
    Override.
 
Purpose: 	

Locks the mutex.  Returns zero on success,
otherwise, errno has the reason for the failure.
Note that if we own the mutex the lock level is
incremented, otherwise, this function may block.

*/
int 
CThreadRecursiveMutex::Lock()  
{ 
  // If the lock is owned by this process we just increment
  // the lock level and continue:

  m_MonitorMutex.Lock();	// Freeze member data:
#ifdef __DEBUG__
  cerr << "CThreadRecursiveMutex::Lock() - level: " << m_nLockLevel
       << " Owner: " << hex << m_tOwningThread << dec << endl;
  cerr.flush();
#endif
  if( (m_nLockLevel > 0) && (m_tOwningThread == daqthread_self())) {
    m_nLockLevel++;
    m_MonitorMutex.UnLock();
    return 0;
  }
  m_MonitorMutex.UnLock();

  // enter the queue of threads waiting for the mutex:
  //

  // Compete for the lock with the owner of the lock.
  //
  m_Mutex.Lock();
  m_MonitorMutex.Lock();	// Freeze member data.
  m_nLockLevel++;
  m_tOwningThread = daqthread_self();
#ifdef __DEBUG__
  cerr << "CThreadRecursiveMutex::Lock() New owner: " 
       << hex << m_tOwningThread << dec << endl;;
  cerr.flush();
#endif
  m_MonitorMutex.UnLock();

  return 0;


}  

/*!
  \fn int CThreadRecursiveMutex::UnLock() 
 Operation Type:
    Override
 
Purpose: 	

Unlocks a locked mutex.  If the mutex is already locked by us,
the lock level is decremented.  The mutex is not actually released
until the lock level goes to zero.  If we don't own the mutex, and
error results.
Returns zero on success,
otherwise, errno has the reason for the failure.

*/
int 
CThreadRecursiveMutex::UnLock()  
{

  //! \bug Status returns from lock and unlock are not checked.

  // Freeze member data:

  m_MonitorMutex.Lock();
#ifdef __DEBUG__
  cerr << "CThreadRecursiveMutex::UnLock(), Level: "
       << m_nLockLevel << " Owner " << hex << m_tOwningThread << dec << endl;
  cerr.flush();
#endif

  // Require that I actually own the mutex:

  if((m_nLockLevel > 0) && (m_tOwningThread == daqthread_self())) {
    m_nLockLevel--;
    if(m_nLockLevel <= 0) {	// Release mutex if completely unlocked.
#ifdef __DEBUG__
      assert(m_nLockLevel == 0); // Should be exactly 0 here.
#endif
      m_Mutex.UnLock();
      m_MonitorMutex.UnLock();
      return 0;
    }
  }
  m_MonitorMutex.UnLock();	// Unfreeze member data.

  // I don't own the lock.

  errno = EPERM;
  return -1;
    
}  




/*!
  \fn int CThreadRecursiveMutex::isLocked() 
 Operation Type:
    Override.
 
Purpose: 	

Returns non zero if someone, anyone (even self()) owns the mutex.
Note that the mutex is considered owned if the lock level is 
nonzero.  This should even be faster than the base class 
implementation.

*/
int 
CThreadRecursiveMutex::isLocked()  
{ 
  int LockLevel;
  m_MonitorMutex.Lock();
  LockLevel = m_nLockLevel;
  m_MonitorMutex.UnLock();

  return (LockLevel > 0) ? -1 : 0;
}  

/*!
  \fn void CThreadRecursiveMutex::UnLockCompletely() 
 Operation Type:
    
Purpose: 	

Releases all lock levels.  If we don't 
already own the mutex, this call is a no-op,
otherwise, the m_nLockLevel variable is set to
zero and the underlying semaphore is unlocked.

*/
void 
CThreadRecursiveMutex::UnLockCompletely()  
{
  m_MonitorMutex.Lock();	// Freeze data structures.

  // Only do something if we own the mutex:

  if((m_nLockLevel > 0) && (m_tOwningThread == daqthread_self())) {
    m_nLockLevel = 0;		// Set the lock level back to zero...
    m_Mutex.UnLock();		// Release the lock.
  }

  m_MonitorMutex.UnLock();	// Unfreeze data.
  
}
