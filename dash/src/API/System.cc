/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include <sys/timeb.h>

#ifndef DAQHWYAPI_SYSTEM_H
#include <dshapi/System.h>
#endif

#ifndef DAQHWYAPI_EXCEPTIONS_H
#include <dshapi/Exceptions.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#ifndef DAQHWYAPI_STRINGTOKENIZER_H
#include <dshapi/StringTokenizer.h>
#endif

// Needed for the exec funcion family
extern char **environ;

namespace daqhwyapi {
/**
* @var system_runtime_exception
* @brief Exception to throw for runtime errors.
*
* Exception to throw for runtime errors.
*/
static RuntimeException system_runtime_exception;
} // namespace daqhwyapi

using namespace daqhwyapi;

/*===================================================================*/
/** @fn SystemBase::SystemBase()
* @brief Default constructor.
*                                        
* Default concstructor.
*                                         
* @param None
* @return this
*/      
SystemBase::SystemBase() {
}

/*===================================================================*/
/** @fn SystemBase::~SystemBase()
* @brief Destructor.
*                                        
* Destroy this object.
*                                         
* @param None
* @return None
*/      
SystemBase::~SystemBase() {
}

/*===================================================================*/
/** @fn SystemBase& SystemBase::instance()
* @brief Get an instance of this object.  
*                                        
* Get an instance of this object.  Implementation of the
* singleton pattern.
*                                         
* @param None
* @return An instance of this object.
*/      
SystemBase& SystemBase::instance() { 
  static SystemBase *systhis = NULL;
  if (systhis == NULL) systhis = new SystemBase;
  return(*systhis); 
}

/*===================================================================*/
/** @fn unsigned long SystemBase::currentTimeMillis()
* @brief Get the current time in milliseconds.
*                                        
* Get the current time in milliseconds.
*                                         
* @param None
* @return The current time in milliseconds.
*/      
unsigned long SystemBase::currentTimeMillis() {
  struct timeb tp;
  ftime(&tp);
  unsigned long millis = (tp.time * 1000) + tp.millitm;
  return millis;
}


/*==============================================================*/
/** @fn void attachDebugger(const char *iam,int secs)
* @brief Issue a debug message and delay. 
*
* Issue a debug message and delay a specified number of
* seconds before continuing.
*
* @param iam The process name.
* @param secs The number of seconds to delay.
* @return None
*/
void SystemBase::attachDebugger(const char *iam,int secs) {
  cerr << "In a separate terminal window, attach a debugger to " << iam << " pid=" << getpid() << " (eg, gdb " << iam << " " << getpid() << ") within " << secs << " seconds." << endl;

  for (int i = secs; i > 0; i--) {
    cerr << "\r" << "                 " << "\r" << i << ends;
    sleep(1);
  }

  cerr << "\rContinuing." << endl;
}

/*==============================================================*/
/** @fn void executeProgram(String& file,String& args)
* @brief Execute a program.
*
* Replace the current process image with a new one.  That is,
* the the current running process will be replaced with a new
* execution of the program specified by path.
*
* @param file The program to execute.
* @param args The command line to pass to the program.
* @return None
* @throw RuntimeException If there's an error starting the new program.
*/
void SystemBase::executeProgram(String& file,String& args) {
  StringTokenizer toker(args);
  int tcnt = toker.countTokens();

  char *argarry[tcnt+2]; // 1 for the file and 1 for a NULL

  // Position 0 contains the file name
  int sz = file.size();
  argarry[0] = new char[sz+1];  
  strncpy(argarry[0],file.c_str(),sz+1); 
  argarry[1] = NULL;
 
  // Construct the argument array 
  if (tcnt > 0) {
    for (int i = 1; i < tcnt+2; i++) argarry[i] = NULL;

    for (int i = 1; i <= tcnt; i++) {
      String s = toker.nextToken();
      sz = s.size();
      if (sz > 0) {
        argarry[i] = new char[sz+1];
        strncpy(argarry[i],s.c_str(),sz+1); 
      } 
    }
  } 

  // Execute the program
  int rc = ::execvp(file.c_str(),argarry); 

  // Woops!
  if (rc < 0) {
    int eno = errno;
    char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
    throw system_runtime_exception.format(CSTR("System::executeProgram() msg=\"%s\" rc=%d"),strerror_r(eno,ebuf,DAQHWYAPI_STRERROR_SIZE),eno);
  }
}

namespace daqhwyapi {
  SystemBase& System = SystemBase::instance();
} // namespace daqhwyapi
