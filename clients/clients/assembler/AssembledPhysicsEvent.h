#ifndef __ASSEMBLEDPHYSICSEVENT_H
#define __ASSEMBELDPHYSICSEVENT_H
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

/*!
   Assembled physics event objects contain data that is
   the results of an assembly of a bunch of physics event fragments
   from the participating nodes.  As such, the originating node is always
   0 indicating that the data are an assembled event, and,
   of course the type is physics data.
*/
class AssembledPhysicsEvent : public AssembledEvent
{
private:
  std::vector<unsigned short>    m_body; //!< Body of the event.

public:
  AssembledPhysicsEvent();

  void addData(void* pData,
	       size_t wordCount);

  void copyOut(void* pTarget) const;
 
  size_t size() const;
  std::vector<unsigned short>::iterator begin();
  std::vector<unsigned short>::iterator end();
  unsigned short& operator[](unsigned int index);

};


#endif
