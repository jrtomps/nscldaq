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
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <limits.h>

#ifndef DAQHWYAPI_STRINGUTILS_H
#include <dshapi/StringUtils.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

namespace daqhwyapi {
/**
* @var integer_format_exception
* @brief Exception to throw for conversion errors.
*
* Exception to throw for conversion errors.
*/
static NumberFormatException stringutils_format_exception;
} // namespace daqhwyapi

using namespace daqhwyapi;

/*===================================================================*/
/** @fn StringUtils::StringUtils()
* @brief Default constructor.
*                                        
* Default concstructor.
*                                         
* @param None
* @return this
*/      
StringUtils::StringUtils() { }

/*==============================================================*/
/** @fn int parseInteger(String& aStr)
* @brief Parse a String into an integer.
*
* Parse a String into an integer.
*
* @param aStr The string to parse.
* @return The integer value of the string.
* @throw NumberFormatException If the string cannot be converted to an int.
*/
int StringUtils::parseInteger(String& aStr) {
  char *nptr = (char*)(aStr.c_str());
  char *endptr = (char*)nptr;
  int ival = strtol(nptr,&endptr,0);

  if (ival == 0) {
    if (nptr == endptr) { // No conversion
      throw stringutils_format_exception.format(CSTR("StringUtils::parseInteger() Could not format the String(%s) as an integer"),aStr.c_str());
    }
  }

  return ival;
}

/*==============================================================*/
/** @fn float parseFloat(String& aStr)
* @brief Parse a String into a float.
*
* Parse a String into a float.
*
* @param aStr The string to parse.
* @return The float value of the string.
* @throw NumberFormatException If the string cannot be converted to a float.
*/
float StringUtils::parseFloat(String& aStr) {
  char *nptr = (char*)(aStr.c_str());
  char *endptr = (char*)nptr;
  float fval = strtof(nptr,&endptr);

  if (fval == 0) {
    if (nptr == endptr) { // No conversion
      throw stringutils_format_exception.format(CSTR("StringUtils::parseFloat() Could not format the String(%s) as a float"),aStr.c_str());
    }
  }

  return fval;
}

/*==============================================================*/
/** @fn bool parseBoolean(String& aStr)
* @brief Parse a String into a bool.
*
* Parse a String into a bool.
*
* @param aStr The string to parse.
* @return The bool value of the string.
* @throw NumberFormatException If the string cannot be parsed as a boolean.
*/
bool StringUtils::parseBoolean(String& aStr) {
  bool bval = false;
  String tstr("true");
  String fstr("false");
  String ystr("yes");
  String nstr("no");
  if (tstr.equalsIgnoreCase(aStr)) bval = true;
  else if (ystr.equalsIgnoreCase(aStr)) bval = true;
  else if (fstr.equalsIgnoreCase(aStr)) bval = false;
  else if (nstr.equalsIgnoreCase(aStr)) bval = false;
  else throw stringutils_format_exception.format(CSTR("StringUtils::parseBoolean() Boolean::getBoolean() \"%s\" cannot be parsed as a boolean string"),aStr.c_str());
  return bval;
}

