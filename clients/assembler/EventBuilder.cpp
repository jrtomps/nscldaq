
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


#include "EventBuilder.h"
#include "AssemblerCommand.h"
#include "AssemblerOutputStage.h"
#include "PhysicsFragment.h"
#include "StateTransitionFragment.h"
#include "EventFragment.h"
#include "OutputPhysicsEvent.h"
#include "StateTransitionEvent.h"
#include "AssembledEvent.h"
#include "OutputPhysicsEvent.h"
#include "AssemblyEvent.h"
#include "StateTransitionAssemblyEvent.h"
#include "PhysicsAssemblyEvent.h"

#include <buftypes.h>
#include <string.h>
#include <assert.h>
#include <iostream>

using namespace std;

////////////////////////////////////////////////////////////////
/*!
 *   In addition to setting the member variables that
 * are references to the objects we need to help us do our job,
 * the constructor must:
    - Clear the statistics
    - Reconstruct the node table from what the configuration will
      give us
    - Register our callback with the input stage.
*/
EventBuilder::EventBuilder(AssemblerCommand&     configuration,
						   InputStage&           fragmentSource,
						   AssemblerOutputStage& eventSink) :
	m_fragmentSource(fragmentSource),
	m_sink(eventSink),
	m_configuration(configuration)
{
	memset(&m_statistics, 0, sizeof(m_statistics));
	reloadConfiguration();
	m_fragmentSource.addCallback(EventBuilder::onInputStageEvent, this);
}
/*!
 *   Just need to get rid of the non-null entries in the m_nodeTable:
 */
EventBuilder::~EventBuilder()
{
	for(int i =0; i < 0x10000; i++) {
		delete m_nodeTable[i];
	}
}
//////////////////////////////////////////////////////////////////////////////////
/*!
 *   Process events from the input stage.  This member function will just figure out 
 * the type of event and re-dispatch to one of:
 *  - onFragments - for event fragments.
 *  - onError     - for error events.
 *  - onShutdown  - for input stage shutdown.
 * \param pObject - actually a pointer to the object that registered the callback.
 * \param why   - The reason the event was declared.
 * \param which - Where appropriate, the node for which the event was declared.
 */
void
EventBuilder::onInputStageEvent(void* pObject, 
		 						uint16_t InputStage::event why, uint16_t which )
{
	EventBuilder* pEventBuilder = dynamic_cast<EventBuilder*>(pObject);
	switch (why) {
	case NewFragments:
		pEventBuilder->onFragments(which);
		break;
	case ShuttingDown:
		pEventBuilder->onShutdown();
		break;
	case Error:
		pEventBuilder->onError(which);
		break;
	default:
		// ignore unexpected event types (e.g. don't care about startup).
		break;
	}
}
////////////////////////////////////////////////////////////////////////////////
/*!
 * Processes fragments from an input queue.  What we are going to do is 
 * peek at the fragment from the node queue and dispatch to the appropriate
 * fragment handling function:
    - onPhysicsFragment if the fragment is, as it will be the majority of the time,
                        a physics fragment.
    - onStateTransitionFragment  if the fragment is a state transition event.
    - onPassThroughFragment for anything else.
    \param node - Node that has new fragments ready.
    */
void
EventBuilder::onFragments(uint16_t node)
{
	EventFragment* pFragment;
	pFragment = m_fragmentSource.peek(node);
	switch (pFragment->type()) {
	case DATABF:
		onPhysicsFragment(node, *pFragment);
		break;
	case BEGRUNBF:
	case ENDRUNGBF:
	case PAUSEBF:
	case RESUMEBF:
		onStateTransitionFragment(node, *pFragment);
		break;
	default:
		onPassThroughFragment(node, *pFragment);
		break;
	}
}
///////////////////////////////////////////////////////////////////
/*!
 *   clear the statistics counters:
 */
void
EventBuilder::clear()
{
	memset(&m_statistics, 0, sizeof(m_statistics));
}
//////////////////////////////////////////////////////////////////
/*!
 *  Return the statistics to the caller
 */
Statistics
EventBuilder::statistics()
{
	return m_statistics;
}
//////////////////////////////////////////////////////////////////
/*!
 *  Reload the configuration in to the local members.
 */
void
EventBuilder::reloadConfiguration()
{
	// clear the old node-list/table.
	// The table just has pointers back into the node list so it
	// can just be zeroed.  The list, however has pNodeName strings that
	// must be deleted before the list can be cleared:
	
	memset(m_nodeTable, 0, sizeof(m_nodeTable));
	list<AssemblerCommand::EventFragmentContributor>::iterator p =
		m_nodeList.begin();
	while(p != m_nodeList.end()) {
		delete []p->pNodeName;
		p++;
	}
	m_nodeList.clear();
	
	// Build the new node-list/lookup table.
	
	m_nodeList = m_configuration.getConfiguration();
	p          = m_nodeList.begin();
	while (p != m_nodeList.end()) {
		m_nodeTable[p->cpuId] = &(*p);  // Seems strange but p is an iterator not a ptr.
		p++;
	}
}

///////////////////////////////////////////////////////////////////
/*!
 *   This is the hard one (well it's not actually that hard
 * as mostly we farm off the stuff to sub-functions.  
 * A physics fragment has arrived.
 * Two cases:
 * - From the trigger node, in which case we just need to make
 *   a new event assembly and insert it in our collection,
 *   Then check to see if there are already matching events in the
 *   list...we do that until the input queue for that node is empty.
 * - From a non trigger node, here we just match as many events as possible
 *   from the node that signalled with events in the in-flight assembly list.
 *   There's an assumption that the events in the in-flight list will be
 *   monotonically increasing in time (modulo wrapping), so if we fine a match
 *   for fragment i at in-flight event j, we can look for matches for i+1 starting
 *   with j+1...we do this until either the input queue is empty or we cannot match
 *   a fragment with an in-flight assembl.
 * Afterwards we do a 'prune pass' on all nodes.  This means that we discard all
 * events from the front of all event queues (except the trigger queue) which can
 * never possibly match the first event in the queue ever (e.g. the end of the window
 * for those events are prior to the trigger time of the first node in the in-flight
 * assembly list.
 * 
 * \param node     - The ndoe that has new fragments.
 * \param fragment - Reference to the first fragment in the list.
 */
void 
EventBuilder::onPhysicsFragment(uint16_t         node, 
							    PhysicsFragment& fragment)
{
	// node had better be in the configuration:
	
	AssemblerCommand::EventFragmentContributor* pNodeDescription = m_nodeTable[node];
	assert(pNodeDescripion);
	
	if(pNodeDescription->isTrigger()) {  // Trigger node
		createTriggerAssemblies(node);
		checkMatchingFragments();
	}
	else{                                // non-trigger node
		checkMatchingFragments(node);
	}
	pruneNonTriggerNodes();
	
}
////////////////////////////////////////////////////////////////////////
/*!
 * State transition fragment handling.  The assumption is that these are rare enough
 * there's only typically going to be one of these in flight at any given time.
 * If we can find a state transition fragment of our type we aggregate with it, otherwise
 * make a assembly.
 * If in the end we have a complete assembly we
 *  - flush all prior fragments from the event queue.
 *  - Commit the assembled state transition event to the outut stream
 *  - pop the completed assembly from the queue and delete it (and it's little fragments
 *    too).
 * \param node     - node that has signalled it has a fragment.
 * \param fragment - reference to the state transition fragment we got.
 *  
 */
void
EventBuilder::onStateTransitionFragment(uint16_t node, 
										StateTransitionFragment& fragment)
{
	OutuptPhysicsEvents::iterator p = m_inFlight.findByType(fragment.type());
	StateTransitionAssemblyEvent* pEvent;
	if (p == m_inFlight.end()) {    // New
		pEvent = new StateTransitionAssemblyEvent(fragment);
		m_inFlight.add(*pEvent);
		
	}
	else {                          // Existing.
		pEvent = *p;
		p->add(fragment);
	}
	// At this time:
	// We need to see if pEvent is complete if so;
	// Avoid discarding as many events as possible by running a matching pass
	// over all input queues.
	// then discard the remnant events prior to the one we just inserted..
	// which should we'll hunt down again, just in case.
	// We can afford to spend this time re-hunting 'cause state transitions are rare.
	
	
	if (pEvent->isComplete()) {
		checkMatchingFragments();
		
		OutputPhysicsEvents::AssemblyList killList = removePrior(p);
		for (OutputPhysicsEvents::iterator i = killList.begin(); i != killList.end(); i++) {
			delete *i;
		}
		commitStateTransitionEvent(*dynamic_cast<StateTransitionEvent*>(pEvent->assembledEvent()));
		p = find(m_inFlight.begin(), m_inFlight.end, pEvent);
		m_inFlight.erase(p);
		delete pEvent;
	}
}
////////////////////////////////////////////////////////////////////////////
/*!
 * Handle pass through fragments.  Pass through fragments never need assembly.
 * They are passed on to the output stage as is.
 * If removing the event from the input queue does not leave the input queue
 * empty, We will recurse inon on Fragments.
 * 
 * \param node  - Node who's queue is being processed.
 * \param fragment - The passthrough fragment.
 */
void
EventBuilder::onPassThroughFragment(uint16_t       node, 
								    EventFragment& fragment)
{
	m_fragmentSource.pop(node);
	
	pruneNonTriggerNodes();               // Try to trash any fragments that are too old
	checkMatchingFragments();             // Try to match any stragglers.
	
	commitPassThroughEvent(fragment.assembledEvent());
	delete &fragment;
	
	EventFragment* pNextFragment = m_fragmentSource.peek(node);
	if(pNextFragment) {
		onFragments(node);                // Process more fragments.
	}
}
/////////////////////////////////////////////////////////////////////////////
/*!
 * Commit an assembled physics event to the output queue.
 *  \param event - an assembled event that can be committed to the output stage.
 */
void
EventBuilder::commitAssembledPhysicsEvent(AssembledPhysicsEvent& event)
{
	m_statistics.eventsByType[event.type()]++;
	
	m_sink.submitEvent(event);
}
/////////////////////////////////////////////////////////////////////////////////
/*!
 * Commit an assembled state transition event.
 *  \param event Event to commit.
 */
void
EventBuilder::commitAssembledStateTransitionEvent(StateTransitionEvent& event)
{
	m_statistics.eventsByType[event.type()]++;
	
	m_sink.submitEvent(event);
}
/////////////////////////////////////////////////////////////////////////////
/*!
  Commits a passthrough event to the output stage.
  \param event - The event to commit
 */
void
EventBuilder::commitPassthroughEvent(AssembledEvent& event)
{
	m_statistics.eventsByType[event.type()]++;
	m_sink.submitEvent(event);
}
//////////////////////////////////////////////////////////////////////////////
/*!
 *  Take action on an input error on a sink.  At present, this will just put out
 * an error message to the stderr.  It may well be the application will allow
 * hooks to handle this at the Tcl/script level.
 * \param node - The node that had the error.
 */
void
EventBuilder::onError(uint16_t node)
{
	AssemblerCommand::EventFragmentContributor* pNode = m_nodeTable[node];
	assert(pNode);
	cerr << "Warning, an input error was detected on node id " << node
		<< " (0x" << hex << node << dec << ") Name: " << pNode->pNodeName
		<< " Node " << pNode->isTrigger ? "is " : "is not " << "the trigger node\n";
 
}
//////////////////////////////////////////////////////////////////////////////////
/*!
 * Processes the shtudown of the input stage.  At present this is a no-op.
 */
void
EventBuilder::onShtudown()
{
	
}
//////////////////////////////////////////////////////////////////////////////
/*!
 * Check all nodes except the trigger node for fragments that match
 * in-flight assembly fragments. 
 */
void
EventBuilder::checkMatchingFragments()
{
	// just loop over the nodes calling the node specific overload
	// for all non trigger cases:
	
	list<AssemblerCommand::EventFragmentContributor>::iterator p = m_nodeList.begin();
	while (p != m_nodeList.end()) {
		if (!p->isTrigger) {
			checkMatchingFragments(p->cpuId);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
/*!
 *   Check for matches between physics fragments in this node's input queue
 *   with in flight assemblies.  If there are matches, the item is removed from
 *   the queue and added to the fragment.
 * \param node  The id of the node to check.
 */
void
EventBuilder::checkMatchingFragments(uint16_t node)
{
	AssemblerCommand::EventFragmentContributor* pSourceInfo = m_nodeTable[node];
	assert(pSourceInfo);

	// We can break out of the loop below when we don't get a match.
	// the loop computes the matching window for each fragment it
	// inspects and sees if there are assemblies the event can be added to.
	//  note that only physics events are checked.. if a non-physics
	// event is seen, onFragments is recursively called to deal with it appropriately.
	//
	EventFragment* pFragment = m_fragmentSource.peek(node);
	OutputPhysicsEvents::iterator start = m_inFlight.begin();
	OutputPhysicsEvents::iterator stop  = m_inFlight.end();
	
	while (pFragment) {
		// If not physics deal with that
		
		if (pFragment->type() != DATABF) {
			onFragments(node);                  // Redispatch the non physics fragment.
			
		}
		else {
			// Get the matching window
			
			EventFragment* pEventFragment = dynamic_cast<EventFragment*>(pFragment);
			TimeWindow interval = matchInterval(*pSourceInfo, *pEventFragment);
			
			
			start = m_inFlight.findPhysByWindow(interval.startTime, 
												interval.endTime, start, stop);
			if (start == stop) {
				return;                   // No match
			}
			else {
				AssemblyEvent* pAssembly = *start;
				pAssembly->add(*pEventFragment);             // Add fragment...
				m_fragmentSource.pop(node);              // Remove from input queue
				if (pAssembly->isComplete()) {
					// Remove and commit the event.. and reset start as
					// removal can invalidate the iterator.
					
					m_inFlight.remove(start);          // Invalidates start so...
					start = m_inFlight.begin();        // reset search from beginning.
					
					commitAssembledPhysicsEvent(*(pAssembly->assembledEvent()));
					
					delete pAssembly;
				}
				else {
					// start next search with next assembly...
					
					start++;
					if (start == stop) {
						return;                         // no more assemblies to match.
					}
				}
				
			}
			
		}
		pFragment = m_fragmentSource.peek(); // Next fragment.
	}
}
////////////////////////////////////////////////////////////////////////////////////
/*!
 *  Create a trigger assembly for each physics fragment in the designated input
 * queue.  For any fragment that is not a physics event, we'll recurse to onFragments
 * \param node The node to process. This must be a trigger node.
 */
void
EventBuilder::createTriggerAssemblies(uint16_t node)
{
	AssemblerCommand::EventFragmentContributor* pNodeInfo = m_nodeTable[node];
	assert(pNodeInfo);
	assert(pNodeInfo->isTrigger);
	
	EventFragment* pFragment;
	while (pFragment = m_fragmentSource.peek(node)) {
		if (pFragment->isPhysics()) {
			PhysicsFragment* pPhysicsFragment = dynamic_cast<PhysicsFragment*>(pFragment);
			m_inFlight.add(*(new PhysicsAssemblyEvent(pPhysicsFragment)));
			m_fragmentSource.pop(node);
		}
		else {
			onFragments(node);       // should remove the fragment from the input queue.
			
		}
		pFragment = m_fragmentSource.peek(node); // Look at the next fragment.
	}
}
/////////////////////////////////////////////////////////////////////////////////////
/*!
 *   Prune events from non trigger nodes.  See the comments to 
 * pruneNode for more information about what this means, other than
 * to get rid of fragments that could no longer possibly match.
 */
void
EventBuilder::pruneNonTriggerNodes()
{
	list<AssemblerCommand::EventFragmentContributor>::iterator  p = m_nodeList.begin();
	while (p != m_nodeList.end()) {
		if (!p->isTrigger) {
			pruneNode(p->cpuId);
		}
	}
}
///////////////////////////////////////////////////////////////////////
/*!
 * Prunes the fragments from a non-trigger node that cannot possibly
 * be matched.  These are fragments whose window end time is prior
 * to the trigger time of the first physics event in the in flight
 * assembly list (naturally if there are no in flight physics events,
 * nothing can be pruned.
 * \param node The node whose input queue will be pruned.
 * \note - there's an edge case for the condition where the trigger window
 *         wraps across zero...in that case the trigger time must also be less
 *         than the window start time.. the wrap is evident (for window << MAX_UINT)
 *         because the window end time will appear to be smaller than the start time.
 * \note - It is assumed the caller is handing us a valid, non-trigger node.
 * \note - We stop processing if either we find a fragment that could match, or
 *         we find a non physics fragment in the input queue, or we run out of
 *         input queue fragments.
 */
void
EventBuilder::pruneNode(uint16_t node)
{
	AssemblerCommand::EventFragmentContributor* pNodeInfo = m_nodeTable[node];
	OutputPhysicsEvents::iterator firstPhysics = m_inFlight.findByType(DATABF);
	if (firstPhysics == m_inFlight.end()) return;       // No physics fragments.
	uint32_t timestamp = (*firstPhysics)->timestamp();	
	
	EventFragment* pFragment;
	while(pFragment = m_fragmentSource.peek(node)) {
		if (pFragment)
		TimeWindow match = matchInterval(*pNodeInfo, *p, timestamp)
		bool canPrune = false;
		if (match.endTime < timestamp) {
			if (match.endTime < match.startTime) {
				if (match.startTime > timestamp) {  // may need more smarts than this...
					canPrune = true;
				}
			}
			else {
				canPrune = true;
			}
			if (canPrune) {
				delete pFragment;
				m_fragmentSource.pop(node);
			}
			else {
				return;
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////
/*!
 * Compute the time matching window for an event fragment given it's node
 * description.
 * \param nodeInfo - Describes the node and it's place in the configuration.
 * \param fragment - Fragment for which the matching interval is computed.
 * \return TimeWindow
 * \retval The time interval that would constitute a match.
 */
EventBuilder::TimeWindow
EventBuilder::matchInterval(AssemblerCommand::EventFragmentContributor& nodeInfo,
        				    PhysicsFragment&                            fragment)
{
	uint32_t timestamp = fragment.getTimestamp();
	if (nodeInfo.offsetDefined) timestamp += nodeInfo.offset;
	
	TimeWindow result;
	result.startTime = timestamp - nodeInfo.windowWidth;
	result.endTime   = timestamp + nodeInfo.windowWidth;
	
	return result;
}