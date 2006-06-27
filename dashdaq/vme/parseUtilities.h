/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __PARSEUTILITIES_H
#define __PARSEUTILITIES_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STDCPP_IOSTREAM
#include <iostream>
#ifndef __STDCPP_IOSTREAM
#define __STDCPP_IOSTREAM
#endif
#endif


/*!
   This file contains code that assists in parsing
   description files.  This code is common enough
   in use and utility it does not belong in any
   single class.  It will, however get its own
   namespace in order to prevent 'namespace pollution'.
  
*/

namespace descriptionFile {
  extern std::string whitespace;

  std::string firstWord(std::string line);
  std::string stripLeadingBlanks(std::string line);
  std::string stripComment(std::string line,
			   std::string commentIntroducer = std::string("#"));
  std::string stripTrailingBlanks(std::string line);
  std::string getLine(std::istream& str);
  
  std::pair<std::string, std::string> getKeywordValue(std::string& line);
}

#endif
