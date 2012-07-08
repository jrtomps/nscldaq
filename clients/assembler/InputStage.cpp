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
#include "InputStage.h"
#include <TCLChannel.h>
#include "FragmentQueue.h"
#include "EventFragment.h"
#include "AssemblerWrongStateException.h"
#include "InvalidNodeException.h"

#include "PhysicsFragment.h"
#include "StateTransitionFragment.h"
#include "StringListFragment.h"
#include "ScalerFragment.h"


#include "AssemblerCommand.h"
#include "InputBufferFactory.h"
#include "InputBuffer.h"


#include <buftypes.h>
#include <buffer.h>

#include <algorithm>

#include <string.h>
#include <netdb.h>
#include <stdio.h>



using namespace std;


static const string PATHSEP("/");

// Predicate to match a channel list entry with a node.
//
static 
bool matchChannelNode(InputStage::pInputReadyData item, uint16_t node)
{
	return item->s_node == node;
}

/////////////////////////////////////////////////////////////////////////
/*!
  Create the input stage.
  The input stage is created in the stopped stage.
  All queues are null pointers,  statistics are cleared etc.

  \param config  - The assembler input stage that will be used to
                   configure the object when assembly is started.

*/
InputStage::InputStage(AssemblerCommand& config) :
  m_pConfig(&config),
  m_running(false)
{
  // Clear the fragment queues:

  memset(m_queues, 0, sizeof(m_queues));

  // clear the statistics:

  clearStatistics();

}
//////////////////////////////////////////////////////////////////////
/*!
   Destroy the input stage (normally this won't be done, the input
   stage will live for the lifetime of the program).
*/
InputStage::~InputStage()
{
  if (isRunning()) {
    stop();			// This cleans up a lot of stuff.
  }                             // and notifies clients.

}

/////////////////////////////////////////////////////////////////////////
/*!
  Start the assembly process.  This is done by processing the
  configuration in the assembler command we were constructed with.
  For each node that is active the node is started.

*/
void
InputStage::start()
{

  // Throw if we're already running:

  if (m_running) {
    throw AssemblerWrongStateException(AssemblerWrongStateException::active,
				       string("Start attempted"),
				       string("InputStage::start already active"));
  }
  else {
    // Start each node in the list, then mark us started and return:
    
    list<AssemblerCommand::EventFragmentContributor> nodeList =
      m_pConfig->getConfiguration();
    
    
    while(!nodeList.empty()) {
      AssemblerCommand::EventFragmentContributor& node(nodeList.front());
      startNode(node.pNodeName, node.cpuId);
      nodeList.pop_front();
    }
    m_running = true;
    declareStarting();		// Invoke the user callbacks.
  }
}
//////////////////////////////////////////////////////////////////////
/*!
   Stop assembly.. 
   - The data sources are shutdown with their callbacks removed.
   - The stop is declared as an event invoking the callbacks that are registered.
   - The queues are emptied.
*/
void
InputStage::stop()
{
  if (!m_running) {
    // Already stopped:

    throw AssemblerWrongStateException(AssemblerWrongStateException::inactive,
				       string("Stop attempted"),
				       string("InputStage::stop already stopped"));
  }
  else {
    while (!m_channels.empty()) {
      pInputReadyData info(m_channels.front());
      stopNode(info->s_node);     // Also removes it from the list.
    }
    declareStopping();

    m_running = false;
  }
}

////////////////////////////////////////////////////////////////////
/*!
   Returns true if the input stage is running.
*/
bool
InputStage::isRunning() const
{
  return m_running;
}
//////////////////////////////////////////////////////////////////
/*!
    Clear all the statistics counters:
*/
void
InputStage::clearStatistics()
{
  memset(m_fragmentCounts, 0, sizeof(m_fragmentCounts));
  memset(m_typeCounts, 0, sizeof(m_typeCounts));
  memset(m_nodeTypeCounts, 0, sizeof(m_nodeTypeCounts));
}
////////////////////////////////////////////////////////////////
/*!
   Return a vector of the non zero node fragment counters and their
   counts
*/
vector<AssemblerUtilities::typeCountPair>
InputStage::nodeFragmentCount() const
{

  return AssemblerUtilities::makeTypeCountVector(m_fragmentCounts, 0x1000);
}
//////////////////////////////////////////////////////////////////
/*!
   Returns a vector of the non zero type counters and their types.
*/
vector<AssemblerUtilities::typeCountPair>
InputStage::perTypeFragmentCount() const
{
  return AssemblerUtilities::makeTypeCountVector(m_typeCounts, 0x1000);

}
/////////////////////////////////////////////////////////////////
/*!
    Return a vector of typecount pair vectors for
    each node that has nonzero types.
*/
vector<pair<uint16_t, vector<AssemblerUtilities::typeCountPair> > >
InputStage::nodePerTypeFragmentCount() const
{
  // This is just a matter of calling makeTypeCountVector for each
  // node index and selecting only the nonempty ones:

  vector<pair<uint16_t, vector<AssemblerUtilities::typeCountPair> > > result;

  for (int i= 0; i < 0x1000; i++) {
    vector<AssemblerUtilities::typeCountPair> stats = AssemblerUtilities::makeTypeCountVector(m_nodeTypeCounts[i],
						      0x1000);
    if (stats.size()) {
      result.push_back(pair<uint16_t, vector<AssemblerUtilities::typeCountPair> >(i, stats));
    }
  }
  return result;
}

//////////////////////////////////////////////////////////////
/*!
   Inject a  fragment into the system.  This is only 
   legal if the input stage is running.

*/
void
InputStage::injectFragment(EventFragment* fragment)
{

  if (!m_running) {
    throw AssemblerWrongStateException(AssemblerWrongStateException::inactive,
				       string("injectFragment"),
				       string("InputStage::injectFragment injecting"));
  }
  else {
    uint16_t type = fragment->type();
    uint16_t node = fragment->node();

    // ensure the event fragment queue exists:

    FragmentQueue* queue = m_queues[node];
    if (queue) {
      queue->insert(*fragment);
      updateCounters(node,type);
      declareNewFragments(node);
    }
    else {
      // The fragment queue does not exist.. That's an error.

      throw InvalidNodeException(node,
				 "InputStage::injectFragment - injecting");
    }
  }
}
/////////////////////////////////////////////////////////////
/*!
     Add a callback. Callbacks get invoked on specific
     events.  In particulare when the system starts or stops,
     or when event fragments are added from a node.
     \param proc   - Function pointer to the function to call.
                     The usual trick is for this to be a static
		     member function of some object.
     \param clientData - Stuff passed to the proc without
                     interpretation.  The usual trick is for this to be
		     a pointer to an object the proc will relay to.

*/
void
InputStage::addCallback(InputStage::FragmentCallback  proc,
			void*                             clientData)
{
  m_callbackHandlers.push_back(pair<FragmentCallback, void*>(proc, clientData));
}

///////////////////////////////////////////////////////////////////////
/*!
    Locate and remove a callback handler from the callback handler list.
    The handler must matchin proc and clientData.
*/
void
InputStage::removeCallback(InputStage::FragmentCallback proc,
			   void*                            clientData)
{
  std::list<pair<FragmentCallback, void*> >::iterator p = 
    find(m_callbackHandlers.begin(),
	 m_callbackHandlers.end(),
	 pair<FragmentCallback, void*>(proc, clientData));
  if (p != m_callbackHandlers.end()) {
    m_callbackHandlers.erase(p);
  }

}
///////////////////////////////////////////////////////////////
/*!
 *   Peek at the head of the event fragment queue
 * for fragments from a specific node:
 * \param node - The node to peek from
 * \return EventFragment* 
 * \retval  Pointer to the first fragment in the queue.
 * \retval  NULL - queue is empty
 * \note  This does not alter the contents of the queue itself
 */
EventFragment*
InputStage::peek(uint16_t node)
{
	EventFragment* pResult = static_cast<EventFragment*>(NULL);
	if (m_queues[node]) {
		pResult = m_queues[node]->peek();
	}
	else {
	  throw InvalidNodeException(node,
				 "InputStage::peek");
	}
	return pResult;
}
//////////////////////////////////////////////////////////////
/*!
 * Remove the least recently inserted entry in a node fragment queue, 
 * returning a pointer to it
 * \param node  - The number of the node to remove.
 * \return EventFragment*
 * \retval Pointer to the remove fragment.
 * \retval NULL - queue was empty.
 */
EventFragment*
InputStage::pop(uint16_t node)
{
	EventFragment* pResult = static_cast<EventFragment*>(NULL);
	if (m_queues[node]) {
		pResult = m_queues[node]->remove();
	}
	else {
	  throw InvalidNodeException(node,
				 "InputStage::pop");
	}
	return pResult;
}
////////////////////////////////////////////////////////////
/*!
 * Removes all fragments from a specific event fragment queue.
 * \param node - Number of the node whose queue will be emptied.
 * \return std::list<EventFragment*>  
 * \retval  List of event fragments removed in the order they
 *          were removed.  This list will be empty if the queue was
 *          empty or is not participating in the assembly.
 */
list<EventFragment*>
InputStage::clear(uint16_t node)
{
	list<EventFragment*> result;
	if (m_queues[node]) {
		EventFragment* frag;
		while ((frag = m_queues[node]->remove())) {
			result.push_back(frag);
		}
	}
	else {
	  throw InvalidNodeException(node,
				 "InputStage::clear");
	}
	return result;
}
///////////////////////////////////////////////////////////////
/*!
 * Static member function that is called when data is available
 * from one of the input nodes.  We must read a buffer of data
 * from the channel, establish object context and dispatch to the
 * appropriate handler object function.
 * \param clientData    - This is actually a pointer to a
 *                        InputReadyData struct that will be used
 *                        to establish object context, read the data and
 *                        figure out how to dispatch it.
 * \param eventMask     - Mask of events that have occured on the channel
 *                        ..ignored as only reads can fire an event.
 */
void
InputStage::onBuffer(ClientData clientData, int eventMask)
{
	InputReadyData* readyData((reinterpret_cast<pInputReadyData>(clientData)));
	
	// Read the data and extract appropriate information to continue:
	
	uint16_t* pBuffer = new uint16_t[BUFFERSIZE];
	uint8_t* p        = reinterpret_cast<uint8_t*>(pBuffer);
	uint8_t** pNew(0);	// Will hold the new data.
	int bytesToRead   = BUFFERSIZE*(sizeof(uint16_t));
	int bytesRead;
	while (bytesToRead && 
	       ((bytesRead = readyData->s_pChannel->Read((void**)pNew, bytesToRead)) > 0)) {
	  uint8_t* pData = *pNew;
	  memcpy(p, pData, bytesRead);

	  delete []pData;
	  pNew = 0;
	  bytesToRead -= bytesRead;
	  p+= bytesRead;
	}
	if (bytesToRead) {
		//
		// error case: Residual bytes left:
		
		readyData->s_pObject->processInputFailure(pBuffer,
							  BUFFERSIZE*sizeof(uint16_t)-bytesToRead,
							  readyData->s_node);
	}
	else {
		// Full data buffer read.
	  readyData->s_pObject->processBuffer(pBuffer);



	}
}
/////////////////////////////////////////////////////////////////////////////////
/*!
  Our input buffer factory can
  generate the appropriate type of input buffer which,
  in turn, will generate an appropriate typeof iterator that
  will return the event fragments we need to insert in the
  event fragment queue.
  This code implicitly assumes that all fragments in a buffer
  are of the same type and from the same node ...when it comes
  to maintaining the statistics.
  */
void
InputStage::processBuffer(void* pBuffer)
{
  InputBuffer*         pInputBuffer = InputBufferFactory::create(pBuffer);
  InputBufferIterator* pIterator    = pInputBuffer->begin();
  uint16_t             node         = pInputBuffer->getNode();
  uint16_t             type         = pInputBuffer->getType();
  
  if (m_queues[node]) {
    while (!pIterator->End()) {
      EventFragment* pFragment = *(*pIterator);
      m_queues[node]->insert(*pFragment);
      updateCounters(node, type);
      pIterator->Next();
    }
  } 
  else {
    throw InvalidNodeException(node,
			       "processing buffer in input stage");
  }
  declareNewFragments(node);
}
/////////////////////////////////////////////////////////////////
/*!
 *  Process an input failure by invoking the error callbacks
 * with an error event.  The node is the node that suffered the 
 * failure.  At this point, input errors also shutdown the
 * associated node.
 * Note that if the node is the trigger node, that's really 
 * really bad.
 */
void
InputStage::processInputFailure(uint16_t* pPartialBuffer, 
								int       bytesRead,
								uint16_t  node)
{
	declareError(node);
	stopNode(node);
}

////////////////////////////////////////////////////////////////
/*
 *  Convert an event enumerated constant into the 
 *   corresponding string.  At present this is a switch
 *   default returns an unknown kind of string ... be sure to
 *   extend this when the event type list is extended.
 */
string
InputStage::eventToString(InputStage::event evt)
{
  switch (evt) {
  case NewFragments:
    return string("new");
  case ShuttingDown:
    return string("shutdown");
  case Starting:
    return string("startup");
  case Error:
    return string("error");
  default:
    return string("unknown");
  }
}

//////////////////////////////////////////////////////////////
/*
 * Start a node taking data.  This means getting a 
 * CTCLChannel for the node with SpecTclDAQ running
 * on the other end of it (pipe channel).  The
 * Channel is entered in the channel list and a fragment
 * queue is created for it.
 * \param nodeName    - Name of the node data will be from.
 * \param nodeId      - Id of the node.
 */
void
InputStage::startNode(const char* nodeName, uint16_t nodeId)
{
	string program = spectclDaqPath();
	string url     = spectclDaqURL(nodeName);
	
	const char*    argv[3];
	argv[0]      = program.c_str();
	argv[1]      = url.c_str();
	argv[2]      = "-everything";


	CTCLChannel* pChannel = new CTCLChannel(m_pConfig->getInterpreter(),
						3,
						argv,
						0);
	pChannel->SetEncoding("binary");
	m_queues[nodeId] = new FragmentQueue;
	
	// Create the callback for readability.
	
	Tcl_Channel chan = pChannel->getChannel();
	pInputReadyData  pData = new InputReadyData;
	pData->s_pObject = this;
	pData->s_pChannel= pChannel;
	pData->s_node    = nodeId;
	m_channels.push_back(pData);
	
	Tcl_CreateChannelHandler(chan, TCL_READABLE, InputStage::onBuffer, 
				 static_cast<ClientData>(pData));
	
}
///////////////////////////////////////////////////////////////
/*!
 * Stop accpeting input on a specific node. This is done in 
 * two cases:
 * - When the input stage is stopping.
 * - When an input stage data source has made an error.
 *   (in that case an error is presumably reported as well).
 * 
 * Action is as follows:
 * - Locate the node in the channel list.
 * - Drain and delete its fragment queue.
 * - Cancel its channel handler 
 * - Destroy the CTCLChannel object.
 * - Remove the item from the list.
 * 
 * (Actions are not necessarily in that order)
 * 
 * \param node - The node to stop.
 * 
 */

void 
InputStage::stopNode(uint16_t node)
{
	std::list<pInputReadyData>::iterator pChannel =
		find_if(m_channels.begin(),
				m_channels.end(),
				bind2nd(ptr_fun(matchChannelNode), node));
	
	// It's a no-op to stop a channel that is not running
	//
	if (pChannel != m_channels.end()) {
		pInputReadyData pData = *pChannel;
		emptyQueue(node);
		delete m_queues[node];
		m_queues[node] = static_cast<FragmentQueue*>(0);
		
		CTCLChannel* pTclChannel = pData->s_pChannel;
		m_channels.erase(pChannel);
		Tcl_Channel chan = pTclChannel->getChannel();
		Tcl_DeleteChannelHandler(chan, InputStage::onBuffer, pData);
		delete pData;
		delete pTclChannel;
		
	}
}
//////////////////////////////////////////////////////////////
/*!
 *  Emtpy the contents of an event fragment queue.
 * \param node   - Node to empty.
 * 
 * \note it is a no-op to empty a queue that does not exist.
 */
void 
InputStage::emptyQueue(uint16_t node)
{
	FragmentQueue* pQueue = m_queues[node];
	if (pQueue) {
		EventFragment* pFrag;
		while ((pFrag = pQueue->remove())) {
			delete pFrag;
		}
	}
}
////////////////////////////////////////////////////////////
/*!
 *   Declare an arbitrary event. All the callback handlers
 * are invoked in order of registration.
 * \param reason   - The reason for the event.
 * \param node     - node for which the event is declared.
 * 
 */
void 
InputStage::declareEvent(InputStage::event reason, uint16_t node)
{
	std::list<std::pair<FragmentCallback, void*> >::iterator p
		= m_callbackHandlers.begin();
	while(p != m_callbackHandlers.end()) {
		void*            pData    = p->second;
		FragmentCallback callback = p->first;
		(*callback)(pData, reason, node);
		p++;
	}
}
/////////////////////////////////////////////////////////////
/*!
 *  Declare a starting event.
 */
void 
InputStage::declareStarting()
{
	declareEvent(Starting, 0);
}
//////////////////////////////////////////////////////////////
/*!
 * Declare a stopping event.
 */
void
InputStage::declareStopping()
{
	declareEvent(ShuttingDown, 0);
}
/////////////////////////////////////////////////////////////
/*!
 *   Declare a node has an error:
 */
void
InputStage::declareError(uint16_t node)
{
	declareEvent(Error, node);
}
//////////////////////////////////////////////////////////////
/*!
 * Declare a node has new fragments.
 */
void
InputStage::declareNewFragments(uint16_t node)
{
	declareEvent(NewFragments, node);
}

//////////////////////////////////////////////////////////
/*
 * Update appropriate statistics counters for a fragment
 * production
 */
void
InputStage::updateCounters(uint16_t node, uint16_t type)
{
	m_fragmentCounts[node]++;
	m_typeCounts[type]++;
	m_nodeTypeCounts[node][type]++;
}
/////////////////////////////////////////////////////////
/*
 * Compute the path to the spectcldaq program.
 * This is a path join of the PREFIX externally defined
 * preprocessor variable, bin and spectcldaq.
 * We're just going to use PATHSEP as the path
 * separator and just merge the string.
 */
string
InputStage::spectclDaqPath()
{
	string topdir(PREFIX);
	string bindir("bin");
	string program("spectcldaq");
	
	return topdir + PATHSEP + bindir + PATHSEP + program;
}
/////////////////////////////////////////////////////////////
/*
 * Construct the URL for spetcldaq to use given a node.
 * This for now is tcp://nodename:sdlite-link  where sdlite-link
 * is translated from getservbyname...or defaults to 2700 if
 * that's not possible.
 */
string
InputStage::spectclDaqURL(const char* node)
{
	string urlFront("tcp://");
	string url;
	
	url = urlFront + node + ':';
	
	// Now we just need the port number:
	
	int port;
	struct servent* pService = getservbyname("sdlite-link", NULL);
	if (!pService) {
		port = 2700;
	} 
	else {
		port = ntohs(pService->s_port);
		
	}
	// Encode the port as a string and append it to the url:
	
	char portString[100];       // Can't overflow this.
	sprintf(portString, "%u", port);
	
	url += portString;
	url += "/";
	return url;
}