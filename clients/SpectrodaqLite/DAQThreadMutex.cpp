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
#include <spectrodaq.h>

using namespace std;

/*!
   Create a DAQThreadMutex..  This has a synchronizable and a pointer
    to a guard.  The guard is created when the mutex is locked,
    it is stored in the member data when the guard creation completes
    indicating entry to the code protected by the synchronizable.
    Ulocking is deleteing the guard.
*/
DAQThreadMutex::DAQThreadMutex() :
  m_pGuard(0)
{}

/*!  
   Destroy a DAQThreadMutex... if necessary the gaurd is destroyed
   which unlocks the mutex.
*/
DAQThreadMutex::~DAQThreadMutex()
{
  UnLock();
}
/*!
   Lock the mutex.. The gaurd is created on the synchronizable
   object.  This will complete when the mutex is locked.
   The guard is then saved in m_pGuard for UnLock().
*/
void
DAQThreadMutex::Lock()
{
  m_pGuard = new SyncGuard(m_synchObject); // Lock.
}
/*!
  Unlock the mutex.  The order of operations is very important
  to ensure there's no race condition on m_pGuard... m_pGuard
  is copied locally.  m_pGuard is then zeroed.  Then the
  copy is deleted.
*/
void
DAQThreadMutex::UnLock()
{
  SyncGuard* pGuard = m_pGuard;
  m_pGuard          = 0;
  delete pGuard;
}
