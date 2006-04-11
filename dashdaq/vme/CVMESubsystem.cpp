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
#include "CVMESubsystem.h"
#include "CVMEInterface.h"
#include "parseUtilities.h"

#include <RangeError.h>
#include <ErrnoException.h>
#include <CInvalidInterfaceType.h>
#include <CVMEInterfaceFactory.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

using namespace descriptionFile;

// Crappy X-open makes us define semun:

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
  int val;                  /* value for SETVAL */
  struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
  unsigned short *array;    /* array for GETALL, SETALL */
  /* Linux specific part: */
  struct seminfo *__buf;    /* buffer for IPC_INFO */
};
#endif


// Static class members:

bool CVMESubsystem::m_lockCreated(false);
int  CVMESubsystem::m_lockId;
bool CVMESubsystem::m_lockHeld(false);

CVMESubsystem::Interfaces CVMESubsystem::m_Interfaces;


/*!
   Install a new interface.  A crate number is allocated and returned
   for the interface that is added.
   \param interface : CVMEInterface& [in]
      A reference to an object derived from the abstract base class
      CVMEInterface of all VME interfaces.
   \param deleteWhenDone : bool [in];  defaults to: false
      If this is true, then when the VME Subsystem no longer needs
      the interface, it will be deleted.  This happens when:
      - The VME subsystem is destroyed.
      - The interface is replaced via a call to replaceInterface.
   \return int
   \retval Number to use to retrieve this VME crate from the subsystem.
*/
int
CVMESubsystem::installInterface(CVMEInterface& interface,
				bool deleteWhenDone)
{
  Interface i = {deleteWhenDone, &interface};
  m_Interfaces.push_back(i);

  return (m_Interfaces.size() - 1);
}

/*!
   Replace an existing interface with another one.
   \param index : unsigned int [in]
       Index of the interface to replace. 
   \param replacement : CVMEInterface& 
       The new interface that will take over that crate.
   \param deleteWhenDone : bool, defaults to false
       Determines if the interface is deleted when the subsystem
       no longer needs it.
   \return CVMEInterface*
   \retval The previous interface for this index.  Note that
           if the interface was added with deleteWhenDone true,
           this will point to an object that has been deleted.
           If you want to do any pre-replacement cleanup you should
	   get hold of the interface prior to this call by e.g.
           indexing do the cleanup and then do the replacement.
*/
CVMEInterface*
CVMESubsystem::replaceInterface(unsigned int   index,
				CVMEInterface& replacement,
				bool           deleteWhenDone)
{
  // Make sure the index is in range.

  if (index >= m_Interfaces.size()) {
    throw CRangeError(0, m_Interfaces.size()-1, index,
		      "CVMESubsystem::replaceInterface - index to replace");
  }
  // Get the old interface record by reference so we can modify it:

  Interface&     old(m_Interfaces[index]);

  /// Take care of the old interface:

  CVMEInterface* pPrior  = old.s_pInterface;

  if (old.s_mustDelete) {
    delete pPrior;		// If here prior is now invalid!!!!!!!!
  }
  // replace with the new

  old.s_pInterface = &replacement;
  old.s_mustDelete = deleteWhenDone;

  //// Warning pPrior may be invalid!!!!

  return pPrior;
}

/*!

   \return size_t
   \retval Number of interfaces that are currently installed.
*/
size_t
CVMESubsystem::size() 
{
  return m_Interfaces.size();
}
/*!
   Return a begin of iteration iterator to the interfaces.
   The iterator can be treated as a pointer to a structure
   that contains the following elements:
   - s_mustDelete - true if the interface is deleted after
                    being used by the subsystem (see installInterface).
   - s_pInterface - Pointer to the CVMEInterface* object.
   
   The iterator can be incremented or decremented.  If after an
   increment it is equal to the value returned by end(), you've run off
   the end of the array.
   \return InterfaceIterator
*/
CVMESubsystem::InterfaceIterator
CVMESubsystem::begin()
{
  return m_Interfaces.begin();
}
/*!
    Returns an end of iteration iterator.  For more on interators,
    see the Musser, Derge, Saini book on the Standard Template Library.
    See also begin earlier in this file.
*/
CVMESubsystem::InterfaceIterator
CVMESubsystem::end()
{
  return m_Interfaces.end();
}

/*!
   Index into the subsystems and return a reference to the 
   interface.  Note that many interfaces don't support copy construction.
   Furthermore you don't want to deal with slicing down to the
   base class which, after all is abstract.
   Thus the only ways to safely index are:
   - subsystem[index].method();  Invoke a method from the indexed element
     directly
   - CVMEInterface& interface(subsystem[index]);
   - CVMEInterface* pInterface = &(subsystem[index]);
   
   \param index : unsigned int [in]
        Number of VME crate for which you want an interface object reference.
   \return CVMEInterface&
   \retval A reference to the selected interface.  See above for
           safe ways to use this.
   \throw CRangeError - if the index is not in range.
*/
CVMEInterface& 
CVMESubsystem::operator[](unsigned int index)
{
  // range check the index:

  if(index >= size()) {
    throw CRangeError(0, size()-1, index,
		      "CVMESubsystem::[] index is too large");
  }
  return *(m_Interfaces[index].s_pInterface);
}
 

/*!
    Process an interface description line to create a new interface.
    The interface type is the first word extracted from the
    description, the remainder is considered to be configuration.

    \param description std::string [in]
       Description string of the form "type device dependent configuration"
    \return int
    \retval Crate number assigned to this interface.

    \throw CInvalidInterfaceType   if the factory returned a null.
          
*/
int
CVMESubsystem::processDescription(string description)
{
  CVMEInterface* pInterface = createFromDescription(description);

  return installInterface(*pInterface, true);
  
}
/*!
   Process an entire description file.
   \param file : istream [modified]
       File to process.
*/
void
CVMESubsystem::processDescriptionFile(istream& file)
{
  // The correctly created interfaces are stored in created.
  // if there's an exception, they get destroyed.
  //
  vector<CVMEInterface*> created;

  
  // Try to create the interfaces requested by the file..

  try {
    while (!file.eof()) {
      string line = getLine(file);
      line        = stripComment(line);
      line        = stripLeadingBlanks(line);
      line        = stripTrailingBlanks(line);

      if (line != string("")) {
	created.push_back(createFromDescription(line));
      }
    }
  }
  catch (...) {
    while(! created.empty()) {
      CVMEInterface* pInterface = created.back();
      delete pInterface;
      created.pop_back();
    }

    throw;
  }
  // We've made all the interfaces, now add them:

  for (int i = 0; i < created.size(); i++) {
    CVMEInterface* pInterface = created[i];
    installInterface(*pInterface, true);
  }
}

/*!
    lock the subsystem.  Once locked the interface onLock functions are called.
*/
void
CVMESubsystem::lock()
{
  EnsureSemaphoreExists();
  if (m_lockHeld) {
    throw string("Recursive lock attempted!");
  }
  sembuf lock = {0, -1, SEM_UNDO};

  while(1) {
    int stat = semop(m_lockId, &lock,  1);
    if(stat == 0) {
      for (int i = 0; i < m_Interfaces.size(); i++) {
	m_Interfaces[i].s_pInterface->onLock();
      }
      m_lockHeld = true;
      return;
    }
    if(stat < 0 && errno != EINTR) {
      throw CErrnoException("Locking VME Semaphore");
    }
  }
}

/*!
   Unlock the subystem.
*/
void
CVMESubsystem::unlock()
{
  if(!m_lockCreated || !m_lockHeld) {
    throw string("Invalid vme lock sequence... unlock either with no lock or lock not held");
  }  
  sembuf unlock = {0, 1,  0};
  semop(m_lockId, &unlock, 1);
  m_lockHeld = false;
  for (int i = 0; i < m_Interfaces.size(); i++) {
    m_Interfaces[i].s_pInterface->onUnlock();
  } 
}

/////////////////
// Utility to create from description or throw.
//
CVMEInterface*
CVMESubsystem::createFromDescription(string description)
{
  CVMEInterface* pInterface = CVMEInterfaceFactory::create(description);
  if (!pInterface) {
    throw CInvalidInterfaceType(description, 
	       string("CVMESubsystem::processDescription Creating an interface"));
  }
  return pInterface;
}

///
// Utility to ensure the semaphore exists.
//

void
CVMESubsystem::EnsureSemaphoreExists()
{
  if (m_lockCreated) return;

  int semkey = 0x564d4520;   // "VME " - compatible with older sems.

  // Retry loop in case anybody makes and then kills it:
  int semid;
  while(1) {
    // Try to get the id of an existing semaphore:

    semid = semget(semkey, 0, 0777); // Try to map:
    if(semid >= 0) break;            // Previously existing!!
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
  m_lockId = semid;
  m_lockCreated = true;
  return;

}
////
void
CVMESubsystem::destroyAll()
{
  while (! m_Interfaces.empty()) {
    Interface& last(m_Interfaces.back());
    if (last.s_mustDelete) {
      delete last.s_pInterface;
    }

    m_Interfaces.pop_back();
  }
}
