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
#include <setjmp.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <string>

#ifndef DAQHWYAPI_MAIN_H
#include <dshapi/Main.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

extern char **environ;

using namespace daqhwyapi;

namespace daqhwyapi {
  extern int    daqhwyapi_argc;
  extern char **daqhwyapi_argv;
  extern char **daqhwyapi_envp;
  extern int    daqhwyapi_envspace;
  extern Main *__daqhwyapi_mainsystem__;
}


/**
* @class Main
* @brief Main process.
*
* Main process.
*
* @author  Eric Kasten
* @version 1.0.0
*/

/*===================================================================*/
/** @fn Main::Main()
* @brief Constructor.
*                                        
* Basic constructor method for this class. 
*                                         
* @param None
* @return this                 
*/      
Main::Main() 
{
  daqhwyapi::__daqhwyapi_mainsystem__ = this;
}

/*===================================================================*/
/** @fn Main::~Main()
* @brief Destructor.
*                                        
* Basic destructor method for this class. 
*                                         
* @param None
* @return this                 
*/      
Main::~Main()
{
  daqhwyapi::__daqhwyapi_mainsystem__ = NULL;
}

/*===================================================================*/
/** @fn void Main::boot(int argc,char **argv,char **envp)
* @brief Bootstrap this process.
*                                        
* Bootstrap this process.  
*                                         
* @param argc Number of command line parameters.
* @param argv Array of command line parameters.
* @param envp Process environment array.
* @return None
*/      
void Main::boot(int argc,char **argv,char **envp)
{
  daqhwyapi::daqhwyapi_argc = argc;
  daqhwyapi::daqhwyapi_argv = argv; 
  daqhwyapi::daqhwyapi_envp = envp; 
  daqhwyapi::daqhwyapi_envspace = moveEnviron(envp);
}

/*===================================================================*/
/** @fn void Main::run()
* @brief Run the main process.
*                                        
* Run the main process.
*                                         
* @param None
* @return None
*/      
void Main::run()
{
  this->main(daqhwyapi::daqhwyapi_argc,daqhwyapi::daqhwyapi_argv); 
}

/*===================================================================*/
/** @fn int Main::moveEnviron(char **envp)
* @brief Safely move the environment.
*                                        
* Move the environment for safety and so we can use the space to
* the process title. 
*                                         
* @param envp Process environment array.
* @return None
*/      
int Main::moveEnviron(char **envp)
{
  char *ptr;
  register int i;

  // Move the environment
  for (i = 0; envp[i] != NULL; i++);
  environ = new char*[i + 1];

  for (i = 0; envp[i] != NULL; i++) { 
    int sz =strlen(envp[i])+1;
    environ[i] = new char[sz];
    memcpy(environ[i],envp[i],sz);
  }
  environ[i] = NULL;

#ifndef NO_SET_PROCNAME
  // Calculate the available space for process title manipulation
  if ((daqhwyapi::daqhwyapi_argc < 1)||(daqhwyapi::daqhwyapi_argv == NULL)) return(0);

  ptr = daqhwyapi::daqhwyapi_argv[0] + strlen(daqhwyapi::daqhwyapi_argv[0]);

  for (i = 1; i < daqhwyapi::daqhwyapi_argc; i++) {
    if (daqhwyapi::daqhwyapi_argv[i] == ptr + 1) ptr += strlen(++ptr);
  }

  for (i = 1; i < daqhwyapi::daqhwyapi_argc; i++) {
    if (daqhwyapi::daqhwyapi_argv[i] == ptr + 1) ptr += strlen(++ptr);
  }

  if (daqhwyapi::daqhwyapi_envp[0] == ptr + 1) {
    for (i = 0; daqhwyapi::daqhwyapi_envp[i]; i++) {
      if (daqhwyapi::daqhwyapi_envp[i] == ptr + 1) ptr += strlen(++ptr);
    }
  }

  return(ptr - daqhwyapi::daqhwyapi_argv[0]); 
#else
  return(0);
#endif
}

