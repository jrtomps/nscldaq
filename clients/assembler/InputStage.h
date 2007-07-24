#ifndef __INPUTSTAGE_H
#define __INPUTSTAGE_H
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

#ifndef __STL_LIST
#include <list>
#ifndef __STL_LIST
#define __STL_LIST
#endif
#endif


#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#endif
#endif


// Forward class definitions.

class CTCLChannel;
class FragmentQueue;
class AssemblerCommand;
class EventFragment;

/*!
   The InputStage class handles the work of accepting buffers from 
   a pipe (spectcldaq), and creating a set of fragment queues, one for
   each node in our configuration.

   The software has a two states.  In active state, it is accepting data
   and building queues.  In inactive state, it has shutdown all the 
   channels connecting it to the data sources and is static.

   this allows the system to be safely configured after startup.
  
*/
class InputStage
{
  // public types:

public:
  typedef enum _event {
    NewFragments,
    ShuttingDown,
    Starting,
    Error
  } event;
  typedef void (*FragmentCallback)(void*, event, uint16_t);

  typedef pair<uint16_t, uint32_t> typeCountPair;

  // Private types:
private:
  typedef struct {
	  InputStage*   s_pObject;
	  CTCLChannel*  s_pChannel;
	  uint16_t      s_node;
  } InputReadyData, *pInputReadyData;
  
  // private data:

private:
  
	
  FragmentQueue*             m_queues[0x1000]; // A queue for each potential node.
  std::list<pInputReadyData> m_channels; 
  uint32_t                   m_fragmentCounts[0x1000];
  uint32_t                   m_typeCounts[0x1000];
  uint32_t                   m_nodeTypeCounts[0x1000][0x1000]; // [node][type].
  std::list<std::pair<FragmentCallback, void*> > m_callbackHandlers;


  AssemblerCommand*          m_pConfig;
  bool                       m_running;

public:
  InputStage(AssemblerCommand& config);
  ~InputStage();

  // Unimplemented canonicals.

private:
  InputStage(const InputStage&);
  InputStage& operator=(const InputStage&);
  std::list<AssemblerCommand::EventFragmentContributor>::iterator p =
    nodeList  int operator==(const InputStage&) const;
  int operator!=(const InputStage&) const;
public:

  // Functions:
 
  void start();
  void stop();
  bool isRunning() const;

  void clearStatisics();
  std::vector<typeCountPair> nodeFragmentCount()     const;
  std::vector<typeCountPair> perTypeFragmentCount(); const;
  std::vector<std::pair<uint16_t, std::vector<typeCountPair> > >
    nodePerTypeFragmentCount() const;
  

  void injectFragment(EventFragment* fragment);
  

  void addCallback(FragmentCallbackProc proc, void* clientData);
  vod removeCallback(FragmentCallbackProc proc, void* clientData);

  EventFragment* peek(uint16_t node);
  EventFragment* pop(uint16_t node);
  std::list<EventFragment*> clear(uint16_t node);

  static void onBuffer(ClientData clientData, int eventMask);

  void processPhysicsBuffer(uint16_t* pBuffer);
  void processStateChangeBuffer(uint16_t* pBuffer);
  void processStringlistBuffer(uint16_t* pBuffer);
  void processScalerBuffer(uint16_t* pBuffer);
  
  void processInputFailure(uint16_t* pPartialBuffer, int bytesRead,
		  				   uint16_t  node);
  
private:
  void startNode(const char* nodeName, uint16_t nodeId);
  void stopNode(uint16_t node);
  void emptyQueue(uint16_t node);

  // Declare events to callbacks.

  void declareEvent(event reason, uint16_t node);
  void declareStarting();
  void declareStopping();
  void declareError(uint16_t node);       // Foist errors on the callbacks.
  void declareNewFragments(uint16_t node);

  // Statistics utilities:

  std::vector<typeCountPair> makeTypeCountVector(const uint32_t* statistics, 
						 size_t          size);
  void updateCounters(uint16_t node, uint16_t type);
  
  static std::string spectclDaqPath();
  static std::string spectclDaqURL(const char* node);
};


#endif
