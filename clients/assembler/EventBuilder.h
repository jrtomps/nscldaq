#ifndef EVENTBUILDER_H_
#define EVENTBUILDER_H_
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

#ifndef __STL_LIST
#include <list>
#ifndef __STL_LIST
#define __STL_LIST
#endif
#endif

#ifndef __ASSEMBLERCOMMAND_H
#include "AssemblerCommand.h"
#endif



#ifndef __OUTPUTPHYSICSEVENTS_H
#include "OutputPhysicsEvents.h"
#endif

#ifndef __INPUTSTAGE_H
#include "InputStage.h"
#endif


class AssemblerOutputStage;
class PhysicsFragment;
class StateTransitionFragment;
class EventFragment;
class AssembledStateTransitionEvent;
class AssembledEvent;
class AssembledPhysicsEvent;
class InputStageCommand;

/*!
 *  This class is the core logical class for event building.
 * It establishes notification events on the event queues of an input
 * stage and uses them to process fragments as they become available.
 * Three sorts of events are assembled, in order of increasing difficulty:
  - Passthrough Events are events that don't actually require assembly at all.
    Examples include documentation events which are self-contained.
  - State transition events are assembled by type and, when assembled cause any
    in-flight events to be discarded.  One reason to discard, consider what the
    in-flight events would look like on a restart after a failure.
  - Physics events are assembled based on timing windows that are described in the
    configuration.  All events within a specific timing window of a trigger
    event are assembled into a single event.
    
    \param configuration  - object that manages the configuration.
    \param fragmentSource - object that is the source of event fragments.
    \param eventSink      - object that will accept assembled events.
 */
class EventBuilder
{
		// Exported types
public:
	typedef struct _Statistics {                   // Assembly statistics.
		uint32_t fragmentsByNode[0x10000];
		uint32_t eventsByType[0x100];
		uint32_t unmatchedByNode[0x10000];
		uint32_t discardedByNode[0x10000];
	} Statistics, *pStatistics;
	typedef struct _TimeWindow {
		uint32_t startTime;
		uint32_t endTime;
	} TimeWindow, *pTimeWindow;
private:
	InputStageCommand&             m_InputStageCommand;
	InputStage*                    m_pInputStage;
	
	AssemblerOutputStage&	m_sink;
	AssemblerCommand&		m_configuration;
	Statistics              m_statistics;
	
	// Things go faster if we reconstruct the cofiguration node table
	
	std::list<AssemblerCommand::EventFragmentContributor> m_nodeList;
	AssemblerCommand::EventFragmentContributor*            m_nodeTable[0x10000];
	OutputPhysicsEvents                                    m_inFlight;
public:
	EventBuilder(AssemblerCommand&     configuration,
		     InputStageCommand&    fragmentSource,
		     AssemblerOutputStage& eventSink);
	virtual ~EventBuilder();
private:
	EventBuilder(const EventBuilder& rhs);
	EventBuilder& operator=(const EventBuilder& rhs);
	int operator==(const EventBuilder& rhs) const;
	int operator!=(const EventBuilder& rhs) const;
public:
	// Object operations
	
public:
	static void  onInputStageEvent(void* pObject, 
				       InputStage::event why, 
				       uint16_t which );
	void clear();
	Statistics statistics() const;
	void reloadConfiguration();
	
	// Utilities:
private:
	void onFragments(uint16_t node);
	void onPhysicsFragment(uint16_t node, PhysicsFragment& fragment);
	void onStateTransitionFragment(uint16_t node, StateTransitionFragment& fragment);
	void onPassThroughFragment(uint16_t node, EventFragment& fragment);
	void commitAssembledPhysicsEvent(AssembledPhysicsEvent& event);
	void commitAssembledStateTransitionEvent(AssembledStateTransitionEvent& event);
	void commitPassthroughEvent(AssembledEvent& event);
	void onError(uint16_t node);
	void onShutdown();
	void checkMatchingFragments();
	void checkMatchingFragments(uint16_t node);
	void createTriggerAssemblies(uint16_t node);
	void pruneNonTriggerNodes();
	void pruneNode(uint16_t node);
	static TimeWindow matchInterval(AssemblerCommand::EventFragmentContributor& nodeInfo,
					PhysicsFragment&                            fragment);
			                 
};

#endif /*EVENTBUILDER_H_*/
