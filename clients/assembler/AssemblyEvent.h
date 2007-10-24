#ifndef __ASSEMBLYEVENT_H
#define __ASSEMBLYEVENT_H
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

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

class NodeScoreboard;
class EventFragment;
class AssembledEvent;

/*!
 *   Abstract base class for events that are in the process
 * of being assembled.  The common functionality of maintaining the trigger timestamp
 * and the node scoreboard are all handled by this class, as well as the provision of 
 * a common set of event type independent interfaces for assembly.
 */
class AssemblyEvent
{
private:
	uint32_t 			m_triggerTime;
	NodeScoreboard&		m_scoreboard;
public:
	
	// Canonicals:
	
	AssemblyEvent(uint32_t triggerTime);
	virtual ~AssemblyEvent();
private:
	AssemblyEvent(const AssemblyEvent& rhs);
	AssemblyEvent& operator=(const AssemblyEvent& rhs);
	int operator==(const AssemblyEvent& rhs);
	int operator!=(const Assembly Event& rhs);
public:
	
	// class operations:
	
	uint32_t timestamp() const;
	bool     isComplete() const;
	
	// Pure Virtual Interface:
	
	virtual bool isPhysics() const = 0;
	virtual      add(EventFragment& frag) = 0;
	virtual AssembledEvent* assembledEvent()  = 0;
	virtual uint16_t type() const= 0;
};

#endif /*ASSEMBLYEVENT_H_*/
