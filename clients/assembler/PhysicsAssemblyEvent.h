#ifndef __PHYSICSASSEMBLYEVENT_H
#define __PHYSICSASSEMBLYEVENT_H
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

#ifndef __ASSEMBLYEVENT_H
#include "AssemblyEvent.h"

#ifndef __STL_LIST
#include <list>
#ifndef __STL_LIST
#define __STL_LIST
#endif
#endif



class PhysicsFragment;


class PhysicsAssemblyEvent : public AssemblyEvent
{
public:
	typedef std::list<PhysicsFragment*> FragmentList;
	typedef FragmentList::iterator      FragmentListIterator;
private:
	FragmentList    m_fragments;
public:
	// Canonicals
	
	PhysicsAssemblyEvent(PhysicsFragment* pFirstFragment);
	virtual ~PhysicsAssemblyEvent();
private:
	PhysicsAssemblyEvent(const PhysicsAssemblyEvent&);
	PhysicsAssemblyEvent& operator=(const PhysicsAssemblyEvent&);
	int operator==(const PhysicsAssemblyEvent&) const;
	int operator!=(const PhysicsAssemblyEvent&) const;
public:
	
	// virtual overrides:
	
	virtual bool isPhysics() const;
	virtual      add(EventFragment& frag);
	virtual AssembledEvent* assembledEvent();
	virtual uint16_t type() const;
	
	
};

#endif /*PHYSICSASSEMBLYEVENT_H_*/
