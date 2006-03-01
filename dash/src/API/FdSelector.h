#ifndef DAQHWYAPI_FDSELECTOR_H
#define DAQHWYAPI_FDSELECTOR_H

/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_SELECTOR_H
#include <dshapi/Selector.h>
#endif

namespace daqhwyapi {

/*=====================================================================*/
/**
* @class FdSelector
* @brief Basic selector interface.
*
* Basic selector interface for implementing selection operations
* on sets of file descriptors or other objects or primitives.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class FdSelector : public Selector {
  public:
    FdSelector(); // Constructor
    virtual ~FdSelector(); // Destructor
    int select();          // Wait forever
    int poll();          // Check without waiting
    int select(long long); // With time in microseconds
    void addWriteFd(int);  // Add an fd to watch for write events
    void removeWriteFd(int);  // Add an fd to watch for write events
    void addReadFd(int);  // Add an fd to watch for read events
    void removeReadFd(int);  // Add an fd to watch for read events
    void addExceptionFd(int);  // Add an fd to watch for exceptions
    void removeExceptionFd(int);  // Add an fd to watch for exceptions
    bool isReadable(int); // Check if fd is readable
    bool isWritable(int); // Check if fd is writable 
    bool hasException(int); // Check if an exception has occured

  private:
    int *grow_set(int*,int,int&);
    int *add_fd(int*,int,int&,int&);
    int *del_fd(int*,int,int&);

    int *rfds;
    int rfd_cnt;
    int rfd_siz;
    int *wfds;
    int wfd_cnt;
    int wfd_siz;
    int *efds;
    int efd_cnt;
    int efd_siz;

    int last_selcnt;
    fd_set rfs;
    fd_set wfs;
    fd_set efs;
};

} // namespace daqhwyapi

#endif
