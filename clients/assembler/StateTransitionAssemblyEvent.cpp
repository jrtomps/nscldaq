
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
#include "StateTransitionAssemblyEvent.h"
#include "StateTransitionFragment.h"
#include "AssembledStateTransitionEvent.h"
#include <buftypes.h>

using namespace std;

////////////////////////////////////////////////////////////
/*!
 *   Createing the event just means setting the timestamp to
 *   0 as the concept of a timestamp does not exist for state 
 *   transition events.
 */
StateTransitionAssemblyEvent::StateTransitionAssemblyEvent(StateTransitionFragment& firstFragment) :
	AssemblyEvent(0)
{
	add(firstFragment);
}
///////////////////////////////////////////////////////////
/*!
 *   Destroying the event will mean destroying the fragments that
 * make it up.
 */
StateTransitionAssemblyEvent::~StateTransitionAssemblyEvent()
{
  FragmentListIterator i = m_fragments.begin();
  while (i != m_fragments.end()) {
    delete *i;
    i++;
	}
  // The list knows how to clean itself up from here.

}
//////////////////////////////////////////////////////////////
/*!
 * \return bool
 * \retval false as the event is not a physics event.
 */
bool
StateTransitionAssemblyEvent::isPhysics() const
{
	return false;
}
///////////////////////////////////////////////////////////////
/*!
 * Add an event to the fragment.
 */
void
StateTransitionAssemblyEvent::add(EventFragment& frag)
{
  // Errors in the underlying fragment type of frag will throw an exception
  // below; where just casting to a pointer would not.
  
  StateTransitionFragment& 
    transitionFragment(reinterpret_cast<StateTransitionFragment&>(frag));
  
  // Extract the node add it to the scoreboard and add the fragment to our  list.
  
  
  addNode(transitionFragment.node());
  m_fragments.push_back(&transitionFragment);
}
///////////////////////////////////////////////////////////////
/*!
 *   Create the assembled event.  In this case the information
 * just comes from the first fragment.
 * \return AssembledEvent*
 * \retval A dynamically allocated assembled event that was built
 *         from the event fragments that made up this event.
 * 
 */
AssembledEvent* 
StateTransitionAssemblyEvent::assembledEvent()
{
	StateTransitionFragment* pFirstFragment = m_fragments.front();
	AssembledEvent::BufferType type;
	
	// Need to map the fragment type to a buffer type:
	switch (pFirstFragment->type()) {
	case BEGRUNBF:
		type = AssembledEvent::BeginRun;
		break;		
	case ENDRUNBF:
		type = AssembledEvent::EndRun;
		break;
	case PAUSEBF:
		type = AssembledEvent::PauseRun;
		break;
	case RESUMEBF:
		type = AssembledEvent::ResumeRun;
		break;
	}
	AssembledStateTransitionEvent* pEvent =
	  new AssembledStateTransitionEvent(pFirstFragment->getRunNumber(),
					    type);
	pEvent->setTitle(pFirstFragment->title());
	pEvent->setElapsedTime(pFirstFragment->elapsedTime());
	pEvent->setTimestamp(pFirstFragment->absoluteTime());

	
	return pEvent;
	
}
////////////////////////////////////////////////////////////////////////
/*!
 *  \return uint16_t
 *  \retval type of the first fragment on the list.  
 */
uint16_t
StateTransitionAssemblyEvent::type() const
{
	return m_fragments.front()->type();
}


