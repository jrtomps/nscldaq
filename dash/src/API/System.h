#ifndef DAQHWYAPI_SYSTEM_H
#define DAQHWYAPI_SYSTEM_H 

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
#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include <sstream>
#include <string>

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

namespace daqhwyapi {

class String;

/*=====================================================================*/
/**
* @class SystemBase
* @brief SystemBase class.
*
* Basic system functions and objects.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class SystemBase : public Object {
  public:
    ~SystemBase(); // Destructor
    static SystemBase& instance();
    unsigned long currentTimeMillis();
    void attachDebugger(const char*,int);
    void executeProgram(String&,String&);

  protected:
    SystemBase();   // Constructor 
};

extern SystemBase& System;

} // namespace daqhwyapi


#endif
