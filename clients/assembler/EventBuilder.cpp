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

#include "EventBuilder.h"
#include "AssemblerCommand.h"
#include "AssemblerOutputStage.h"
#include "PhysicsFragment.h"
#include "StateTransitionFragment.h"
#include "EventFragment.h"
#include "ScalerFragment.h"
#include "StringListFragment.h"
#include "OutputPhysicsEvents.h"
#include "AssembledEvent.h"
#include "AssemblyEvent.h"
#include "StateTransitionAssemblyEvent.h"
#include "AssembledStateTransitionEvent.h"
#include "AssembledStringArrayEvent.h"
#include "AssembledScalerEvent.h"
#include "AssembledPhysicsEvent.h"
#include "PhysicsAssemblyEvent.h"
#include "NodeScoreboard.h"

#include "InputStage.h"
#include "InputStageCommand.h"

#include <buftypes.h>
#include <string.h>
#include <assert.h>
#include <iostream>

using namespace std;

time_t EventBuilder::m_currentTime; // Time is time.

static const time_t TOO_OLD(2);	   // Number of second's we'll keep fragments.

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
			   InputStageCommand&    fragmentSource,
			   AssemblerOutputStage& eventSink) :
  m_InputStageCommand(fragmentSource),
  m_pInputStage(0),
  m_sink(eventSink),
  m_configuration(configuration)
{
  memset(&m_statistics, 0, sizeof(m_statistics));
  reloadConfiguration();
  
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
				InputStage::event why, uint16_t which )
{
  EventBuilder* pEventBuilder = reinterpret_cast<EventBuilder*>(pObject);


  switch (why) {
  case InputStage::NewFragments:
    pEventBuilder->onFragments(which);
    break;
  case InputStage::ShuttingDown:
    pEventBuilder->onShutdown();
    break;
  case InputStage::Error:
    pEventBuilder->onError(which);
    break;
  default:
    // ignore unexpected event types (e.g. don't care about startup).
    break;
  }
}
////////////////////////////////////////////////////////////////////////////////
/*!
 * Processes fragments from an input queue.
 * We will process fragments until the input queue is empty.
 * Each fragment will be dispatched to its appropriate handler.
 * - Event fragments to the onPhysicsFragment member,
 * - State transition fragments to the onStateTransitionFragment handler.
 * - All others to the onPassThroughFragment handler.
 *
 * \param node - The node from which the fragments have arrived.
 *
*/
void
EventBuilder::onFragments(uint16_t node)
{
  EventFragment* pFragment;
  time_t  prior = m_currentTime;
  m_currentTime = time(NULL);


  while (pFragment = m_pInputStage->pop(node)) {
    m_statistics.fragmentsByNode[node]++; //  Count before dispatching.
    switch (pFragment->type()) {
    case DATABF:
      {
	PhysicsFragment* pPhysicsFragment = 
	  reinterpret_cast<PhysicsFragment*>(pFragment);
	onPhysicsFragment(node, *pPhysicsFragment);
	break;
      }
    case BEGRUNBF:
    case ENDRUNBF:
    case PAUSEBF:
    case RESUMEBF:
      {
	StateTransitionFragment* pTransitionFragment = 
	  reinterpret_cast<StateTransitionFragment*>(pFragment);	
	onStateTransitionFragment(node, *pTransitionFragment);
	break;
      }
    default:
      onPassThroughFragment(node, *pFragment);
      break;
    }
  }
  // Now we rip through the unmatched queue attempting to match any
  // unmatched fragments. State transition buffers may have already done this.


  checkMatchingFragments();
  
  // Prune all fragments that are too old if the time has clicked:

  if (prior != m_currentTime) {
    pruneFragments();
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
EventBuilder::Statistics
EventBuilder::statistics() const
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
  // and setup the expected nodes in the nodescoreboard class.

  vector<uint16_t> expectedNodes;
  m_nodeList = m_configuration.getConfiguration();
  p          = m_nodeList.begin();
  while (p != m_nodeList.end()) {
    m_nodeTable[p->cpuId] = &(*p);  // Seems strange but p is an iterator not a ptr.
    expectedNodes.push_back(p->cpuId);
    p++;
  }
  NodeScoreboard::neededNodes(expectedNodes);


  // If possible get the input stage going:
  
  
  InputStage* pNewStage = m_InputStageCommand.getInputStage();
  if (pNewStage != m_pInputStage) {
    m_pInputStage = pNewStage;
    if (m_pInputStage) {
      m_pInputStage->addCallback(EventBuilder::onInputStageEvent, this);
    }
  }
  // Finally clear out the input and assembly queues.

  emptyUnmatchedQueue();
  flushAssemblyQueue(m_inFlight.end());
}

///////////////////////////////////////////////////////////////////
/*!
 * Processes a physics fragment from an input node.
 * - If the node was the trigger node, we just create a new
 *   in-flight assembly for the fragment.
 * - If the node was a non trigger node, it goes in the
 *   unmatched fragment queue for later match.
 *  
 * \param node     - The node that has new fragments.
 * \param fragment - Reference to the first fragment in the list.
 */
void 
EventBuilder::onPhysicsFragment(uint16_t         node, 
				PhysicsFragment& fragment)
{
  // node had better be in the configuration:
  
  AssemblerCommand::EventFragmentContributor* pNodeDescription = m_nodeTable[node];
  assert(pNodeDescription);
  
  if(pNodeDescription->isTrigger) {  // Trigger node
    createTriggerAssembly(fragment);
  }
  else{                                // non-trigger node
    
    m_unmatchedFragments.push_back(new PendingFragment(m_currentTime, &fragment));
  }

  
}
////////////////////////////////////////////////////////////////////////
/*!
 * Handle a state transition fragment.
 * - Attempt to create a complete transition fragment.
 * - If the transition fragment completes a halting
 *   transition (Pause or end run), all fragments in the
 *   unmatched fragment queue are emptied.
 * \param node     - node that has signalled it has a fragment.
 * \param fragment - reference to the state transition fragment we got.
 *  
 */
void
EventBuilder::onStateTransitionFragment(uint16_t node, 
					StateTransitionFragment& fragment)
{

  OutputPhysicsEvents::iterator p = m_inFlight.findByType(fragment.type());
  StateTransitionAssemblyEvent* pEvent;
  if (p == m_inFlight.end()) {    // New
    pEvent = new StateTransitionAssemblyEvent(fragment, m_currentTime);
    m_inFlight.add(*pEvent);
    p = m_inFlight.end();
    p--;			// Points to last fragment (the one we just inserted).
    
  }
  else {                          // Existing.
    pEvent = dynamic_cast<StateTransitionAssemblyEvent*>(*p);
    pEvent->add(fragment);
  }
  // At this time:
  // We need to see if pEvent is complete if so;
  // Avoid discarding as many events as possible by running a matching pass
  // over all input queues.
  // then discard the remnant events prior to the one we just inserted..
  // which should we'll hunt down again, just in case.
  // We can afford to spend this time re-hunting 'cause state transitions are rare.
  
  
  if (pEvent->isComplete()) {
    checkMatchingFragments();	// This probably won't find anything...
  
    if ((fragment.type() == ENDRUNBF)  || (fragment.type() == PAUSEBF)) {
      emptyUnmatchedQueue();
      flushAssemblyQueue(p);	// Invalidates the p iterator.
    }


    commitAssembledStateTransitionEvent(*reinterpret_cast<AssembledStateTransitionEvent*>(pEvent->assembledEvent()));
    p = find(m_inFlight.begin(), m_inFlight.end(), pEvent); // probably the front element!
    m_inFlight.removeItem(p);
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
 * \note  At present, there are two kinds of event fragments
 *        that can wind up here. stringlist buffers and
 *        scaler buffers.  That's currently handled via an if/else
 *        statement that would have to be modified if other
 *        types of events could land here.
 */
void
EventBuilder::onPassThroughFragment(uint16_t       node, 
				    EventFragment& fragment)
{
  
  AssembledEvent* pAssembledEvent;
  
  // How we set pAssembledEvent depends on the fragment type:
  //
  uint16_t type = fragment.type();
  if ((type == SCALERBF) || (type == SNAPSCBF))  {
    
    // Scaler fragment:
    
    ScalerFragment* pScalerFragment = reinterpret_cast<ScalerFragment*>(&fragment);
    AssembledScalerEvent* pScaler = new AssembledScalerEvent(fragment.node(),
							     pScalerFragment->startTime(),
							     pScalerFragment->endTime(),
							     static_cast<AssembledEvent::BufferType>(type));
    pScaler->addScalers(pScalerFragment->scalers());
    
    pAssembledEvent = pScaler;
    
  }
  else {
    
    // String list fragment.
    
    StringListFragment* pStringListFragment = reinterpret_cast<StringListFragment*>(&fragment);
    AssembledStringArrayEvent* pStringArray = new AssembledStringArrayEvent(fragment.node(),
									    static_cast<AssembledEvent::BufferType>(type));
    pStringArray->addStrings(pStringListFragment->strings());
    pAssembledEvent = pStringArray;
  }

  // Commit the event to the output stage and delete the associated fragment.
  
  commitPassthroughEvent(*pAssembledEvent);
  delete &fragment;

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
EventBuilder::commitAssembledStateTransitionEvent(AssembledStateTransitionEvent& event)
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
	const char* isIsNot = pNode->isTrigger ? "is " : "is not ";
	cerr << "Warning, an input error was detected on node id " << node
		<< " (0x" << hex << node << dec << ") Name: " << pNode->pNodeName
		<< " Node " << isIsNot << "the trigger node\n";
 
}
//////////////////////////////////////////////////////////////////////////////////
/*!
 * Processes the shtudown of the input stage.  At present this is a no-op.
 */
void
EventBuilder::onShutdown()
{
	
}

////////////////////////////////////////////////////////////////////////////////
/*
 *  Take all fragments that are in the unmatched queue and match them
 *  with in flight fragments. 
 *  We exploit the fact that erasures from lists only invalidate the iterators
 *  pointing to the deleted element.
 */
void
EventBuilder::checkMatchingFragments()
{
  PhysicsFragmentList::iterator p = m_unmatchedFragments.begin();
  while (p != m_unmatchedFragments.end()) {
    PhysicsFragment* pFragment = (*p)->fragment;
    PhysicsFragmentList::iterator pNext = p; pNext++; // Do this now so we can invalidate p.
    

    // See if this fragment matches any in the in-flight list:

    OutputPhysicsEvents::iterator pOut = m_inFlight.begin();
    while (pOut != m_inFlight.end()) {
      AssemblyEvent* pAssembly = *pOut;
      if (fragmentMatches(*pAssembly, pFragment)) {
	pAssembly->add(*pFragment);
	if (pAssembly->isComplete()) {
	  commitAssembledPhysicsEvent(*(reinterpret_cast<AssembledPhysicsEvent*>(pAssembly->assembledEvent())));
	  m_inFlight.removeItem(pOut);
	  delete pAssembly;
	}
	m_unmatchedFragments.erase(p); // Remove fragment from the queue.
	break; 
      }
      pOut++;
    }
    p = pNext;
  }
}
/////////////////////////////////////////////////////////////////////////////////
/*
 *  Create an in-flight assembly for a trigger event fragment, and add it to the
 *  inflight list of assemblies in progress.
 *
 * Parameters:
 *    PhysicsFragment& fragment  - The initial fragment.
 */
void
EventBuilder::createTriggerAssembly(PhysicsFragment& fragment)
{
  PhysicsAssemblyEvent* pEvent = new PhysicsAssemblyEvent(&fragment, m_currentTime);
  if (pEvent->isComplete()) {
        commitAssembledPhysicsEvent(*(reinterpret_cast<AssembledPhysicsEvent*>(pEvent->assembledEvent())));
  }
  else {
    m_inFlight.add(*pEvent);
  }
}

//////////////////////////////////////////////////////////////////////////////////
/*
 *  Clear all the events out of the unmatched event fragment queue.
 *  For each event in the queue, we remove it, increment the associated
 *  discardedByNode and unmatchedByNode counters and delete the fragment.
 */
void
EventBuilder::emptyUnmatchedQueue()
{
  PhysicsFragment* pFragment;
  pPendingFragment pPending;
  while (!m_unmatchedFragments.empty()) {
    pPending = m_unmatchedFragments.front();
    pFragment= pPending->fragment;
    m_unmatchedFragments.pop_front();

    uint16_t node = pFragment->node();
    m_statistics.unmatchedByNode[node]++;
    m_statistics.discardedByNode[node]++;
    delete pFragment;
    delete pPending;
  }
}
/////////////////////////////////////////////////////////////////////////////////
/*
 *    Flush the assembly queue of partial events up to but not including
 *     an iterator.
 *    Flush means remove these fragments from the queue and delete them.
 *
 * Parameters:
 *   OutputPhysicsEvents::iterator p  - Iterator to the first item not to flush.
 */
void
EventBuilder::flushAssemblyQueue(OutputPhysicsEvents::iterator p)
{
  OutputPhysicsEvents::AssemblyList listOfPartials = m_inFlight.removePrior(p);

  OutputPhysicsEvents::AssemblyList::iterator i = listOfPartials.begin();
  while(i != listOfPartials.end()) {
    delete *i;
    i++;
  }
  // The list itself knows how to clean up its nodes.

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
//////////////////////////////////////////////////////////////////////////////////
/*
   Determine if a physics fragment matches a partial event.
   This can only be the case if 
   1. The partial event is a partial physics event.
   2. The partial event's trigger time is in the fragment's matching window.

  Note that timestamps can and will wrap (at 100Khz, every 74 min or so),
       Suppose the trigger time is T and the window start time S and stop time
       E.  A match occurs if:
          S <= T <= E
      or 
          S > E && ( (S <= T ) || (0 <= T <= E))

 Parameters:
   
*/
bool
EventBuilder::fragmentMatches(AssemblyEvent& partialEvent, PhysicsFragment* pFragment)
{
  if (partialEvent.isPhysics()) {
    uint32_t triggerTime = partialEvent.timestamp();
    TimeWindow window = matchInterval(*(m_nodeTable[pFragment->node()]), *pFragment);
    if (window.startTime < window.endTime) {
      return (window.startTime <= triggerTime) && (triggerTime <= window.endTime);
    }
    else {
      return ((window.startTime <= triggerTime)  ||
	       (triggerTime <= window.endTime));
    }

  }
  else {
    return false;
  }
}
/*
   Remove fragments that are too old.  Too old is defined as 
   having a time difference that is more than TOO_OLD than
   m_currentTime
*/
void
EventBuilder::pruneFragments()
{

  // Prune the non trigger fragments:

  PhysicsFragmentList::iterator p = m_unmatchedFragments.begin();
  while (p != m_unmatchedFragments.end()) {
    PhysicsFragmentList::iterator next = p; // Since we may invalidate p.
    next++;
    if ((m_currentTime - (*p)->receivedTime) > TOO_OLD) {
      uint16_t node = (*p)->fragment->node();
      m_statistics.unmatchedByNode[node]++;
      m_statistics.discardedByNode[node]++;
      delete (*p)->fragment;
      delete (*p);
      m_unmatchedFragments.erase(p); // Invalidates p.
    }

    p= next;
  }
  // prune the trigger fragments.  Note that since state transitions can take
  // a significant amount of time, we never prune them.
  
  OutputPhysicsEvents::iterator i = m_inFlight.begin();
  while (i != m_inFlight.end()) {
    OutputPhysicsEvents::iterator next = i;
    next++; 			// We may invalidate i.

    AssemblyEvent* pTrigger = *i;
    if (pTrigger->isPhysics()) {
      if ((m_currentTime - pTrigger->receivedTime()) > TOO_OLD) {

	PhysicsAssemblyEvent* pPhysics = dynamic_cast<PhysicsAssemblyEvent*>(pTrigger);
	list<uint16_t> nodes = pPhysics->nodes();
	for (list<uint16_t>::iterator n = nodes.begin(); n != nodes.end(); n++) {
	  m_statistics.unmatchedByNode[*n]++;
	  m_statistics.discardedByNode[*n]++;
	}
	  
	delete pTrigger;
	m_inFlight.removeItem(i); // Invalidates p.
      }
    }
    i = next;
  }
}
