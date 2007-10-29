
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
#include "OutputPhysicsEvents.h"
#include "AssemblyEvent.h"
#include "EventFragment.h"
#include <limits.h>
#include <algorithm>
#include <functional>

using namespace std;

///////////////////////////////////////////////////////////////////
//
// Helper functions to match AssemblyEvents by type
//




bool
matchType(AssemblyEvent* p, uint16_t type)
{
	return (p->type() == type);
}

inline bool
between(uint32_t value, uint32_t low, uint32_t high)
{
	return ((value >= low) && (value <= high));
}
// Match a physics event  by time window.  This is a bit tricky
// with special cases due to the potential for wrapping.
// Wrapping occurs if the stop time is less that the start time.
// in that case the match is for either of two windows:
// [start,maxint], [0,stop].
//
bool
matchWindow(AssemblyEvent* p, pTimeWindow window)
{
	if(p->isPhysics()) {                         // is physics assembly.
		uint32_t timestamp = p->timestamp();
		if (window->start < window->stop) {         // No wrap:
			return between(timestamp, window->start, window->stop);
		}
		else {                                      // wrap
			return (between(timestamp, window->start, UINT_MAX)   ||
					between(timestamp, 0, window->stop));
		}	                                       
	}
	else {                                         // not even a physics assembly.
		return false;
	}
}
// Help us count ranges using the count_if algorithm:

bool
True(AssemblyEvent*)
{
	return true;
}
///////////////////////////////////////////////////////////////////
/*!
 * Construction is pretty well a no-op.
 */
OutputPhysicsEvents::OutputPhysicsEvents()
{
}
//////////////////////////////////////////////////////////////////
/*!
 *   Destruction will require that we destroy the various event
 * assemblies in progress.
 */
OutputPhysicsEvents::~OutputPhysicsEvents()
{
	iterator i = begin();
	while (i != end()) {
		delete *i;
		i++;
	}
		
}
////////////////////////////////////////////////////////////////////////
/*!
 *   Add an event to the list of those being assembled. 
 * Events are always added to the end of the list.. This should keep the
 * physics events ordered by timestamp and the state transition events
 * ordered causally amongst them.
 * \param event  The new event to assemble.
 * 
 */
void
OutputPhysicsEvents::add(AssemblyEvent& event)
{
	m_assemblies.push_back(&event);
}
/////////////////////////////////////////////////////////////////////////////
/*!
 * Remove a specific event given an iterator 'pointing' to the event to remove.
 * A pointer to the removed event is returned.  The caller must manage storage
 * for that event e.g.:
 * \verbatim
 *    AssemblyEvent* pE = list.remove(iter);
 *    delete pE;
 * \endverbatim
 */
AssemblyEvent*
OutputPhysicsEvents::remove(OutputPhysicsEvents::iterator where)
{
	AssemblyEvent* pEvent(0);
	if (where != end()) {
		pEvent = *where;
	}
	m_assemblies.erase(where);
	
	return pEvent;
}
////////////////////////////////////////////////////////////////////////////////
/*!
 * \return OutputPhysicsEvents::iterator
 * \retval an interator that 'points' to the first element in the collection.
 *         note that for an empty collection, begin() == end().
 */
OutputPhysicsEvents::iterator
OutputPhysicsEvents::begin()
{
	return m_assemblies.begin();
}
//////////////////////////////////////////////////////////////////////////////////
/*!
 * \return OutputPhysicsEvents::iterator
 * \retval an iterator that 'points' off the end of the collection.
 */
OutputPhysicsEvents::iterator
OutputPhysicsEvents::end()
{
	return m_assemblies.end();
}
//////////////////////////////////////////////////////////////////////////////
/*!
 * \return size_t
 * \retval The number of in-flight event assemblies.
 */
size_t
OutputPhysicsEvents::size() const
{
	return m_assemblies.size();
}
/////////////////////////////////////////////////////////////////////////////
/*!
 * Finds the first event of a specific type.
 * \param type   - the type to look for.
 * \return OutputPhysicsEvent::iterator
 * \retval Iterator for the first match
 * \retval end() if there  is no match.
 * 
 */
OutputPhysicsEvents::iterator
OutputPhysicsEvents::findByType(uint16_t type)
{
  return findByType(type, begin(), end());
}
///////////////////////////////////////////////////////////////////////////////
/*!
 *    Locate an event assembly in progress with a given type, within
 * some range of the collection of in-flight assemblies:
 * \param type - The type to match on.
 * \param start - Iterator describing where to start the match search
 * \param stop  - Iterator describing where to stop the match search.
 * \return OutputPhysicsEvent::iterator
 * \retval Iterator for the first match
 * \retval end() if there is no match in the range.
 */
OutputPhysicsEvents::iterator
OutputPhysicsEvents::findByType(uint16_t type, 
				OutputPhysicsEvents::iterator start,
				OutputPhysicsEvents::iterator stop)
{
  return find_if(start, stop, bind2nd(ptr_fun(matchType), type));
}
/////////////////////////////////////////////////////////////////////////////////
/*!
 *  Find a physics event that has a timestamp in a given time window.
 *  \param windowStart  time at which the matching window opens.
 *  \param windowStop   Time at which the matching window closes
 * \return OutputPhysicsEvents::iterator 
 * \retval Iterator to first matching event.
 * \retval end() - No events match that time window.
 */
OutputPhysicsEvents::iterator
OutputPhysicsEvents::findPhysByWindow(uint32_t windowStart, uint32_t windowStop)
{
	return findPhysByWindow(windowStart, windowStop, begin(), end());
}
////////////////////////////////////////////////////////////////////////////////
/*!
 *   Find a physics event that has a timestamp in a given time window
 *   within a range of the collection of in-flight assemblies:
 *  \param windowStart  time at which the matching window opens.
 *  \param windowStop   Time at which the matching window closes
 *  \param start  Iterator describing where to start the search.
 *  \param stop   Iterator describing where to end the search.
 *  \return OutputPhysicsEvents::iterator
 *  \retval Iterator that 'points' to the first match.
 *  \retval end() if there is no match.
 */
OutputPhysicsEvents::iterator
OutputPhysicsEvents::findPhysByWindow(uint32_t windowStart, uint32_t windowStop,
									  OutputPhysicsEvents::iterator start,
									  OutputPhysicsEvents::iterator stop)
{
	TimeWindow window = {windowStart, windowStop};
	return find_if(start, stop, bind2nd(ptr_fun(matchWindow), &window));
}
//////////////////////////////////////////////////////////////////
/*!
 *   Remove all events prior to a specific position in the 
 * collection.  The list of removed assemblies is returned.
 * One use of this is to flush out partial events in processing
 * a state transition event.
 * \param stop  - The end point, all assemblies up to but not including
 *                this position will be flushed.  e.g. end() will empty the
 *                collection.
 * \return OutputPhysicsEvents::AssemblyList
 * \retval List of items removed.
 * \note if the assemblies are dynamically allocated (and the probably are),
 *       the caller must take care of destroying them when they are no longer needed.
 */
OutputPhysicsEvents::AssemblyList
OutputPhysicsEvents::removePrior(OutputPhysicsEvents::iterator stop)
{
	return remove(begin(), stop);
}
//////////////////////////////////////////////////////////////////
/*!
 *   Remove a specified range of items from the list.
 *   \param start   - Starting iterator for removal (inclusive)
 *   \param stop    - Ending iterator for removal (exclusive)
 *   \return OutputPhysicsEvents::AssemblyList
 *   \retval List of items removed.
 */
OutputPhysicsEvents::AssemblyList
OutputPhysicsEvents::remove(OutputPhysicsEvents::iterator start,
			    OutputPhysicsEvents::iterator stop)
{
	AssemblyList result;
	iterator i = start;
	while (i != stop) {
		result.push_back(*i);
		i++;
	}
	m_assemblies.erase(start,stop);
	
	return result;
}
///////////////////////////////////////////////////////////////////////
/*!
 *   Remove a single item from the assembly list.
 * This is typically used to remove an item that has been completely
 * assembled.
 * \param item  - Iterator pointing at the item to remove.
 * \return AssemblyEvent* 
 * \retval the item removed.
 * \note it is up to the caller to delete the event when it is no longer needed.
 */
AssemblyEvent*
OutputPhysicsEvents::removeItem(OutputPhysicsEvents::iterator item)
{
	AssemblyEvent* result = *item;
	m_assemblies.erase(item);
	
	return result;
}
////////////////////////////////////////////////////////////////////////
/*!
 *    Count the number of elemnts in [begin, here).
 * \param here - end point of count.
 * \return size_t
 * \retval number of elements in the range [begin,here)
 */
size_t 
OutputPhysicsEvents::countPrior(OutputPhysicsEvents::iterator here)
{
	return count_if(begin(), here, True);
}
//////////////////////////////////////////////////////////////////////
/*!
 *   Count the number of elements between two iterators.
 * \param start where to start couting (inclusive)
 * \param stop where to stop counting (exclusive)
 * \return size_t
 * \retval number of elements in the range [start, stop)
 */
size_t
OutputPhysicsEvents::countRange(OutputPhysicsEvents::iterator start,
								OutputPhysicsEvents::iterator stop)
{
	return count_if(start, stop, True);
}
