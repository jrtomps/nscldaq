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
#include "PhysicsAssemblyEvent.h"
#include "PhysicsFragment.h"
#include "AssembledEvent.h"
#include "AssembledPhysicsEvent.h"
#include <string.h>
#include <buftypes.h>

using namespace std;

//////////////////////////////////////////////////////////////////////
/*!
 *   Create the event from the initial event.
 *   we delegate to add the detail of adding the first
 *   event.
 * \param pFirstFragment - Pointer to the first fragment in the event.
 *                         This should be the fragment from the trigger node.
 */

PhysicsAssemblyEvent::PhysicsAssemblyEvent(PhysicsAssemblyEvent* pFirstEvent) :
	AssemblyEvent(pFirstEvent->getTimestamp())
{
	add(*pFirstEvent);
}
///////////////////////////////////////////////////////////////////////
/*!
 *    Destroy the fragments. The list knows how to take care of itself.
 */
PhysicsAssemblyEvent::~PhysicsAssemblyEvent()
{
	FragmentListIterator i = m_fragments.begin();
	
	while (i != m_fragments.end()) {
		delete *i;
		
		i++;
	}
}
///////////////////////////////////////////////////////////////////////////
/*!
 * \return bool
 * \retval true  - This is a physics fragment.
 */
bool
PhysicsAssemblyEvent::isPhysics() const
{
	return true;
}
////////////////////////////////////////////////////////////////////////////
/*!
 *  Adds a fragment to the list
 * \param frag  - Fragment to add to the list.
 */
void
PhysicsAssemblyEvent::add(EventFragment& fragment)
{
	// The cast below will throw if the fragment is not
	// a physics event fragment:
	
	PhyscisFragment& frag(dynamic_cast<PhysicsFragment&>(frag));
	addNode(frag.node());
	
	m_fragments.push_back(&frag);
}
//////////////////////////////////////////////////////////////////////////////
/*!
 * \return AssembledEvent*
 * \retval A pointer to a dynamcially allocated physics event that is filled in
 *         from the fragments added to this object.  Each fragment becomes a packet
 *         of the form:
 *            size(32) node(16) timestamp(32)  original body.
 */
AssembledEvent*
PhysicsAssemblyEvent::assembledEvent()
{
	AssembledPhysicsEvent* pEvent = new AssembledPhysicsEvent;
	
	// Bodies now get added.. each body requires its own size
	// along with the packet overhead of 
	// the packet size, node number and timestamp for the fragment.
	//
	// One shortcut: we assume sizeof(uint32_t) == 2*sizeof(uint16_t);
	
	FragmentListIterator pFragment = m_fragments.begin();
	while (pFragment != m_fragments.end()) {
		PhysicsFragment* p = *pFragment;
		size_t bodySize     = p->size();
		uint32_t packetsize = bodySize + sizeof(uint32_t) + 1; //sizeof(uint32_t)*2/2.
		uint16_t node       = p->node();
		uint32_t stamp      = p->getTimestamp();
		
		uint16_t* pBody = new uint16_t[packetsize];
		uint16_t* pDest = pBody;
		
		
		
		memcpy(pDest, &packetsize, sizeof(uint32_t));
		pDest++; pDest++;
		*pDest++ = node;
		memcpy(pDest, &timestamp, sizeof(uint32_t));
		pDest++; pDest++;
		
	
		for (size_t i =0; i < bodSize; i++) {
			*pDest++ = (*p)[i];
		}
	
		// Add the packet to the event body:

		pEvent->addData(pBody, packetsize);
		
		// Clean up for the next fragment.
		delete []pBody;
		pFragment++;
	}

	return pEvent;
}
/////////////////////////////////////////////////////////////////////
/*!
 *   \return int16_t
 *   \retval DATABF
 */
uint16_t
PhysicsAssemblyEvent::type() const
{
	return DATABF;
}