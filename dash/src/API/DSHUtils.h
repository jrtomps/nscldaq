#ifndef DAQHWYAPI_DSHUTILS_H
#define DAQHWYAPI_DSHUTILS_H 

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
#include <netinet/in.h>

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

namespace daqhwyapi {

class String;
class StringArray;
class IntArray;
class ByteArray;

/*=====================================================================*/
/**
* @class DSHUtils
* @brief DSHUtils class.
*
* Utilities used by DAQ Super Highway applications.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class DSHUtils : public daqhwyapi::Object {
  public:
    static bool sortFilesByRunNumber(daqhwyapi::StringArray&);
    static bool getDirectoryRunNumbers(daqhwyapi::IntArray&,daqhwyapi::String&);
    static bool getDirectoryRunCTimes(daqhwyapi::IntArray&,daqhwyapi::String&);

    static bool isNumericRange(daqhwyapi::String&);
    static void parsePacketParam(daqhwyapi::String&,uint32_t*);

    static void hexDump(FILE*,daqhwyapi::ByteArray&,int); 

    static void convertEscapeCharacters(daqhwyapi::String&);

  protected:
    DSHUtils();   // Constructor 
};

} // namespace daqhwyapi


#endif
