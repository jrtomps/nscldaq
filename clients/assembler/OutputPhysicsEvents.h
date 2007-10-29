#ifndef __OUTPUTPHYSICSEVENTS_H
#define __OUTPUTPHYSICSEVENTS_H
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

#ifndef __STL_LIST
#include <list>
#ifndef __STL_LIST
#define __STL_LIST
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

class AssemblyEvent;
class EventFragment;


/*!
 * This class is a collection of events that are under assembly.
 * Really we hold both state transitions _and_ physicss events.
 * Events are stored in a list and appended to the end of the list.
 * A quick justification of why that's probably ok with respect to performance;
 * When everything is working, everything comes in sequentially, in monotonicially
 * increasing time.  Most of the action should be at the beginning of the list.
 * or the beginning of the residual of the list after the previous search.
 * Therefore linear searches shoud be quite short.
 */
class OutputPhysicsEvents
{
public:
	typedef std::list<AssemblyEvent*>   AssemblyList;
	typedef AssemblyList::iterator      iterator;
	
private:
	AssemblyList  m_assemblies;
public:
		// Canonicals
	
	OutputPhysicsEvents();
	virtual ~OutputPhysicsEvents();
private:
	OutputPhysicsEvents(const OutputPhysicsEvents&);
	OutputPhysicsEvents& operator=(const OutputPhysicsEvents&);
	int operator==(const OutputPhysicsEvents&) const;
	int operator!=(const OutputPhysicsEvents&) const;
public:
	
		// Object operations:
	
	void add(AssemblyEvent& event);
	AssemblyEvent* remove(iterator where);
	iterator begin();
	iterator end();
	size_t size() const;
	iterator findByType(uint16_t type);
	iterator findByType(uint16_t type, iterator start, iterator stop);
	iterator findPhysByWindow(uint32_t windowStart, uint32_t windowStop);
	iterator findPhysByWindow(uint32_t windowStart, uint32_t windowStop,
							iterator start, iterator stop);
	AssemblyList removePrior(iterator stop);
	AssemblyList remove(iterator start, iterator stop);
	AssemblyEvent* removeItem(iterator item);
	size_t countPrior(iterator here);
	size_t countRange(iterator start, iterator stop);

};

// Exported for testing

typedef struct _TimeWindow {
	uint32_t start;
	uint32_t stop;
} TimeWindow, *pTimeWindow;


extern bool
matchType(AssemblyEvent* p, uint16_t type);

extern inline bool
between(uint32_t value, uint32_t low, uint32_t high);


extern bool
matchWindow(AssemblyEvent* p, pTimeWindow window);

#endif /*OUTPUTPHYSICSEVENTS_H_*/
