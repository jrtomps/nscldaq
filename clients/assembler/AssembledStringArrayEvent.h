#ifndef __ASSEMBLEDSTRINGARRAYEVENT_H
#define __ASSEMBLEDSTRINGARRAYEVENT_H
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



#ifndef __ASSEMBLEDEVENT_H
#include "AssembledEvent.h"
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __CRT_TIME_H
#include <time.h>
#ifndef __CRT_TIME_H
#define __CRT_TIME_H
#endif
#endif

/*!
   Encapsulates string array events.  These are events that
  carry arbitrary documentation information. They are passed
  without assembly, and therefore with their original node ids intact.
  They are not barriers to event assembly, in the sense that event assembly
  can occur across them...nor do they cause partial event data to be 
  flushed to file.
*/
class AssembledStringArrayEvent : public AssembledEvent
{
private:
  std::vector<std::string>    m_strings;

public:
  AssembledStringArrayEvent(unsigned short             node,
			    AssembledEvent::BufferType type);

  
  void addString(const char* pString);
  void addString(std::string item);
  void addStrings(std::vector<std::string> strings);
  std::vector<std::string> getStrings() const;

  size_t size() const;
  std::vector<std::string>::iterator begin();
  std::vector<std::string>::iterator end();
  std::string& operator[](unsigned int index);
  
};

#endif
