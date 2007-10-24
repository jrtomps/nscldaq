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
#incclude <config.h>
#include "AssemblyEvent.h"
#include "NodeScoreboard.h"
#include "EventFragment.h"



/*!
 *    Build the event from it s trigger timestamp.
 *    A new scoreboard to keep track of the nodes this event has
 *    had is also created
 *  \param triggerTime - the trigger time to set for the purpose of matching.
 */
AssemblyEvent::AssemblyEvent(uint32_t triggerTime) :
	m_triggerTime(triggerTime),
	m_scoreboard(*(new NodeScoreboard))
{
	
}
/*!
 *   Destruction just requires we delet the node scoreboard.
 */
AssemblyEvent::~AssemblyEvent()
{
	delete &m_scoreboard;
}
/////////////////////////////////////////////////////////////////////////
/*!
 * Return the timestamp.
 */
uint32_t
AssemblyEvent::timestamp() const
{
	return m_triggerTime;
}
/////////////////////////////////////////////////////////////////////////
/*!
 * Return true if the event has gotten contributions from all the
 * fragments it needs to be a complete event.  This is delegated
 * to the scoreboard.
 */
bool
AssemblyEvent::isComplete() const
{
	return m_scoreboard.isComplete();
}
////////////////////////////////////////////////////////////////////////////
/*
 * add a node to the set of nodes seen
 */
void
AssemblyEvent::addNode(uint16_t node)
{
	m_scoreboard.addNode(node)
}