#ifndef __STATETRANSTIONASSEMBLYEVENT_H
#define __STATETRANSTIONASSEMBLYEVENT_H
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
#endif

#ifndef __STL_LIST
#include <list>
#ifndef __STL_LIST
#define __STL_LIST
#endif
#endif



class StateTransitionFragment;
class AssembledEvent;

class StateTransitionAssemblyEvent : public AssemblyEvent
{
public:
	typedef std::list<StateTransitionFragment*> FragmentList;
	typedef FragmentList::iterator   FragmentListIterator;
private:
FragmentList  m_fragments;
public:
	StateTransitionAssemblyEvent(StateTransitionFragment& firstFragment,
				     time_t                   receivedTime);
	virtual ~StateTransitionAssemblyEvent();
private:
	StateTransitionAssemblyEvent(const StateTransitionAssemblyEvent& rhs);
	StateTransitionAssemblyEvent& operator=(const StateTransitionAssemblyEvent& rhs);
	int operator==(const StateTransitionAssemblyEvent& rhs) const;
	int operator!=(const StateTransitionAssemblyEvent& rhs) const;
public:
	// Virtual functions that are implemented:
	
	virtual bool isPhysics() const;
	virtual void add(EventFragment& frag);
	virtual AssembledEvent* assembledEvent();	
	virtual uint16_t type() const;
};

#endif /*STATETRANSTIONASSEMBLYEVENT_H_*/
