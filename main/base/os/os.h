/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __OS_H
#define __OS_H


#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

// Note we use this to get a definition of useconds_t
// This may need to be typedef'd here once that' gets yanked
// out (usleep that uses it is POSIX deprecated).


#ifndef __CRT_UNISTD_H
#include <unistd.h>
#ifndef __CRT_UNISTD_H
#define __CRT_UNISTD_H
#endif
#endif

/**
 * Static methods that encapsulate operating system calls.
 */
class Os {
public:
  static std::string whoami();		//< Logged in userm
  static bool authenticateUser(std::string sUser, std::string sPassword);
  static int  usleep(useconds_t usec);
  static int  blockSignal(int sigNum);
};

#endif
