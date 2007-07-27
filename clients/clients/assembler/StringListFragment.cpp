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
#include "StringListFragment.h"

#include <buffer.h>

using namespace std;

/*!
  Construct the buffer... 
*/
StringListFragment::StringListFragment(uint16_t* pBuffer) :
  EventFragment(extractNode(pBuffer),
		extractType(pBuffer),
		bodyPointer(pBuffer),
		extractSize(pBuffer) - sizeof(bheader)/sizeof(uint16_t)),

{
  uint16_t entities = extractEntityCount(pBuffer);
  pBuffer          += sizeof(bheader)/sizeof(uint16_t);
  int item=0;
  while(entities) {
    m_strings.push_back(string(pBuffer));

    // Strings are an even number of words hence the +1 below.

    pBuffer += (m_strings[item].size() + 1)/sizeof(uint16_t);

    item++;
    entities--;
  }

}
/*!
   Return a copy of the vector of strings:
*/
vector<string>
StringListFragment::strings() const
{
  returnm_strings;
}
