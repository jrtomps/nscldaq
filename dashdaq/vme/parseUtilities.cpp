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

#include <iostream>

namespace descriptionFile
{
  string whitespace        = " \t"; // Defines whitespace chars.

 
  /*!
      firstWord returns the first word from a string of text.
      The first word is defined as the set of non-whitespace
      characters prior to the first whitespace character.
      Note that in many cases, you should first trim leading
      whitespace with stripLeadingBlanks before calling this,
      or you may get an empty word if there is leading whitespace.
      In this case whitespace is defined by isspace().

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
  /*!
     stripLeadingBlanks returns the a string that is the input
     string with leading characters in 
     descriptionFile::whitespace removed.

     \param line : string [in]
        Input string to strip.
     \return string
  */
  string stripLeadingBlanks(string line)
  {
    size_t firstNon = line.find_first_not_of(whitespace);
    if (firstNon != std::string::npos) {
      return line.substr(firstNon);
    } 
    else {			// No match.
      return line;
    }
  }
  /*!
    stripComment strips comments from a line.
    a comment is considered to be introduced by
    commentIntroducer ..which is a char and continues
    to the end of the line.  This function therefore locates
    the first instance of commentIntroducer, and strips it and
    the remainder of the string from the string.
    \param line : string [in]
         The line to strip.
    \param commentIntroducer : string [in]
         The string that introduces the comment.
    \return string
    \retval the part of the string prior to the first commentIntroducer.
    
  */
  string stripComment(string line, string commentIntroducer)
  {
    size_t commentLeader = line.find(commentIntroducer);
    if (commentLeader == std::string::npos) {
      commentLeader = line.size();
    }
    return line.substr(0, commentLeader);
  }
  /*!
    stripTralingBlanks strips trailing characters that are in 
    whitespace from a string.
    \param line : string [in]
       The line to strip.
    \return string
    \retval The stripped line.

  */
  string stripTrailingBlanks(string line)
  {
    size_t lastNon = line.find_last_not_of(whitespace);
    if (lastNon == std::string::npos) {
      return string("");
    }
    return line.substr(0, lastNon+1);

  }



}
