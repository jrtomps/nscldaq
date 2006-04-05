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

#include <config.h>
#include "parseUtilities.h"

#include <ctype.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

namespace descriptionFile
{

  /*!
      firstWord returns the first word from a string of text.
      The first word is defined as the set of non-whitespace
      characters prior to the first whitespace character.
      Note that in many cases, you should first trim leading
      whitespace with stripLeadingBlanks before calling this,
      or you may get an empty word if there is leading whitespace.

      \param line : string[in]
         Line of text from which to return the leading whitespace.
      \return string
      \retval  See the function description above.

  */
  string
  firstWord(string line) 
  {

    // Copy to result string all chars until end of string or 
    // first whitespace.

    string result;
    for (int i =0; i < line.size(); i++) {
      char c = line[i];
      if(isblank(c)) break;
      result += c;
      
    }
    return result;
  }

}
