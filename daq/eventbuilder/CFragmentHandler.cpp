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

#include "CFragmentHandler.h"
#include "fragment.h"

#include <string.h>
#include <assert.h>

#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <time.h>
#include <stdint.h>


/*---------------------------------------------------------------------
** Static  data:
*/

CFragmentHandler* CFragmentHandler::m_pInstance(0);

static const time_t DefaultBuildWindow(20); // default seconds to accumulate data before ordering.
static const uint32_t IdlePollInterval(2);  // Seconds between idle polls.

/*---------------------------------------------------------------------
 * Debugging
 */

static void hexDump(uint8_t* p, uint32_t bytes) {
  for (int i = 0; i < bytes; i++) {
    unsigned value = *p++;
    std::cerr << value << " ";
    if (((i+1) & 0xf) == 0) {
      std::cerr << std::endl;
    }
  }
}

static void dumpFragment(EVB::pFragment p) {
  std::cerr << "------------------ Fragment -------------------\n";
  std::cerr << "Timestamp:    " << std::hex << p->s_header.s_timestamp << std::dec << std:: endl;
  std::cerr << "Source:       " << p->s_header.s_sourceId << std::endl;
  std::cerr << "PayloadSize:  " << std::hex << p->s_header.s_size << std::dec << std::endl;
  std::cerr << "Barrier type: " << p->s_header.s_barrier << std::endl;
  std::cerr << "Payload: \n";
  std::cerr << std::hex;
  hexDump(reinterpret_cast<uint8_t*>(p->s_pBody), p->s_header.s_size);
  std::cerr << std::dec << std::endl;

}

/*--------------------------------------------------------------------------
 ** Creationals: Note this is a singleton, constructors are private.
 */

/**
 * CFragmentHandler
 *
 * constructor:
 *   - m_nNewest -> 0, m_nOldest -> maxint64.
 *   - the build window and coincidence windows are set to some
 *     default values -- they should not be left that way.
 *   - m_pInstance -> this.
 */
CFragmentHandler::CFragmentHandler()

{
    m_nBuildWindow = DefaultBuildWindow;
    m_pInstance = this;
    resetTimestamps();

    m_nNow = time(NULL);	// Initialize the time.
    m_nOldestReceived = INT32_MAX; // Hopefully that makes it infinitely future.

    // Start the idle poll off:

    Tcl_CreateTimerHandler(1000*IdlePollInterval, &CFragmentHandler::IdlePoll, this);
}

/**
 * getInstance
 *
 * This is the public creational.  If m_pInstance doesn't exist,
 * a new CFragmentHandler is created.
 *
 * @return CFragmentHandler*
 * @retval - A pointer to the unique instance of the CFragmentHandler.
 *           note this may have been created by this call.
 */
CFragmentHandler*
CFragmentHandler::getInstance()
{
    // If needed, construct the object:
    
    if (!m_pInstance) {
        new CFragmentHandler();    // Constructor sets m_pInstance.
    }
    return m_pInstance;
}

/*--------------------------------------------------------------
 ** Object operations.
 */

/**
 * addFragments
 *   Adds a set of fragments to their queues.  The fragments
 *   are passed in as an array of flattened fragments with
 *   a total byte count for the fragments.
 *   - Fragments are put in the right queues.
 *   - Where appropriate, the newest/oldest events are updated.
 *   - If appropriate, events are built.
 *
 *   @param nSize      - Number of bytes worth of event fragment data.
 *   @param pFragments - Pointer to the first fragment.
 *
 *   @note This method is a no-op if nSize is 0.
 *   @throw std::string exception if there's an inconsistency between
 *          the fragment sizes and nSize so that fragments
 *          don't end when nSize is exactly zero.
 */
static  EVB::FragmentHeader lastHeader;
static bool first(true);

void
CFragmentHandler::addFragments(size_t nSize, EVB::pFlatFragment pFragments)
{
    m_nNow = time(NULL);
    if (m_nNow < m_nOldestReceived) m_nOldestReceived = m_nNow; // Really done first time.



    while (nSize) {
      EVB::pFragmentHeader pHeader = &(pFragments->s_header);
      size_t fragmentSize = totalFragmentSize(pHeader);
      if((pHeader->s_size + sizeof(EVB::FragmentHeader)) > nSize) {
	std::stringstream s;
	s << "Last fragment has too many bytes: " << nSize
	  << " bytes in fragment group but " << fragmentSize
	  << " bytes are in the last fragment!";
	throw s.str();
      }



      addFragment(pFragments);
      
      // Point to the next fragment.
      
      char* pNext = reinterpret_cast<char*>(pFragments);
      lastHeader = *pHeader;
      first      = false;

      pNext      += fragmentSize;
      pFragments  = reinterpret_cast<EVB::pFlatFragment>(pNext);
      nSize -= fragmentSize;
    }

    flushQueues();		// flush events with received time stamps older than m_nNow - m_nBuildWindow


}
/**
 * setBuildWindow
 * 
 * Set the build window.  The build window determines how far apart the
 * oldest and newest event can get in time before events are built.
 * 
 * @param windowWidth - seconds in the build window.
 */
void
CFragmentHandler::setBuildWindow(time_t windowWidth)
{
    m_nBuildWindow = windowWidth;
}
/**
 * getBuildWindow
 *
 * Return the value of the current build window.
 *
 * @return time_t - build window in seconds.
 */
time_t
CFragmentHandler::getBuildWindow() const
{
  return m_nBuildWindow;
}

/**
 * addObserver
 *
 * The fragment handler does not really  know how to deal with built
 * events.  Chunks of code (like an output stage) that want to know
 * about events that have been built register 'observer' objects
 * when an event has been built all registered observers are
 * called with a gather vector of pointer to the fragments that make
 * up the event passed as a parameter.
 *
 * This method registers an event built observer.
 *
 * @param pObserver - Pointer to the observer object to register.
 */
void
CFragmentHandler::addObserver(::CFragmentHandler::Observer* pObserver)
{
    m_OutputObservers.push_back(pObserver);
}
/**
 * removeObserver:
 *
 * Removes an observer that was registerd by addObserver
 * This method is a no-op if there are no such observers as
 *
 * @param pObserver - Pointer to the observer to remove.
 */
void
CFragmentHandler::removeObserver(::CFragmentHandler::Observer* pObserver)
{
    std::list<Observer*>::iterator p = find(
        m_OutputObservers.begin(), m_OutputObservers.end(), pObserver);
    if (p != m_OutputObservers.end()) {
        m_OutputObservers.erase(p);
    }
}
/**
 * addDataLateObserver
 *
 *  Add an observer to the list of objects that are invoked when 
 *  a data late event occurs See dataLate below for information about what
 *  that means.
 * 
 * @param pObserver - pointer to the  new observer.
 */
void
CFragmentHandler::addDataLateObserver(CFragmentHandler::DataLateObserver* pObserver)
{
  m_DataLateObservers.push_back(pObserver);
}
/**
 * removeDataLateObserver
 *   
 * Removes an existing data late observer from the observer list.
 * note that it is not an error to attempt to remove an observer that does
 * not actually exist.
 *
 * @param pObserver - Pointer to the observer to remove.
 */
void
CFragmentHandler::removeDataLateObserver(CFragmentHandler::DataLateObserver* pObserver)
{
  std::list<DataLateObserver*>::iterator p = find(
					  m_DataLateObservers.begin(), m_DataLateObservers.end(),
					  pObserver);
  if (p != m_DataLateObservers.end()) {
    m_DataLateObservers.erase(p);
  }
}
/**
 * addBarrierObserver
 *
 *  Barrier observers are called when a successful barrier has been 
 *  dispatched.  A successful barrier is one where all input queues
 *  have a barrier by the time timestamps for events following the
 *  barrier have gone past the build interval.
 *
 *  Observers are called passing a vector of pairs where each pair is
 *  a source Id and the type of barrier contributed by that source.
 *  It is really applciation dependent to know if mixed barrier types
 *  are errors, or normal.  No judgement is passed here.
 *
 * @param pObserver - Pointer to a BarrierObserver class which implements
 *                    operator()(std::vector<std::pair<uint32_t, uint32_t> >);
 */
void
CFragmentHandler::addBarrierObserver(CFragmentHandler::BarrierObserver* pObserver)
{
  m_goodBarrierObservers.push_back(pObserver);
}
/**
 * removeBarrierObserver
 *
 *  Remove a barrier observer from the set of observers that get called on a
 *  completed barrier.  It is a no-op to try to remove an observer that is not
 *  in the list.
 *
 * @param pObserver - pointer to the observer to remove.
 */
void
CFragmentHandler::removeBarrierObserver(CFragmentHandler::BarrierObserver* pObserver)
{
  std::list<BarrierObserver*>::iterator p = std::find(m_goodBarrierObservers.begin(),
						      m_goodBarrierObservers.end(), pObserver);
  if (p != m_goodBarrierObservers.end()) {
    m_goodBarrierObservers.erase(p);
  }
}
/**
 * addPartialBarrierObserver
 *
 * In the event a barrier is only partially made (some input queues are missing
 * their barrier fragments, a list of partial barrier observers is invoked.
 * This method adds a new partial barrier observer to the end of the list
 * of observers for this event.
 *
 * @param pObserver - Pointer to the observer to add.
 */
void
CFragmentHandler::addPartialBarrierObserver(CFragmentHandler::PartialBarrierObserver* pObserver)
{
  m_partialBarrierObservers.push_back(pObserver);
}
/**
 * removePartialBarrierObserver
 *
 *  Removes a partial barrier observer.
 *
 * @param pObserver - Partial barrier
 */
void
CFragmentHandler::removePartialBarrierObserver(CFragmentHandler::PartialBarrierObserver* pObserver)
{
  std::list<PartialBarrierObserver*>::iterator p = std::find(m_partialBarrierObservers.begin(),
							     m_partialBarrierObservers.end(), 
							     pObserver);
  if (p != m_partialBarrierObservers.end()) {
    m_partialBarrierObservers.erase(p);
  }
}

/**
 * flush
 * 
 * There come times when it is necessary to just build event fragments
 * until there are none left. This method does that.
 *
 */
void
CFragmentHandler::flush()
{

  flushQueues(true);
  m_nNewest = 0;
  m_nOldest = UINT64_MAX;
}

/**
 * getStatistics
 *
 *   Return information about the fragment statistics.
 *   This can be used to drive a GUI that monitors the status
 *   of the software.
 *
 *   @return CFragmentHandler::InputStatistics
 *   @retval struct that has the following key fields:
 *   - s_oldestFragment - The timestamp of the oldest queued fragment.
 *   - s_newestFragment - The timestamp of the newest queued fragment.
 *   - s_totalQueuedFragments - Total number of queued fragments.
 *   - s_queueStats     - vector of individual queue statistics.
 *                        each element is a struct of type
 *                        CFragmentHandler::QueueStatistics which has the fields:
 *                        # s_queueId - Source id of the queue.
 *                        # s_queueDepth - number of fragments in the queue.
 *                        # s_oldestElement - Timestamp at head of queue.
 */
CFragmentHandler::InputStatistics
CFragmentHandler::getStatistics()
{
    InputStatistics result;
    
    // get the individual chunks:
    
    result.s_oldestFragment = m_nOldest;
    result.s_newestFragment = m_nNewest;
    
    QueueStatGetter statGetter;
    
    statGetter = for_each(m_FragmentQueues.begin(), m_FragmentQueues.end(), statGetter);
    
    result.s_totalQueuedFragments = statGetter.totalFragments();
    result.s_queueStats           = statGetter.queueStats();
    
    
    return result;
}
/**
 * createSourceQueue
 *
 *  Creates a fragment queue for a source id.  This is called to set up 
 *  an  initial set of a-priori queues in order to make initial barrier
 *  handling simpler.  If the queue already exists, this is a no-op.
 *
 * @param[in] sockName - name of socket that has this id.
 * @param[in] id  - source id of the queue.
 */
void
CFragmentHandler::createSourceQueue(std::string sockName, uint32_t id)
{

  getSourceQueue(id);		// Creates too.
  m_liveSources.insert(id);		          // Sources start live.
  m_socketSources[sockName].push_back(id);
}

/**
 * markSourceFailed
 *
 * Marks a source as failed.  This just removes it from the live sources
 * set but maintains its input queue.  If there are dead sources,
 * - Barrier synchronization can proceed without them.
 * - All barriers synchronizations are considered incomplete.
 * 
 * @note receipt of a fragment from a source automatically makes it live again. 
 *
 * @param[in] id - Id of the source to mark, as dead.
 */
void
CFragmentHandler::markSourceFailed(uint32_t id)
{
  m_liveSources.erase(id);
  
  // If there's a pending barrier synchronization and all of the missing
  // sources are dead do an incomplete barrier.
  //
  if(m_fBarrierPending) {
    if (countPresentBarriers() == m_liveSources.size()) {	
      std::cerr << "markSourceFailed -- generating barrier on source dead\n";
      std::vector<EVB::pFragment> sortedFragments;
      generateMalformedBarrier(sortedFragments);
      observe(sortedFragments);
    }
  }
}
/** 
 * markSocketFailed
 *
 *  Mark a socket failed.:
 * - Locate the socket in the m_socketSources map
 * - declare each source in that sockets list failed via markSourceFailed
 * - enter the source list in the deadSockets map and 
 * - Kill it from the socketSources map.
 *
 * @param sockName - socket name.
 *
 * @note See reviveSocket below.
 */
void
CFragmentHandler::markSocketFailed(std::string sockName)
{
  std::map<std::string, std::list<uint32_t> >::iterator p = m_socketSources.find(sockName);
  if (p != m_socketSources.end()) {
    std::list<uint32_t>::iterator pSource = p->second.begin();
    while (pSource != p->second.end()) {
      markSourceFailed(*pSource);
      ++pSource;
    }

    m_deadSockets[sockName] = p->second;
    m_socketSources.erase(p);
  } else {
    std::string msg = sockName;
    msg += " is not a known socket to the fragment handler";
    throw msg;
  }
}
/**
 * 
 * reviveSocket
 *
 * Called to revive the sources associated with a socket.
 * - The socket is looked up in the m_deadSockets map.
 * - The sources for it are created.
 * - The source list is moved to the socketSources map.
 *
 * @param sockName - Name of the socket being revived.
 */
void
CFragmentHandler::reviveSocket(std::string sockname)
{

  std::map<std::string, std::list<uint32_t> >::iterator p = m_deadSockets.find(sockname);
  if (p != m_deadSockets.end()) {
    std::list<uint32_t>::iterator pSource = p->second.begin();
    while (pSource != p->second.end()) {
      SourceQueue& queue = getSourceQueue(*pSource);
      m_liveSources.insert(*pSource);		     // Sources start live.
      pSource++;
    }
    m_socketSources[sockname] = p->second;
  }
  else {
    std::string msg = sockname;
    msg += " not in the dead source list";
    throw msg;
  }
}

/**
 * resetTimestamps
 *   
 * Restes the timestamps to their contruction values.
 */
void
CFragmentHandler::resetTimestamps()
{
  m_nOldest = (UINT64_MAX);
  m_nNewest = (0);
  m_nMostRecentlyPopped = (0);
  m_nFragmentsLastPeriod = (0);
  m_fBarrierPending = (false);

  for (Sources::iterator p = m_FragmentQueues.begin(); p != m_FragmentQueues.end(); p++) {
    p->second.s_newestTimestamp = 0;
  }
}
      
/**
 * clearQueues
 *
 *  Clears all knowledge of data sources. Specifically:
 *  - m_deadSockets is cleared.
 *  - m_socketSources is cleared.
 *  - m_liveSources is cleared.
 *  - m_FragmentQueues all fragments are deleted, the queues and the
 *    map itsel are cleared.
 *  - The m_fBarrierPending flag is cleared.
 *
 */
void
CFragmentHandler::clearQueues()
{
  m_deadSockets.clear();
  m_socketSources.clear();
  m_liveSources.clear();
  m_fBarrierPending = false;

  // Get rid of all queued fragments:

  for (Sources::iterator s = m_FragmentQueues.begin(); s != m_FragmentQueues.end(); s++) {
    SourceQueue& q(s->second);
    while (!q.s_queue.empty()) {
      delete q.s_queue.front().second;
      q.s_queue.pop();
      
    }
  }
  m_FragmentQueues.clear();
}

/*---------------------------------------------------------------------
 ** Utility methods (private).
 */

/**
 * flushQueues
 *
 *  Flush the output queues to the observers.  By default, this flushes
 *  queues until the oldest queue element is 'too new' to flush.
 *  If, however completly is true, queues are fluhsed until empty.
 *
 * @param completely - If true, queues are flushed until empty.
 *                     otherwise, the build window and m_nNow determine
 *                     when the flush stops.
 *                     Once the events are ordered into a vector, the
 *                     observers are called to deal with them
 *                     and storage associated with them deleted.
 */
void
CFragmentHandler::flushQueues(bool completely)
{
  m_nNow = time(NULL);


  // Ensure there's at least one fragment available:


  std::vector<EVB::pFragment> sortedFragments;
  while (noEmptyQueue() // || (m_nNow - m_nOldestReceived > m_nBuildWindow)
	  || completely ) {
    if (queuesEmpty()) break;	// Done if there are no more frags.
    std::pair<time_t, ::EVB::pFragment>* p = popOldest();
    if (p) {
      sortedFragments.push_back(p->second);
      delete p;
      if ((m_nNow - m_nOldestReceived <= m_nBuildWindow) && (!completely)) {
	break;		// No more genuine fragments
      }
    } else {
      break;		// If there are more fragments they are barriers.
    }
  }
  // Observe the fragments we have now:
  
  observe(sortedFragments);
  
  // If a barrier is pending check it and, if the flush was complete,
  // tail call to continue building:
  
  if(m_fBarrierPending) {
    checkBarrier(completely);	// Complete forces barriers out.
  }
  // If we still have non-empty queues and were asked to completely flush
  // tail call:
  
  if (!queuesEmpty() && completely) {
    flushQueues(completely);
  }
  findOldest();			// Ensure we know which the oldest is.
}
    

/**
 * popOldest
 *
 *   Remove an oldest fragment from the sources queue and update m_nOldest
 *
 *   @return ::EVB::pFragment - pointer to a fragment whose timestamp
 *           matches m_nOldest
 *   @retval - Null is returned if there are no non-barrier events in the
 *             queues.
 *
 *   @note this is all very brute force.  A much quicker algorithm to find
 *         the oldest fragment would be to retain in addtion to m_nOldest
 *         the queue that it was put in...however we still need to iterate
 *         over the queues to update m_nOldest.  This is a bit
 *         short circuited by:
 *         - Keeping track of it as we search for the first match to m_nOldest
 *         - short circuiting the loop if we find another queue with
 *           an m_nOldest match as there can't be anything older than that
 *           by definition.
 */
std::pair<time_t, ::EVB::pFragment>*
CFragmentHandler::popOldest()
{
    uint64_t nextOldest = UINT64_MAX;   // Must be older than that.
    time_t   nextReceived = m_nNow;      // Next most recently received.
    std::pair<time_t, ::EVB::pFragment>* pOldest(0);

    /*
       This loop does 2 things:  Find the first fragment that
       has a timestamp that matches m_nOldest and set it to
       pOLdest, and find the next value for m_nOldest
    */

    std::vector<uint32_t> sourceIds;
    std::vector<uint64_t> timestampValues;

    int nonEmptyQs = 0;
    for(Sources::iterator p = m_FragmentQueues.begin();
        p != m_FragmentQueues.end(); p++) {
      if (!p->second.s_queue.empty()) {
	nonEmptyQs++;

	std::pair<time_t, ::EVB::pFragment> Frag = p->second.s_queue.front();

	sourceIds.push_back(p->first);
	timestampValues.push_back(Frag.second->s_header.s_timestamp);

	// Zero timestamps at the front of a queue shouild be pretty strange

#ifdef DEBUG
	if (Frag.second->s_header.s_timestamp == 0) {
	  std::cerr << "Front of a queue has timestamp of zero!!\n";
	  dumpFragment(Frag.second);
	}
#endif
	// We can only process non-barriers.
	
	if (Frag.second->s_header.s_barrier == 0) {

	  uint64_t stamp = Frag.second->s_header.s_timestamp;
	  time_t   received = Frag.first;


	  /* This condition is met for the first queue whose
	     head fragment has a timestamp == m_nOldest.
	  */

	  if (stamp < m_nOldest) {
	    std::cerr << "Found a time stamp older than m_nOldest!!\n";
	    std::cerr << std::hex << "Oldest: " << m_nOldest
		      << " Stamp: " << stamp << std::dec << std::endl;

	  } else if (((stamp - m_nOldest) > 0x100000000ll) && !m_fBarrierPending) {
	    std::cerr << "Element at front of queue " << p->first 
		      << " look like a big skip: " << std::hex
		      << stamp << " : " << m_nOldest << std::dec << std::endl;
	  }

	  if((stamp == m_nOldest) && !pOldest) {
	    
	    /*  This is the one we will return, so save it as pOldest
	        and remove it from the queue.
	    */
	    pOldest  = new std::pair<time_t, EVB::pFragment>(Frag);
	    p->second.s_queue.pop();
#ifdef DEBUG
	    if (pOldest->second->s_header.s_timestamp == 0ll) {
	      std::cerr << "Popped a tstamp 0 fragment: \n";
	      dumpFragment(pOldest->second);
	    }
#endif
	    /* It's possible the next fragment in the queue
	       also has the smallests remaining timestamp
	       so we need to grab it for the comparisons at
	       the bottom of the loop.
	    */

	    if(!p->second.s_queue.empty()) {
	      Frag = p->second.s_queue.front();
	      
	      received = Frag.first;
	      stamp = Frag.second->s_header.s_timestamp;
	      sourceIds.push_back(p->first);
	      timestampValues.push_back(stamp);
	    
	      /* If the current queue is empty, that means that
		 the current fragment is 'the one' and it emptied
		 out its queue.  We don't want to update the times
		 in that case because by definition that will be
		 oldest not next oldest...and will prevent us from getting
		 the 'right' next oldest.
	      */
	      
	    }
	  }
	  if ((stamp < nextOldest) && 
	      (Frag.second->s_header.s_barrier == 0)) nextOldest = stamp;
	  if (received < nextReceived) nextReceived = received;
	  
	  /* This is only true if we found a second fragment with the
	     same timestamp as m_nOldest.  In that case we don't
	     have to process any more queues because m_nOldest won't be
	     modified.
	  */
	  
	  if (nextOldest == m_nOldest) break;
	} else {
	  m_fBarrierPending = true;	// There's at least on barrier fragment.
	}
      }
    }

    /*
      If we found an oldest segment we are done and can update all of the times
      etc.
    */

    if (pOldest) {
#ifdef DEBUG
      if (nonEmptyQs < 2) {
	std:: cerr << "Not all queues had data: new oldest: " 
		   << std::hex << nextOldest << std::endl;
      }
#endif
      if ((nextOldest - m_nOldest) > 0x100000000ll && (nextOldest != NULL_TIMESTAMP)) {
	std::cerr << "CFragmentHandler::popOldest next oldest is way in the future: "
		  << std::hex << nextOldest <<  " : " << m_nOldest  << std::endl;

	std::cerr << "Oldest timestamp found was: " << pOldest->second->s_header.s_timestamp 
		  << std::endl;
	for (int i =0; i < sourceIds.size(); i++) {
	  std::cerr << "Source ID: " << sourceIds[i] << " Timestamp: " << timestampValues[i] 
		    << std::endl;
	}

	std::cerr << std::dec;
      }
#ifdef DEBUG
      if (nextOldest < m_nOldest) {
	std::cerr << "Oldest decreasing from " << std::hex << m_nOldest
		  << "  to " << nextOldest << std::dec << std::endl;
      }
#endif
      m_nMostRecentlyPopped = m_nOldest;
      m_nOldest = nextOldest;
      m_nOldestReceived = nextReceived;
  
    }
    /*
       If we could not find one and there's no barrier pending, recompute m_nOldest and 
       try again... that may result in a null return in which case we have nothing left
       to pop.
    */
    if (!pOldest) {
#ifdef DEBUG
      std::cerr<< "Findoldest\n";
#endif
      findOldest();
#ifdef DEBUG
      std::cerr << "back popoldest recursion\n";
#endif
      if (!m_fBarrierPending) {
	return popOldest();
      }

    }
#ifdef DEBUG
    if (m_nOldest == 0) {
      std::cerr << "popOldest assigned oldest => 0\n";
    }
    if (!pOldest) {
      std::cerr << "No queue had front matching "
		<< std:: hex << m_nOldest << std::dec << "... or all batrriers\n";
    }
#endif
    return pOldest;
}

/**
 * observe
 *
 * Invoke each observer for the event we've been passed.
 *
 * @param event - Gather vector for the event.
 *
 * @note - the storage associated with the fragments in the vector
 *         is released after all observers have been called.
 *         If observers want to maintain fragments they will therefore need
 *         to copy them.
 */
void
CFragmentHandler::observe(const std::vector<EVB::pFragment>& event)
{
    std::list<Observer*>::iterator p = m_OutputObservers.begin();
    while(p != m_OutputObservers.end()) {
        Observer* pObserver = *p;
        (*pObserver)(event);
        p++;
    }
    // Delete the fragments in the vector as we're done with them now:

    for(int i =0; i < event.size(); i++) {
      freeFragment(event[i]);
    }
}
/**
 * dataLate
 * 
 *  This method is called by addFragment in the event a fragment timestamp
 *  is older than m_nNewest by more than m_nBuildWindow, indicating that
 *  the data should have been output earlier. At this time,
 *  the data has not yet been put in the data source queue.
 *  We will pass the fragment along with the value of the largest timestamp
 *  to each element of the m_DataLateObservers list.
 *
 * @param fragment - Reference to the fragment that is late.
 */
void 
CFragmentHandler::dataLate(const ::EVB::Fragment& fragment)
{
  std::list<DataLateObserver*>::iterator p = m_DataLateObservers.begin();
  while (p != m_DataLateObservers.end()) {
    DataLateObserver* pObserver = *p;
    (*pObserver)(fragment, m_nNewest);
    p++;
  }

}
/**
 * addFragment
 *
 * Add a single event fragment to the appropriate event queue.
 * The fragment is copied so that ownership of the parameter remains
 * with the caller.  allocateFragment is used to create the fragment
 * storage adnfreeFragment should be used to release that storage
 * (and that is done by buildEvent() typically).
 *
 * @param pFragment - Pointer to the flattened fragment.
 * 
 * @note This method can also alter the value of m_nNewest if its
 *       timestamp says it is the newest fragment.
 */
void
CFragmentHandler::addFragment(EVB::pFlatFragment pFragment)
{
    bool     assigned            = false;
    uint64_t priorOldest         = m_nOldest;

    m_nFragmentsLastPeriod++;	//  We were not idle.

    // Allocate the fragmentand copy it:
    
    EVB::pFragmentHeader pHeader = &pFragment->s_header;
    EVB::pFragment pFrag         = allocateFragment(pHeader); // Copies the header.
    uint64_t timestamp           = pHeader->s_timestamp;
    bool     isBarrier           = pHeader->s_barrier != 0;

    memcpy(pFrag->s_pBody, pFragment->s_body, pFrag->s_header.s_size);

    SourceQueue& destQueue(getSourceQueue(pHeader->s_sourceId));


    // If the timestamp is null, assign the newest timestamp from that source to it:

    if ((timestamp == NULL_TIMESTAMP)) {
      assigned = true;
      timestamp = destQueue.s_newestTimestamp;
      pFrag->s_header.s_timestamp = timestamp;
      assigned = true;
#ifdef DEBUG
      std::cerr << "Assigned timestamp " << std::hex << timestamp << std::dec << std::endl;
#endif
    } else {
      if ((pFrag->s_header.s_timestamp - timestamp) > uint64_t(0x100000000ll)) {
	std::cerr << "orderer: Big timestamp jump last: 0x" << std::hex
		  << timestamp << " Next: 0x" << pFrag->s_header.s_timestamp << std::endl;
      }
    }
    destQueue.s_newestTimestamp = timestamp; // for sure the newest 

    
   // Get a reference to the fragment queue, creating it if needed:
    
    destQueue.s_queue.push(std::pair<time_t, EVB::pFragment>(m_nNow, pFrag));
    m_liveSources.insert(pHeader->s_sourceId); // having a fragment makes a source live.
    
    // update newest/oldest if needed -- and not a barrier:
    // 
    // Since data can come out of order across sources it's possible
    // to update oldest as well as newest.

    // If the timing of receiving this fragment would result in an
    // out of order fragment, it's late:
    //
    if(timestamp < m_nMostRecentlyPopped) {
      dataLate(*pFrag);
    }
    if ((timestamp < m_nOldest)) {
	if ((m_nOldest - timestamp) > 0x100000000ll) {
	  std::cerr << "addFragment... timestamp taking a big step back from : " << std::hex
		    << m_nOldest << " to " << timestamp << std::dec << std::endl;
	}
#ifdef DEBUG
	std::cerr << "add fragment decreasing oldest timestamp from: "
		  << std::hex << m_nOldest << " to: " 
		  << timestamp << std::dec << std::endl;
#endif
	m_nOldest = timestamp;
    }
    // Queue out of order fragmen?
    if (!destQueue.s_queue.empty()) {
      if (timestamp < destQueue.s_newestTimestamp) {
	std::cerr << "Queue timestamp took a jump backwards from : "
		  << std::hex << destQueue.s_newestTimestamp
		  << " to " << timestamp << std::dec << std::endl;
      }
    }
    
    if (timestamp > m_nNewest) m_nNewest = timestamp;
#ifdef DEBUG

    if ((priorOldest != m_nOldest) && (m_nOldest == 0)) {
      std::cerr << "addFragment assigned m_nOldest -> 0 was: " 
		<< std::hex << priorOldest << std::dec << std::endl;
      dumpFragment(pFrag);
      
    }
#endif      

}
/**
 * totalFragmentSize
 *
 * Computes the total size of a fragment.  This is just the size
 * of the payload + size of the header.
 *
 * @param pHeader - Pointer to an event builder fragment header.
 *
 * @return size_t
 * @retval total size of fragment descsribed by the header.
 */
size_t
CFragmentHandler::totalFragmentSize(EVB::pFragmentHeader pHeader)
{
    return pHeader->s_size + sizeof(EVB::FragmentHeader);
}

/**
 * queuesEmpty
 *   Returns true if all data source queues are empty.
 *
 *  @return bool
 *
 *  @note we don't use for_each as short of tossing an exception
 *        there's no way to short circuit the loop.
 */
bool
CFragmentHandler::queuesEmpty()
{
    for(Sources::iterator p = m_FragmentQueues.begin();
        p != m_FragmentQueues.end(); p++) {
        if (!p->second.s_queue.empty()) return false;
    }
    return true;
}
/**
 * noEmptyQueue
 *
 *  @return bool 
 *  @retval true if all queues have some data.
 *  @retval false if at least one queue is empty.
 */

bool
CFragmentHandler::noEmptyQueue()
{
    for(Sources::iterator p = m_FragmentQueues.begin();
        p != m_FragmentQueues.end(); p++) {
        if (p->second.s_queue.empty()) return false;
    }
    return true;
}
/*-----------------------------------------------------------
 ** Locally defined classers
 */
/**
 * @class QueueStateGetter
 *
 * Event source queue visitor that gathers input statistics.
 * This class is a functional and is intended to be called from
 * a for_each loop over the set of input queues.
 * It gathers the total number of fragments as well
 * as the number of fragments in the queue and the oldest fragment in the queue.
 */

/**
 * Construction sets the total fragment count to zero.
 */
CFragmentHandler::QueueStatGetter::QueueStatGetter() :
  m_nTotalFragments(0)
{}


/**
 * operator()
 *   Called for each queue to accumulate stats for that queue.
 *
 *  @param source - reference to that data source queue.
 */
void
CFragmentHandler::QueueStatGetter::operator()(SourceElementV& source)
{
    SourceQueue&  sourceQ(source.second);    
    QueueStatistics stats;
    stats.s_queueId       = source.first;
    stats.s_queueDepth    = sourceQ.s_queue.size();
    stats.s_oldestElement = stats.s_queueDepth ?sourceQ.s_queue.front().second->s_header.s_timestamp :
      0;
    
    m_nTotalFragments += stats.s_queueDepth;
    m_Stats.push_back(stats);
}
/**
 * totalFragments()
 * 
 * Return the total number of queued elements.
 *
 * @return uint32_t
 */
uint32_t
CFragmentHandler::QueueStatGetter::totalFragments()
{
    return m_nTotalFragments;
}
/**
 * queueStats
 *
 * Return the queue statistics vector:
 *
 * @return std::vector<QueueStatistics>
 *
 */
std::vector<CFragmentHandler::QueueStatistics>
CFragmentHandler::QueueStatGetter::queueStats()
{
    return m_Stats;
}
/**
 * generateBarrier
 *   
 *  Called to remove all barriers from the fronts of source
 *  queues and add them to the output event list.
 *  
 * @param outputList - the output list.  This is an output parameter and
 *                     we do it this way to avoid copy construction of the whole
 *                     vector.
 *
 * @return BarrierSummary
 * @retval A summary of the barrier fragment types for the sources that
 *         contributed and the sources that did not have a barrier rarin' to go.
 *
 */
CFragmentHandler::BarrierSummary
CFragmentHandler::generateBarrier(std::vector<EVB::pFragment>& outputList)
{
  // Iterate through the output queues and add any
  // barrier events to the outputList.


  BarrierSummary result;

  std::cerr << "Generating a barrier\n";
  
  for (Sources::iterator p = m_FragmentQueues.begin(); p!= m_FragmentQueues.end(); p++) {
    if (!p->second.s_queue.empty()) {
      ::EVB::pFragment pFront = p->second.s_queue.front().second;
      if (pFront->s_header.s_barrier) {
	outputList.push_back(pFront);
	p->second.s_queue.pop();
	result.s_typesPresent.push_back(
            std::pair<uint32_t, uint32_t>(p->first, pFront->s_header.s_barrier)
        );
      } else {
	std::cerr << "Missing barrier fragment from queue: " << p->first << std::endl;
	result.s_missingSources.push_back(p->first);
      }
    } else {
      std::cerr << "Empty queue in barrier check: " << p->first << std::endl;
      result.s_missingSources.push_back(p->first); // just as missing if the queue is empty.
    }
  }
  std::cerr << "outputting a barrier\n";  
  m_fBarrierPending = false;
  findOldest();

  return result;
  
}
/**
 * generateMalformedBarrier
 *
 *  Called when we have finished processing output events but there is an 
 *  incomplete barrier. *  this is an error...but we need to flush those
 *  fragments.
 *
 * @param outputList - Output fragment list (see above).
 */
void
CFragmentHandler::generateMalformedBarrier(std::vector<EVB::pFragment>& outputList)
{
  std::cerr << "Was asked to make a malformed barrier\n";
  BarrierSummary bs = generateBarrier(outputList);
 
  partialBarrier(bs.s_typesPresent, bs.s_missingSources);

}
/**
 * goodBarrier
 *
 *   Generate a complete barrier and fire the observers associated with them.

 * @param outputList - Output fragment list (see above).
 */
void
CFragmentHandler::goodBarrier(std::vector<EVB::pFragment>& outputList)
{
  BarrierSummary bs = generateBarrier(outputList);

  // If there's a non empty missing sources this is a partial barrier actually.

  if (bs.s_missingSources.empty()) {
    observeGoodBarrier(bs.s_typesPresent);
  } else {
    std::cerr << "Though I had a good barrier but there are missing sources\n";
    partialBarrier(bs.s_typesPresent, bs.s_missingSources);
  }
}
/**
 * findOldest
 *
 * When a barrier (even a partial one) we may not have a correct value for
 * m_nOldest.  This method re-determines the oldest fragment by examing all
 * nonempty fragment queue's front element.  The m_nOldest is set to the timesstamp
 * in the oldest fragment or to m_nNewest if all queues are emtpy.
 */
void
CFragmentHandler::findOldest()
{
  uint64_t oldest = UINT64_MAX;		// Automatically right if all queues are empty.

  for (Sources::iterator p = m_FragmentQueues.begin(); p != m_FragmentQueues.end(); p++) {
    if (!p->second.s_queue.empty()) {
      ::EVB::pFragment pf = p->second.s_queue.front().second;
      if ((pf->s_header.s_timestamp < oldest) && (pf->s_header.s_barrier ==0)) { // nonbarriers only counted
#ifdef DEBUG
	std::cerr << "Find oldest changing from "
		  << std::hex << oldest << " to " << pf->s_header.s_timestamp
		  << std::dec << std::endl;
	if (oldest == 0) {
	  std:: cerr << "That's zero!! \n";
	  dumpFragment(pf);
	}
#endif
	oldest = pf->s_header.s_timestamp;
      }
    }
  }
  if (oldest != UINT64_MAX) {
    m_nOldest = oldest;
  }
#ifdef DEBUG
  if (m_nOldest == 0) {
    std::cerr << "findoldest found 0\n";
  }
#endif
  

}
/**
 * goodBarrier
 *  
 * Fire off all of the good barrier observers.
 *
 * @param types - Vector of pairs. Each pair contains, in order, the data source ID
 *                and the barrier event type committed to output for that data source.
 */
void
CFragmentHandler::observeGoodBarrier(std::vector<std::pair<uint32_t, uint32_t> >& types)
{

  for (std::list<BarrierObserver*>::iterator p = m_goodBarrierObservers.begin();
       p != m_goodBarrierObservers.end(); p++) {
    (*p)->operator()(types);
  }

}
/**
 * partialBarrier:
 * 
 * Fire off all partial barrier observers.
 *
 * @param types - Vector of pairs as described in goodBarrier above.
 * @param missingSources - Vector of source ids that did not have a barrier.
 */
void
CFragmentHandler::partialBarrier(std::vector<std::pair<uint32_t, uint32_t> >& types, 
				 std::vector<uint32_t>& missingSources)
{

  for (std::list<PartialBarrierObserver*>::iterator p = m_partialBarrierObservers.begin();
       p != m_partialBarrierObservers.end(); p++) {
    (*p)->operator()(types, missingSources);
  }

}
/**
 * countPresentBarriers
 *
 *  Counts the number of queues with barriers that are at their heads.
 *  
 * @return size_t - number of fragment queues that have barriers at their head.
 */
size_t
CFragmentHandler::countPresentBarriers() const
{
  size_t count(0);
  for (Sources::const_iterator p = m_FragmentQueues.begin(); p != m_FragmentQueues.end(); p++) {
    const SourceQueue& queue(p->second);
    if (!queue.s_queue.empty()) {
      EVB::pFragment pFront = queue.s_queue.front().second;
      if (pFront->s_header.s_barrier) count++;
    }
  }
  return count;
}
/**
 * get the source queue associated with an id, creating it if needed
 *
 * @param id - the id of the source.
 * 
 * @return SourceQueue& reference to the (possibly new) source queue.
 */
CFragmentHandler::SourceQueue&
CFragmentHandler::getSourceQueue(uint32_t id)
{
  Sources::iterator p = m_FragmentQueues.find(id);
  if (p  == m_FragmentQueues.end()) {	       // Need to create.
    SourceQueue& queue = m_FragmentQueues[id]; // Does most of the creation.
    queue.s_newestTimestamp = m_nNewest;	       // Probably the best initial value.
    return queue;
  }  else {			              // already exists.
    return p->second;
  }
}

/**
 * checkBarrier
 *   See if we can emit a barrier after sorted fragments have been emitted fromt he queues.
 *   If a complete flush is underway and the queues that don't have barriers at the front
 *   are empty, that constitutes an incomplete barrier even if timing says we shouild wait
 *   for more fragments. Procesing assumes that barrier processing is infrequent:
 *
 * @param completeFlush - true if a complete flush is underway.  
 */
void
CFragmentHandler::checkBarrier(bool completeFlush)
{
  std::vector<EVB::pFragment> outputList;
  m_nNow = time(NULL);		// Update the time.
  size_t nBarriers = countPresentBarriers();


  // Check for all queues having barriers:
  if (nBarriers == m_FragmentQueues.size()) {
    goodBarrier(outputList);
    observe(outputList);
    findOldest();
    return;
  }
  // If we got here and a complete flush is requested, we must have an incomplete barrier:

  if (completeFlush) {
    std::cerr << "Complete flush requested ..malformed barrier\n";
    generateMalformedBarrier(outputList);
    observe(outputList);
    findOldest();
    return;
  }
  // If the least recently received barrier is older than the build window than
  // m_now we also have a malformed barrier:

  if ((nBarriers != 0) && ((m_nNow - oldestBarrier()) > (m_nBuildWindow*4))) {
    std::cerr << "Generating malformed barrier oldest received: "
	      << std::hex << oldestBarrier() 
	      << " unix time(m_nNow) " << m_nNow << std::dec << std::endl;
    generateMalformedBarrier(outputList);
    observe(outputList);
  }
  // Otherwise we can wait for more fragments to come in before making any decisions.
  // 
  findOldest();			// Ensure that we know which frag is still the oldest.
}
/**
 * oldestBarrier
 *
 *  Determines which barrier was received earliest in time.
 *
 *  @return time_t
 *  @retval oldest barrier.  If there are no barriers, m_nNow is returned.
 */
time_t
CFragmentHandler::oldestBarrier()
{
  time_t result = m_nNow;

  for (Sources::iterator p = m_FragmentQueues.begin(); p != m_FragmentQueues.end();
       p++) {
    if (!p->second.s_queue.empty()) {
      std::pair<time_t, ::EVB::pFragment> Frag = p->second.s_queue.front();
      if ((Frag.first < result) && (Frag.second->s_header.s_barrier != 0)) {
	result = Frag.first;
      }
    }
  }
  return result;
}
  
/*--------------------------------------------------------------------------------------
 * Static methods
 */

/**
 * IdlePoll
 *
 * If data are not getting pushed through we periodically call flushQueues to ensure
 * the last dribs of data are sent..this also helps if data rates are low.
 *
 * method also resschedules itselfr.
 *
 * @param obj - Pointer to the singleton.
 */
void
CFragmentHandler::IdlePoll(ClientData data)
{
  CFragmentHandler* pHandler = reinterpret_cast<CFragmentHandler*>(data);

  if (!pHandler->m_nFragmentsLastPeriod) {
    pHandler->m_nNow = time(NULL);	// Update tod.
    pHandler->findOldest();		// Update oldest fragment time.
    pHandler->flushQueues();
  } else {
    pHandler->m_nFragmentsLastPeriod = 0;
  }

  // reschedule

  Tcl_CreateTimerHandler(1000*IdlePollInterval,  &CFragmentHandler::IdlePoll, pHandler);
}
