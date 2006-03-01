#ifndef DAQHWYAPI_STRINGUTILS_H
#define DAQHWYAPI_STRINGUTILS_H 

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
* @class StringUtils
* @brief StringUtils class.
*
* Basic String parsing utility functions.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class StringUtils : public daqhwyapi::Object {
  public:
    static int parseInteger(daqhwyapi::String&);
    static float parseFloat(daqhwyapi::String&);
    static bool parseBoolean(daqhwyapi::String&);

  protected:
    StringUtils();   // Constructor 
};

} // namespace daqhwyapi


#endif
