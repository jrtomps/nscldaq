#ifndef __STRINGLISTFRAGMENT_H
#define __STRINGLISTFRAGMENT_H

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

#ifndef __EVENTFRAGMENT_H
#include "EventFragment.h"
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

/*!
   Class that encapsulates the various sort of string
   list event fragments.  We'll do an up-front fetch of
   the strings into an internal vector as that's probably simplest.
... and space is cheap.
*/
class StringListFragment : public EventFragment
{
private:
  std::vector<std::string> m_strings;
public:
  StringListFragment(uint16_t* pBuffer);
  

  std::vector<std::string> strings() const;

};


#endif
