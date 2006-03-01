#ifndef DAQHWYAPI_PROCESS_H
#define DAQHWYAPI_PROCESS_H

/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

#ifndef __DAQHWYAPI_CONFIG_H
#include <dshapi/daqconfig.h>
#endif

#ifndef DAQHWYAPI_CSTR_H
#include <dshapi/cstr.h>
#endif

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_RUNNABLE_H
#include <dshapi/Runnable.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#define _USE_BSD
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <iostream>
#include <stdexcept>

namespace daqhwyapi {

/**
* @class Process
* @brief Basic Unix style process class.
*
* A class that reprsents a basic Unix style execution process.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class Process : public Runnable {
  public: 
    Process();                       // Constructor
    virtual ~Process();              // Destructor
    long getId();                    // Return the pid
    void setName(const String&);     // Set the process name and title
    const String& getName() const;   // Get the process name
    void start();                    // Start this process
    void start_main();               // Start without forking
    virtual void run() = 0;    

    static void setTitle(const String&); // 

  protected: 
    void daemonize(bool = true,bool = true,bool = true); // Daemonize
    long waitForAnyChild(bool); // Wait for any child process
    long waitForChild(long,bool); // Wait for a child process

  private:
    String processname;
    long my_id;
};

} // namespace daqhwyapi

#endif
