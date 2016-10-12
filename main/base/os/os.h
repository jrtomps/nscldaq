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

#ifndef OS_H
#define OS_H


#include <string>
#include <vector>

// Note we use this to get a definition of useconds_t
// This may need to be typedef'd here once that' gets yanked
// out (usleep that uses it is POSIX deprecated).


#include <unistd.h>

/**
 * Static methods that encapsulate operating system calls.
 */
class Os {
public:
  static std::string whoami();		//< Logged in userm
  static bool authenticateUser(std::string sUser, std::string sPassword);
  static int  usleep(useconds_t usec);
  static int  blockSignal(int sigNum);
  static int  checkStatus(int status, int checkStatus, std::string msg);
  static int  checkNegativeStatus(int returnCode);
  static std::string  getfqdn(const char* host);
  static std::vector<std::string> getProcessCommand(pid_t pid);
  static std::string hostname();
};

#endif
