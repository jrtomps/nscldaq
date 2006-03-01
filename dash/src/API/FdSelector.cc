/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

using namespace std;

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#ifndef DAQHWYAPI_FDSELECTOR_H
#include <dshapi/FdSelector.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

namespace daqhwyapi {
/**
* @var fdselector_rtexception
* @brief Exception to throw for general runtime errors.
*
* Exception to throw for for general runtime errors.
*/
static RuntimeException fdselector_rtexception;

#define __FDSELECTOR_DEFSETSIZ__ 10 
#define __FDSELECTOR_GROWRATE__ 10 
}

using namespace daqhwyapi;

/*===================================================================*/
/** @fn FdSelector::FdSelector()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
FdSelector::FdSelector() {
  rfds = new int[__FDSELECTOR_DEFSETSIZ__];
  rfd_cnt = 0;
  rfd_siz = __FDSELECTOR_DEFSETSIZ__;
  wfds = new int[__FDSELECTOR_DEFSETSIZ__];
  wfd_cnt = 0;
  wfd_siz = __FDSELECTOR_DEFSETSIZ__;
  efds = new int[__FDSELECTOR_DEFSETSIZ__];
  efd_cnt = 0;
  efd_siz = __FDSELECTOR_DEFSETSIZ__;

  for (int i = 0; i < __FDSELECTOR_DEFSETSIZ__; i++) {
    rfds[i] = -1; wfds[i] = -1; efds[i] = -1;
  } 

  // Zero the select() sets
  FD_ZERO(&rfs);
  FD_ZERO(&wfs);
  FD_ZERO(&efs);

  last_selcnt = 0; // Nothing selected yet
}

/*===================================================================*/
/** @fn FdSelector::~FdSelector()
* @brief Destructor.
*                                        
* Destructor.
*                                         
* @param None
* @return None
*/      
FdSelector::~FdSelector() {
  if (rfds != NULL) delete[] rfds; 
  rfds = NULL;
  if (wfds != NULL) delete[] wfds; 
  wfds = NULL;
  if (efds != NULL) delete[] efds; 
  efds = NULL;
}

/*===================================================================*/
/** @fn int FdSelector::select(long long timeout)
* @brief Check for events with a timeout.
*                                        
* Check for events with a timeout.  The timeout is
* specified in microseconds.
*                                         
* @param timeout Timeout in microseconds.
* @return The number of events that have occured.
* @throw RuntimeException On error.
*/      
int FdSelector::select(long long timeout) {
  struct timeval tv;
  struct timeval *tvp = NULL;
  fd_set *rfp = NULL;
  fd_set *wfp = NULL;
  fd_set *efp = NULL;
  int maxfd = 0;
  last_selcnt = 0;  // Nothing selected yet
 
  // For what events do we have descriptors? 
  if (rfd_cnt > 0) rfp = &rfs;
  if (wfd_cnt > 0) wfp = &wfs;
  if (efd_cnt > 0) efp = &efs;

  // Set up the timeout
  if (timeout >= 0) {
    tvp = &tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    if (timeout > 0) {
      unsigned int secs = timeout / 1000000;
      unsigned int usecs = timeout - (secs * 1000000);
      tv.tv_sec = secs;
      tv.tv_usec = usecs;
    }
  }

  // Prepare for read events
  if (rfp != NULL) {
    FD_ZERO(rfp);
    for (int i = 0; i < rfd_cnt; i++) {
      FD_SET(rfds[i],rfp);
      if (rfds[i] > maxfd) maxfd = rfds[i];
    }
  }

  // Prepare for write events
  if (wfp != NULL) {
    FD_ZERO(wfp);
    for (int i = 0; i < wfd_cnt; i++) {
      FD_SET(wfds[i],wfp);
      if (wfds[i] > maxfd) maxfd = wfds[i];
    }
  }

  // Prepare for exceptional events
  if (efp != NULL) {
    FD_ZERO(efp);
    for (int i = 0; i < efd_cnt; i++) {
      FD_SET(efds[i],efp);
      if (efds[i] > maxfd) maxfd = efds[i];
    }
  }

  // And select...
  int rc = ::select(maxfd+1,rfp,wfp,efp,tvp);

  // Got something
  if (rc >= 0) {
    last_selcnt = rc;
    return rc;
  }

  // Whoops!
  if (rc < 0) {
    char buf[DAQHWYAPI_STRERROR_SIZE+1];
    throw fdselector_rtexception.format(CSTR("FdSelector::select() select error: msg=\"%s\" rc=%d"),strerror_r(errno,buf,DAQHWYAPI_STRERROR_SIZE),errno);
  }

  return rc;
}

/*===================================================================*/
/** @fn int FdSelector::select()
* @brief Check for events without a timeout.
*                                        
* Check for events without a timeout.  This method waits until
* an event happens (possibly waiting forever).
*                                         
* @param None
* @return The number of events that have occured.
* @throw RuntimeException On error.
*/      
int FdSelector::select() {
  return select(-1); // Call select without a timeout
}

/*===================================================================*/
/** @fn int FdSelector::poll()
* @brief Poll for events wihout blocking. 
*                                        
* Poll for events without blocking. 
*                                         
* @param None
* @return The number of events that have occured.
* @throw RuntimeException On error.
*/      
int FdSelector::poll() {
  return select(0); // Call select with a zero timeout
}

/*===================================================================*/
/** @fn bool FdSelector::isReadable(int fd)
* @brief Check if an fd is readable.
*                                        
* Check if an fd is readable.  The fd must have been entered 
* using the addReadFd() method and select() or poll() called
* prior to calling this method for this method to determine if
* the fd is readable.
*                                         
* @param fd The fd to check.
* @return True if the fd is readable.
*/      
bool FdSelector::isReadable(int fd) {
  return FD_ISSET(fd,&rfs) ? true : false;
}

/*===================================================================*/
/** @fn bool FdSelector::isWritable(int fd)
* @brief Check if an fd is writable.
*                                        
* Check if an fd is writable.  The fd must have been entered 
* using the addWriteFd() method and select() or poll() called
* prior to calling this method for this method to determine if
* the fd is writable.
*                                         
* @param fd The fd to check.
* @return True if the fd is writable.
*/      
bool FdSelector::isWritable(int fd) {
  return FD_ISSET(fd,&wfs) ? true : false;
}

/*===================================================================*/
/** @fn bool FdSelector::hasException(int fd)
* @brief Check if an fd has an exception.
*                                        
* Check if an fd has an exception.  The fd must have been entered 
* using the addExceptionFd() method and select() or poll() called
* prior to calling this method for this method to determine if
* the fd has an exception.
*                                         
* @param fd The fd to check.
* @return True if the fd has an exception.
*/      
bool FdSelector::hasException(int fd) {
  return FD_ISSET(fd,&efs) ? true : false;
}

/*===================================================================*/
/** @fn void FdSelector::addReadFd(int fd)
* @brief Add a descriptor to watch for read events.
*                                        
* Add a file descriptor to watch for read events.  
*                                         
* @param fd The file descriptor.
* @return None
* @throw RuntimeException If fd is not >= 0 or other failure.
*/      
void FdSelector::addReadFd(int fd) {
  if (fd < 0) throw fdselector_rtexception.format(CSTR("FdSelector::addReadFd: Bad file descriptor %d"),fd);
  int *newfds = add_fd(rfds,fd,rfd_cnt,rfd_siz);
  if (newfds == NULL) {
    throw fdselector_rtexception.format(CSTR("FdSelector::addReadFd: Failed to add file descriptor"));
  } else {
    rfds = newfds;
  }
}

/*===================================================================*/
/** @fn void FdSelector::removeReadFd(int fd)
* @brief Remove a descriptor from those watched for read events.
*                                        
* Remove a file descriptor from those to watch for read events.  
*                                         
* @param fd The file descriptor.
* @return None.
* @throw RuntimeException If fd is not >= 0 or other failure.
*/      
void FdSelector::removeReadFd(int fd) {
  if (fd < 0) throw fdselector_rtexception.format(CSTR("FdSelector::removeReadFd: Bad file descriptor %d"),fd);
  int *newfds = del_fd(rfds,fd,rfd_cnt);
  if (newfds == NULL) {
    throw fdselector_rtexception.format(CSTR("FdSelector::removeReadFd: Failed to remove file descriptor"));
  } else {
    rfds = newfds;
  }
}

/*===================================================================*/
/** @fn void FdSelector::addWriteFd(int fd)
* @brief Add a descriptor to watch for write events.
*                                        
* Add a file descriptor to watch for write events.  
*                                         
* @param fd The file descriptor.
* @return None.
* @throw RuntimeException If fd is not >= 0 or other failure.
*/      
void FdSelector::addWriteFd(int fd) {
  if (fd < 0) throw fdselector_rtexception.format(CSTR("FdSelector::addWriteFd: Bad file descriptor %d"),fd);
  int *newfds = add_fd(wfds,fd,wfd_cnt,wfd_siz);
  if (newfds == NULL) {
    throw fdselector_rtexception.format(CSTR("FdSelector::addWriteFd: Failed to add file descriptor"));
  } else {
    wfds = newfds;
  }
}

/*===================================================================*/
/** @fn void FdSelector::removeWriteFd(int fd)
* @brief Remove a descriptor from those watched for write events.
*                                        
* Remove a file descriptor from those to watch for write events.  
*                                         
* @param fd The file descriptor.
* @return None.
* @throw RuntimeException If fd is not >= 0 or other failure.
*/      
void FdSelector::removeWriteFd(int fd) {
  if (fd < 0) throw fdselector_rtexception.format(CSTR("FdSelector::removeWriteFd: Bad file descriptor %d"),fd);
  int *newfds = del_fd(wfds,fd,wfd_cnt);
  if (newfds == NULL) {
    throw fdselector_rtexception.format(CSTR("FdSelector::removeWriteFd: Failed to remove file descriptor"));
  } else {
    wfds = newfds;
  }
}

/*===================================================================*/
/** @fn void FdSelector::addExceptionFd(int fd)
* @brief Add a descriptor to watch for exceptional events.
*                                        
* Add a file descriptor to watch for exceptional events.  
*                                         
* @param fd The file descriptor.
* @return None.
* @throw RuntimeException If fd is not >= 0 or other failure.
*/      
void FdSelector::addExceptionFd(int fd) {
  if (fd < 0) throw fdselector_rtexception.format(CSTR("FdSelector::addExceptionFd: Bad file descriptor %d"),fd);
  int *newfds = add_fd(efds,fd,efd_cnt,efd_siz);
  if (newfds == NULL) {
    throw fdselector_rtexception.format(CSTR("FdSelector::addExceptionFd: Failed to add file descriptor"));
  } else {
    efds = newfds;
  }
}

/*===================================================================*/
/** @fn void FdSelector::removeExceptionFd(int fd)
* @brief Remove a descriptor from those watched for exceptional events.
*                                        
* Remove a file descriptor from those to watch for exceptional events.  
*                                         
* @param fd The file descriptor.
* @return None.
* @throw RuntimeException If fd is not >= 0 or other failure.
*/      
void FdSelector::removeExceptionFd(int fd) {
  if (fd < 0) throw fdselector_rtexception.format(CSTR("FdSelector::removeExceptionFd: Bad file descriptor %d"),fd);
  int *newfds = del_fd(efds,fd,efd_cnt);
  if (newfds == NULL) {
    throw fdselector_rtexception.format(CSTR("FdSelector::removeExceptionFd: Failed to remove file descriptor"));
  } else {
    efds = newfds;
  }
}

/*===================================================================*/
/** @fn int *FdSelector::grow_set(int *fds,int siz,int cursiz)
* @brief Grow a file descriptor set to a larger size. 
*                                        
* Grow a file descriptor set to a larger size.
*                                         
* @param fds The file descriptor set to grow.
* @param siz The new size.
* @param cursiz The current size.  Updated on return.
* @return A pointer to the new set or NULL on error.
*/      
int *FdSelector::grow_set(int *fds,int siz,int& cursiz) {
  if (siz <= 0) return NULL; 
  if (siz <= cursiz) return fds;

  int *newfds = new int[siz];
  for (int i = 0; i < siz; i++) {
    if ((i < cursiz)&&(fds != NULL)) newfds[i] = fds[i];
    else newfds[i] = -1; 
  }

  if (fds != NULL) delete[] fds;
  cursiz = siz;
  return newfds;
}

/*===================================================================*/
/** @fn int *FdSelector::add_fd(int *fds,int fd,int& cmax,int& msiz)
* @brief Add a file descriptor to a set.
*                                        
* Add a file descriptor to a set of file descriptors.
*                                         
* @param fds The file descriptor set.
* @param fd The new file descriptor.
* @param cmax The current max usage of fds; updated on return.
* @param msiz Current size of the fd set; changed if set is grown.
* @return A pointer to the fd set or NULL on error.
*/      
int *FdSelector::add_fd(int *fds,int fd,int& cmax,int& msiz) {
  if (fd < 0) return NULL;  // Not a valid descriptor.
  if ((cmax < 0)||(msiz < 0)) return NULL; // Not a valid maximum.
  if (fds == NULL) return NULL; // Not a valid set.

  int *newfds = fds;
  int siz = msiz;
  if (cmax >= msiz) { // Need to grow
    newfds = grow_set(fds,siz + __FDSELECTOR_GROWRATE__,msiz); 
    if (newfds == NULL) return NULL;
  } 

  int idx = cmax;
  for (int i = 0; i < cmax; i++) {
    if (newfds[i] == fd) { // Already have this one
      idx = -1;
      break;
    }
  } 

  if (idx >= 0) {  // Add it in
    newfds[cmax] = fd;
    cmax++;
  }

  return newfds;
}

/*===================================================================*/
/** @fn int *FdSelector::del_fd(int *fds,int fd,int& cmax)
* @brief Remove a file descriptor from a set.
*                                        
* Remove a file descriptor from a set of file descriptors.
*                                         
* @param fds The file descriptor set.
* @param fd The file descriptor to remove.
* @param cmax The current max usage of fds; updated on return.
* @return A pointer to the fd set or NULL on error.
*/      
int *FdSelector::del_fd(int *fds,int fd,int& cmax) {
  if (fd < 0) return NULL;  // Not a valid descriptor.
  if (cmax < 0) return NULL; // Not a valid maximum.
  if (fds == NULL) return NULL; // Not a valid set.

  if (cmax == 0) return fds;

  int idx = -1;
  for (int i = 0; i < cmax; i++) {
    if (fds[i] == fd) { // Found it
      idx = i;
      break;
    }
  } 

  if (idx >= 0) {  // Remove it
    for (int i = idx; i < (cmax-1); i++) fds[cmax] = fds[i+1];
    fds[cmax-1] = -1;
    cmax--;
  }

  return fds;
}
