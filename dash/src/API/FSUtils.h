#ifndef DAQHWYAPI_FSUTILS_H
#define DAQHWYAPI_FSUTILS_H 

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
class StringArray;

/*=====================================================================*/
/**
* @class FSUtils
* @brief FSUtils class.
*
* Basic filesystem utility functions.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class FSUtils : public daqhwyapi::Object {
  public:
    static bool pathExists(daqhwyapi::String&);
    static bool isaDirectory(daqhwyapi::String&);
    static off_t fileSize(daqhwyapi::String&);
    static off_t fileSize(int);
    static time_t fileCTime(daqhwyapi::String&);
    static time_t fileCTime(int);
    static void directoryList(StringArray&,String&);
    static void directoryList(StringArray&,String&,String&);

  protected:
    FSUtils();   // Constructor 
};

} // namespace daqhwyapi


#endif
