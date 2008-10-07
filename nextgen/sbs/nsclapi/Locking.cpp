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

#include <CVMEInterface.h>
#include <ErrnoException.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <assert.h>
#include <string>

using namespace std;

// In some cases we need to define the semun union:

#if defined(__GNU_LIBRARY__) && !(defined(_SEM_SEMUN_UNDEFINED))
#else
union semun {
  int                val;	// SETVAL value.
  struct semid_ds    *buf;	// IPC_STAT, IPC_SET buffer.
  unsigned short int *array;	// Array for GETALL/SETALL
  struct seminfo*    _buf;	// Buffer for IPC_INFO
};
#endif


/*!
   This file implements coarse grained VME locking.
   If applications use this, the entire VME subsystem will be controlled
   by a single lock.  
*/

static int semid = -1;		// This will be the id of the locking semaphore
static int semkey= 0x564d4520;  // "VME " :-).

/*!
   This internal function is used to establish the semaphore:
   - If the semaphore exists, it's id is just stored in semid.
   - If the semaphore does not exist we try to create it O_EXCL
     this is an attempt to deal with any timing holes that may
     occur when two programs simultaneously attemp to create the semaphore.
   - If the O_EXCL creation succeeds (semaphore does not exist), it is
     given in initial value of 1 so that a single process can pass the lock
     gate.
   - If the O_EXCL creation fails, the process backs off for a while and
     then does a semget for an existing  semaphore again assuming that on the 
     second try, all initialization has been complete.

   \throw   CErrnoException
      If an error occured on any of the system calls.

*/
static void
AttachSemaphore()
{
  // Retry loop in case anybody makes and then kills it:

  while(1) {
    // Try to get the id of an existing semaphore:

    semid = semget(semkey, 0, 0777); // Try to map:
    if(semid >= 0) break;	     // Previously existing!!
    if(errno != ENOENT) {
      throw 
	CErrnoException("AttachSemaphore - semget error unexpected");
    }
    // Sempahore does not exist.  Try to be the only guy to 
    // create it:

    semid = semget(semkey, 1, 0777 | IPC_CREAT | IPC_EXCL);
    if(semid >= 0) {
      // We're the creator... initialize the sempahore, and return.

      union semun data;
      data.val = 1;

      int istat = semctl(semid, 0, SETVAL, data); // Allow 1 holder
      if(istat < 0) {
	throw CErrnoException("AttachSemaphore - semctl error unexpected");
      }

      break;
    }
    if(errno != EEXIST) {
      throw
	CErrnoException("AttachSemaphore - semget error unexpected");
    }
    //   The semaphore popped into being between the initial try
    //   to just attach it and our try to create it.
    //   The next semget should work, but we want to give
    //   the creator a chance to initialize the semaphore so
    //   we don't try to take out a lock on the semaphore before
    //   it is completely initialized:

    sleep(1);
  }
  return;
}
/*!
    Lock the semaphore.  If the semid is -1, the
    semaphore is created first.  

    \throw CErrnoException
       - Really from AttachSemaphore
       - From failures in semop.
*/
void 
CVMEInterface::Lock() 
{
  // If necessary, get the semaphore id..

  if (semid == -1) AttachSemaphore();
  assert(semid >= 0);		// Otherwise attach.. throws.

  struct sembuf buf;
  buf.sem_num = 0;		// Only one semaphore.
  buf.sem_op  = -1;		// Want to take the semaphore.
  buf.sem_flg = SEM_UNDO;	// For process exit.

  while (1) {			// In case of signal...
    int stat = semop(semid, &buf, 1);

    if(stat == 0) break;

    if(errno != EINTR) {	// Bad errno:
      throw CErrnoException("CVMEInterface::Lock semop gave bad status");
    }
    // On EINTR try again.
  }
  return;
}
/*!
  Unlock the semaphore. It is a crime to unlock the semaphore if it doesn
  not yet exist, since that would be unlocking a semaphore that is not yet
  locked.
  
  \throw  CErrnoException
     If the semop operation produced an error.
  \throw string
     If the semaphore did not yet exist.
*/
void
CVMEInterface::Unlock()
{
  if(semid == -1) {
    throw string("Attempt to unlock the semaphore before it was created");
  }
  struct sembuf buf;
  buf.sem_num = 0;
  buf.sem_op  = 1;
  buf.sem_flg= SEM_UNDO;	// Undoes the locking undo.

  while(1) {			// IN case of signal though not likely.
    int stat = semop(semid, &buf, 1);
    if(stat == 0) break;	// Got the job done!!
    if(errno != EINTR) {
      throw CErrnoException("CVMEInterface::Unlock semop gave bad status");
    }
    // on EINTR try again.
  }
  return;
}
