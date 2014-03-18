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
#ifndef __CFRAGMENTHANDLER_H
#define __CFRAGMENTHANDLER_H

#ifndef __CONFIG_H
#include <config.h>
#ifndef __CONFIG_H
#define __CONFIG_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_QUEUE
#include <queue>
#ifndef __STL_QUEUE
#define __STL_QUEUE
#endif
#endif

#ifndef __STL_MAP
#include <map>
#ifndef __STL_MAP
#define __STL_MAP
#endif
#endif

#ifndef __STL_SET
#include <set>
#ifndef __STL_SET
#define __STL_SET
#endif
#endif


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

#ifndef __CRT_TIME_H
#include <time.h>
#ifndef __CRT_TIME_H
#define __CRT_TIME_H
#endif
#endif

#ifndef __TCL_H
#include <tcl.h>
#ifndef __TCL_H
#define __TCL_H
#endif
#endif




// Forward definitions:

namespace EVB {
  struct _Header;
  typedef struct _FragmentHeader FragmentHeader, *pFragmentHeader;

  struct _FlatFragment;
  typedef struct _FlatFragment FlatFragment, *pFlatFragment;
  
  struct _Fragment;
  typedef struct _Fragment Fragment, *pFragment;
};
/**
 * @class CFragmentHandler
 *
 *   Singleton class which handles and dispatches fragments.
 *   The main provisions are for:
 *   - A way to add fragments to the input queues.
 *   - A set of observers that can be handed built events when they are
 *     produced.
 *   - A mechanism to force a 'flush-build'  A flush build is one which builds events
 *     until all queues are empty (think end of run or barriers).
 *   - Mechanisms to set the time tolerance of the build and the time window over which events
 *     are accumulated until a build is done.
 *
 * @note There is an assumption that the timestamps will not roll-over
 *       as they are 64 bits wide and even at 100Mhz that provides
 *       for over 50centuries of continuous operation without rollover.
 *       This puts a burden on event sources with narrower widths to 
 *       maintain the upper bits of the timestamp.u
 */
class CFragmentHandler
{
private:
  // Private data types:

  typedef struct _SourceQueue {
    uint64_t                                         s_newestTimestamp;
    std::queue<std::pair<time_t,  EVB::pFragment> > s_queue;

  } SourceQueue, *pSourceQueue;

  typedef std::map<uint32_t, SourceQueue> Sources, *pSources;
  typedef std::pair<uint32_t, SourceQueue> SourceElement, *pSourceElement;
  typedef std::pair<const uint32_t, SourceQueue> SourceElementV;
  typedef struct _BarrierSummary {
    std::vector<std::pair<uint32_t, uint32_t> > s_typesPresent;
    std::vector<uint32_t>                        s_missingSources;
  } BarrierSummary, *pBarrierSummary;
  
  // public data types:
public:
    typedef struct _QueueStatistics {
        uint32_t   s_queueId;
        uint32_t   s_queueDepth;
        uint64_t   s_oldestElement;
    } QueueStatistics, *pQueueStatistics;
    
    typedef struct _InputStatistics {
        uint64_t s_oldestFragment;
        uint64_t s_newestFragment;
        uint32_t s_totalQueuedFragments;
        
        std::vector<QueueStatistics> s_queueStats;
    } InputStatistics, *pInputStatistics;
  
public:

  // Observer base class:

  class Observer {
  public:
    virtual ~Observer() {}	// So we can chain destructors.
    
    
    
  public:
    virtual void operator()(const std::vector<EVB::pFragment>& event) = 0; // Passed built event gather maps.
  };

  // Observer for data late conditions:

  class DataLateObserver {
  public:
    DataLateObserver() {}
    virtual ~DataLateObserver() {} // Support destructor chaining.

  public:
    virtual void operator()(const ::EVB::Fragment& fragment,  uint64_t newest) = 0;
  };

  // Observer for successful barrier

  class BarrierObserver {
  public:
    BarrierObserver() {}
    virtual ~BarrierObserver() {}
  public:
    virtual void operator()(const std::vector<std::pair<uint32_t, uint32_t> >& barrierTypes) = 0;
  };

  class PartialBarrierObserver {
  public:
    PartialBarrierObserver() {}
    virtual ~PartialBarrierObserver() {}
  public:
    virtual void operator()(const std::vector<std::pair<uint32_t, uint32_t> >& barrierTypes, 
			    const std::vector<uint32_t>& missingSources) = 0;
  };


  // Queue statistics accumulator:
  
private:
    class QueueStatGetter {
    private:
      uint32_t                     m_nTotalFragments;
      std::vector<QueueStatistics> m_Stats;
    public:
      QueueStatGetter();
      void operator()(SourceElementV& source);
      uint32_t totalFragments();
      std::vector<QueueStatistics> queueStats();
    };

  

private:
  static CFragmentHandler* m_pInstance;	     //< The unique instance of this class.
private:
  uint64_t                     m_nOldest;              //!< Oldest fragment seen in terms of ticks.
  uint64_t                     m_nNewest;              //!< Newest fragment seen in terms of ticks.
  uint64_t                     m_nMostRecentlyPopped;    //!< Most recently popped fragment in ticks.

  time_t                       m_nBuildWindow;
  time_t                       m_nNow;
  time_t                       m_nOldestReceived;
  time_t                       m_nMostRecentlyEmptied;


  uint32_t                     m_nFragmentsLastPeriod; //!< # fragments in last flush check interval.

  std::list<Observer*>         m_OutputObservers;
  std::list<DataLateObserver*> m_DataLateObservers;
  std::list<BarrierObserver*>  m_goodBarrierObservers;
  std::list<PartialBarrierObserver*> m_partialBarrierObservers;

  Sources                      m_FragmentQueues;
  bool                         m_fBarrierPending;      //< True if at least one queue has a barrier event.
  std::set<uint32_t>           m_liveSources;	       //< sources that are live.
  std::map<std::string, std::list<uint32_t> > m_socketSources; //< Each socket name has a list of source ids.
  std::map<std::string, std::list<uint32_t> > m_deadSockets;   //< same as above but for dead sockets.




  // Canonicals/creationals. Note that since this is a singleton, construction
  // is private.

private:
  CFragmentHandler();
  ~CFragmentHandler();		// No need to be virtual since you can't derive this.

  // These are just plain illegal:

  CFragmentHandler(const CFragmentHandler&);
  CFragmentHandler& operator=(const CFragmentHandler&);
  int operator==(const CFragmentHandler&) const;
  int operator!=(const CFragmentHandler&) const;

  // The only public creational is getInstance:

public:
  static CFragmentHandler* getInstance();

  // here are the operations we advertised:

public:
  void addFragments(size_t nSize, EVB::pFlatFragment pFragments);

  void setBuildWindow(time_t windowWidth);
  time_t getBuildWindow() const;

  // Observer management:

public:

  void addObserver(Observer* pObserver);
  void removeObserver(Observer* pObserver);

  void addDataLateObserver(DataLateObserver* pObserver);
  void removeDataLateObserver(DataLateObserver* pObserver);

  void addBarrierObserver(BarrierObserver* pObserver);
  void removeBarrierObserver(BarrierObserver* pObserver);

  void addPartialBarrierObserver(PartialBarrierObserver* pObserver);
  void removePartialBarrierObserver(PartialBarrierObserver* pObserver);

  // queue management.

  void flush();
  
  // Get/set state of the queues etc.

  InputStatistics getStatistics();
  void createSourceQueue(std::string socketName, uint32_t id);  
  void markSourceFailed(uint32_t id);
  void markSocketFailed(std::string sockName);
  void reviveSocket(std::string sockName);
  void resetTimestamps();
  void clearQueues();

  // utility methods:

private:
  void flushQueues(bool completely=false);
  std::pair<time_t, ::EVB::pFragment>* popOldest();
  void   observe(const std::vector<EVB::pFragment>& event); // pass built events on down the line.
  void   dataLate(const ::EVB::Fragment& fragment);		    // Data late handler.
  void   addFragment(EVB::pFlatFragment pFragment);
  size_t totalFragmentSize(EVB::pFragmentHeader pHeader);
  bool   queuesEmpty();
  bool   noEmptyQueue();

  BarrierSummary generateBarrier(std::vector<EVB::pFragment>& outputList);
  void generateMalformedBarrier(std::vector<EVB::pFragment>& outputList);
  //   void generateCompleteBarrier(std::vector<EVB::pFragment>& ouptputList); 
  
  void goodBarrier(std::vector<EVB::pFragment>& outputList);
  void partialBarrier(std::vector<std::pair<uint32_t, uint32_t> >& types, 
		 std::vector<uint32_t>& missingSources);
  void observeGoodBarrier(std::vector<std::pair<uint32_t, uint32_t> >& types);
  void findOldest();
  size_t countPresentBarriers() const;


  SourceQueue& getSourceQueue(uint32_t id);


  void checkBarrier(bool complete);
  time_t oldestBarrier();

  // Static private methods:

  static void IdlePoll(ClientData obj);
};


#endif
