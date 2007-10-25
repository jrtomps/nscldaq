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
#include "AssemblerOutputStage.h"

#include <TCLInterpreter.h>
#include <CNSCLOutputBuffer.h>
#include <CNSCLPhysicsBuffer.h>
#include <CNSCLScalerBuffer.h>
#include <CNSCLControlBuffer.h>
#include <CNSCLStringListBuffer.h>
#include "AssembledEvent.h"
#include "AssembledPhysicsEvent.h"
#include "AssembledScalerEvent.h"
#include "AssembledStateTransitionEvent.h"
#include "AssembledStringArrayEvent.h"
#include "AssemblerErrors.h"
#include "AssemblerUtilities.h"

#include "BadEventTypeException.h"

#include <RangeError.h>
#include <buffer.h>
#include <buftypes.h>
#include <time.h>
#include <stdlib.h>

#include <dshapi/daqhwyapi.h>
#include <dshnet/daqhwynet.h>

using namespace daqhwyapi;
using namespace daqhwynet;

#include <spdaq/spdaqlite.h>

using namespace spdaq;
using namespace std;

/// Manifest constants:


///////////////////// Static data initializations: ////////////////////////

AssemblerOutputStage::DispatchTableEntry 
                    AssemblerOutputStage::m_commandDispatchTable[SUBCOMMANDCOUNT] =
  {
    {"clear",      2, &AssemblerOutputStage::clearStatistics},
    {"statistics", 2, &AssemblerOutputStage::reportStatistics},
    {"event",      3, &AssemblerOutputStage::physicsEvent},
    {"control",    5, &AssemblerOutputStage::controlEvent}
    
  };

AssemblerOutputStage::EventProcessor AssemblerOutputStage::m_eventProcessors[VALIDBUFFERTYPES] =
  {
    &AssemblerOutputStage::invalidEvent,             // 0
    &AssemblerOutputStage::appendPhysicsEvent,       // 1
    &AssemblerOutputStage::makeScalerBuffer,         // 2
    &AssemblerOutputStage::makeScalerBuffer,         // 3
    &AssemblerOutputStage::makeDocumentationBuffer,  // 4
    &AssemblerOutputStage::makeDocumentationBuffer,  // 5
    &AssemblerOutputStage::makeDocumentationBuffer,  // 6
    &AssemblerOutputStage::invalidEvent,             // 7
    &AssemblerOutputStage::invalidEvent,             // 8
    &AssemblerOutputStage::invalidEvent,             // 9
    &AssemblerOutputStage::invalidEvent,             // 10
    &AssemblerOutputStage::makeStateChangeBuffer,    // 11
    &AssemblerOutputStage::makeStateChangeBuffer,    // 12
    &AssemblerOutputStage::makeStateChangeBuffer,    // 13
    &AssemblerOutputStage::makeStateChangeBuffer,    // 14
    &AssemblerOutputStage::invalidEvent,             // 15
  };

static const char* Months[] =
  {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
    "Aug", "Sep", "Oct", "Nov", "Dec", NULL
  };

static const char* WeekDays[] =
  {
    "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun", NULL
  };

/// Deal with issues in strptime with timezones:
//

static int lookup(const char* item, const char** table)
{
  int index = 0;
  while (*table) {
    if(strcmp(item, *table) == 0) return index;
    table++;
    index++;
  }
  return -1;
}

int parseTime(const char* timeString,
	      struct tm*  timeStruct)
{
  char wkday[100];
  char month[100];
  char tz[100];
  int day;
  int hour, min, sec;
  int year;

  // times could be 24 hr clock or could be 12 hr with AM/PM >sigh<
  // First try 24 hr.

  int status = sscanf(timeString,
		     "%s %s %d %d:%d:%d %s %d",
		     wkday, month, &day, &hour, &min, &sec, tz, &year);
  if (status != 8) {
    // try 12 hr with AM/PM:

    char ampm[5];
    status = sscanf(timeString,
		    "%s %s %d %d:%d:%d %s %s %d",
		    wkday, month, &day, &hour, &min, &sec, ampm, tz, &year);
    if (status == 9) {
      if (strcmp(ampm, "PM") == 0) {
	// Convert hours to 24 hr format:

	hour +=  12;
      }
    } 
    else {
      return 0;
    }
  }

  // Do the easy stuff first:

  timeStruct->tm_sec = sec;
  timeStruct->tm_min = min;
  timeStruct->tm_hour= hour;

  timeStruct->tm_mday = day -1;
  timeStruct->tm_year = year-1900;

  // That leaves:
  //    tm_mon  0 Month number: lookup in table.
  //    tm_wday - Day of the week - table look up for wkday.
  //    tm_yday - Day of the year - we don't care so set it zero
  //    tm_isdst- Daylight savings flag - we don't care so set it false

  timeStruct->tm_yday = 0;
  timeStruct->tm_isdst   = 0;

  timeStruct->tm_mon  = lookup(month, Months);
  timeStruct->tm_wday = lookup(wkday, WeekDays);

  if ((timeStruct->tm_mon == -1)   ||
      (timeStruct->tm_wday ==-1))
    return 0;
  
  return 1;
}

/////////////////// Explicit canonicals /////////////////////////////

/*!
    The command is constructed with the explicit
    name "outputstage", and immediately registered on the
    interpreter.  The statistics are all cleared as is the pointer to the
    physics buffer.

    \param interp   - The interpreter on which to register this command.
*/
AssemblerOutputStage::AssemblerOutputStage(CTCLInterpreter& interp) :
  CTCLObjectProcessor(interp, string("outputstage"), true),
  m_pPhysicsBuffer(0),
  m_runNumber(0)
{
  clearCounters();		// Shared code with clearStatistics this way
}

/*!
   If the physics buffer member points to a buffer, we must
   submit and destroy it.
*/
AssemblerOutputStage::~AssemblerOutputStage()
{
  if (m_pPhysicsBuffer) {
    commitPhysicsBuffer();
  }
}
///////////////// Public dispatchers ////////////////////////////////////

/*!
    operator() is called whenever our command is hit.
    We will:
    - Ensure there is a subcommand keyword.
    - If so attempt to match it against the keywords in 
      m_commandDispatchTable.
    - In the event of a match with the correct number of paramters,
      dispatch.
    - In the event of a match with the wrong number of parameters report
      that error.
    - In the event of no match, report that error as well.

    \param interp  - Reference to the Tcl interpreter object that is 
                     running this command.
    \param objv    - Reference to the arra of Tcl objects that make up the
                     command words.
    \return int
    \retval TCL_OK - The command was successfully dispatched and
                     ran to completion successfully.
    \retval TCL_ERROR - Either the command could not be dispatched or
                     it was dispatched but failed to succeed.
*/
int
AssemblerOutputStage::operator()(CTCLInterpreter&    interp,
				 vector<CTCLObject>& objv)
{
  if (objv.size() < 2) {
    return
      AssemblerErrors::setErrorMsg(interp,
				       AssemblerErrors::TooFewParameters,
				       Usage());
  }
  //
  // Attempt to match/dispatch the command:

  string subcommand = objv[1];
  for (int i=0;  i < SUBCOMMANDCOUNT; i++) {
    if (subcommand == m_commandDispatchTable[i].pKeyword) {
      if (objv.size() == m_commandDispatchTable[i].parameterCount) {
	return (this->*m_commandDispatchTable[i].processor)(interp, objv);
      }
      else {
	// Wrong parameter count.. which report the direction of error:

	if (objv.size() < m_commandDispatchTable[i].parameterCount) {
	  return 
	    AssemblerErrors::setErrorMsg(interp,
					     AssemblerErrors::TooFewParameters,
					     Usage());
	}
	else {
	  return
	    AssemblerErrors::setErrorMsg(interp,
					     AssemblerErrors::TooManyParameters,
					     Usage());
	}

      }
    }
  }
  // Subcommand did not match any keywords in the table:

  return 
    AssemblerErrors::setErrorMsg(interp,
				     AssemblerErrors::InvalidSubcommand,
				     Usage());

}

/*!
  Submit an event to the output stage.  What we do depends
  on the type of the event; and for the most part that's just
  dispatching control to the appropriate function in
  m_eventProcessors.
  \param  event   - Reference to the event to dispatch.

  \throws CRangeError -if the event has a type we can't dispatch.
 

*/
void
AssemblerOutputStage::submitEvent(AssembledEvent& event)
{
  unsigned short type = static_cast<unsigned short>(event.type());
  if (type < VALIDBUFFERTYPES) {
    (this->*AssemblerOutputStage::m_eventProcessors[type])(event);
  }
  else {
    throw CRangeError(0, VALIDBUFFERTYPES-1, type,
		      "AssemblerOutputStage::submitEvent - dispatching to event processor");
  }
}

//////////////////////////// Command processors /////////////////////////

/*
  Clear the statistics. The dispatcher has already verified there are no
  more parameters on the command line, so this will always be 
  successful...and the parameters will be ignored.
*/
int
AssemblerOutputStage::clearStatistics(CTCLInterpreter&    interp,
				      vector<CTCLObject>& objv)
{
  clearCounters();

  return TCL_OK;
}
/*
  Report the statistics to the user via the interpreter Result for this
  command.  This will also always succeed.
    The final result will be  a list of statistics:
    - element 0: Total number of events received.
    - element 1: Total number of output buffers submitted.
    - element 2: List of pairs describing the number of buffers of each type
                 submitted (zeroes suppressed).  Each pair is a buffer type/counter.
    - element 3: List of pair s describing the events received broken down by node.
                 each pair is a node/counter where nodes that have zeroes
                 are supressed.
*/
int
AssemblerOutputStage::reportStatistics(CTCLInterpreter&     interp,
				       vector<CTCLObject>&  objv)
{
  CTCLObject result;
  result.Bind(interp);

  // Do the easy stuff first:

  result += (int)m_eventsReceived;
  result += (int)m_buffersSubmitted;

  // Now list the buffers by type and append that:

  CTCLObject buffers;
  buffers.Bind(interp);
  for (int i=0; i < VALIDBUFFERTYPES; i++) {
    if (m_buffersByType[i] != 0) {
      CTCLObject oneBuffer;
      oneBuffer.Bind(interp);
      oneBuffer += i;
      oneBuffer += (int)m_buffersByType[i];

      buffers += oneBuffer;
    }
  }
  result += buffers;

  // Similarly for the node event statistics.

  CTCLObject events;
  events.Bind(interp);
  for (int i =0; i < 0x10000; i++) {
    if (m_eventsByNode[i] != 0) {
      CTCLObject node;
      node.Bind(interp);

      node += i;
      node += (int)m_eventsByNode[i];

      events += node;
    }
  }
  result += events;

  // Finally set the result and return ok:

  interp.setResult(result);
  return TCL_OK;

}
/*
   Make a simulated physics event, and submit it for assembly.
   this is part of the testing subsystem of the output stage.
   It allows us to pretty well test the subsystem using tcltest.
   the event body is random and the size of the event (including
   the word count is provided.
   Parameters:
      interp    - The Tcl interpreter that is executing this command.
      objv      - Array of command word objects (objv[2] is the word count).

   Returns: 
     TCL_OK     - If the event was created and submitted.
     TCL_ERROR  - If the event was created but not submitted.

   Errors:
      BadEventSize  - the event size must be a positive integer > 3.
      ExceptionEvent- Exception caught submitting the event. This happens
                      if the event is too big to fit in an empty buffer.
*/
int
AssemblerOutputStage::physicsEvent(CTCLInterpreter&     interp,
				   vector<CTCLObject>&  objv)
{
  objv[2].Bind(interp);
  

  // Get and validate the event size:


  int eventSize;
  try {
    eventSize = objv[2];
  }
  catch (...) {
    return
      AssemblerErrors::setErrorMsg(interp,
				   AssemblerErrors::BadEventSize,
				   Usage());
  }
  if (eventSize < 3) {		// Jumbo events remember...
    return
      AssemblerErrors::setErrorMsg(interp,
				   AssemblerErrors::BadEventSize,
				   Usage());
  }

  // Now construct the event:

  int nWords = eventSize - 1;	//  1 word of event size.

  AssembledPhysicsEvent event;
  unsigned short* pRawData = new unsigned short[nWords];

  for (int i=0; i < nWords; i++) {
    pRawData[i] = static_cast<unsigned short>(random());
  }
  event.addData(pRawData, nWords);

  delete []pRawData;		// Done with the raw data.


  // Try to submit the event:

  try {
    submitEvent(event);
  }
  catch (...) {
    return
      AssemblerErrors::setErrorMsg(interp,
				  AssemblerErrors::ExceptionEvent,
				  Usage());
  }
  // Success if control gets here:


  return TCL_OK;


}
/*
   Creates a control event.  Control events come in three types:
   - scaler events,
   - state transition events
   - Documentation events.  
   We should have two parameters after the subcommand keyword.
   The first is the type of the event.
   The second is a list of event type specific data.
   This function just dispatches based on the overall type of event.

   Parameters:
      interp    - The interpreter that is executing this command.
      objv      - The list of Tcl objects that makes up our command words.
   Returns:
      TCL_OK   - Successful dispatch and completion.
      TCL_ERROR- Something went wrong.
*/
int
AssemblerOutputStage::controlEvent(CTCLInterpreter&      interp,
				   vector<CTCLObject>&   objv)
{
  // Extract the type of event we need to submit
  //
  
  objv[2].Bind(interp);
  unsigned eventType;

  try {
    eventType = (int)objv[2];
  }
  catch (...) {
    return
      AssemblerErrors::setErrorMsg(interp,
				   AssemblerErrors::InvalidType,
				   Usage());
  }
  // Extract the source node.


  objv[3].Bind(interp);
  unsigned  node;

  try {
    node = (int)objv[3];
    if (node > 0xffff) {
      return
	AssemblerErrors::setErrorMsg(interp,
				     AssemblerErrors::BadId,
				     Usage());
    }
  }
  catch (...) {
    return 
      AssemblerErrors::setErrorMsg(interp,
				   AssemblerErrors::BadId,
				   Usage());
  }



  // Now try to dispatch:

  objv[4].Bind(interp);

  if (isStateTransitionEvent(eventType)) {
    return submitFakeStateTransition(interp, eventType, node, objv[4]);
  }
  else if (isScalerEvent(eventType)) {
    return submitFakeScalerEvent(interp, eventType, node, objv[4]);
  }
  else if (isDocumentationEvent(eventType)) {
    return submitFakeDocumentationEvent(interp, eventType, node, objv[4]);
  }
  else {
    return
      AssemblerErrors::setErrorMsg(interp, 
				  AssemblerErrors::InvalidType,
				  Usage());
  }


  interp.setResult("DEFECT -- hit end of AssemblerOutputStage::controlEvent!!!");
  return TCL_ERROR;		// Should not land here
}

/* 
   Report the usage of this command;
*/
string
AssemblerOutputStage::Usage()
{
  string result;
  result  += "Usage:\n";
  result  += "   outputstage clear\n";
  result  += "   outputstage statistics\n";
  result  += "   outputstage event size\n";
  result  += "   outputstage control type node body\n";
  return result;
}

/*
   Process a physics event:
   If the data won't fit in a single buffer throw a range error.
   If the physics output buffer does not yet exist,
   create it.
   If the data won't fit in the current output buffer, flush it and
   create a new one.
   Put the event in the buffer,
   Count the event as submitted as well as submitted from that node.

*/
void
AssemblerOutputStage::appendPhysicsEvent(AssembledEvent& event)
{
  // This must be a physics event:

  AssembledPhysicsEvent* pEvent = dynamic_cast<AssembledPhysicsEvent*>(&event);
  if (!pEvent) {

    throw BadEventTypeException(event.type(), "Physics event",
       "AssemblerOutputStage::appendPhysicsEvent casting generic event");
  }

  int bodysize = BUFFERSIZE - sizeof(struct bheader)/sizeof(unsigned short);

  int eventSize = pEvent->size();
  eventSize    += sizeof(long)/sizeof(unsigned short); // Size longword is 32 bits.

  if (eventSize > bodysize) {
    throw 
      CRangeError(sizeof(long)/sizeof(unsigned short),
		  0, BUFFERSIZE,
		  "Fitting events in a buffer in AssemblerOutputStage::appendPhysicsEvent");

  }
  // Do we need to commit the buffer?

  if (m_pPhysicsBuffer &&
      ((eventSize + m_pPhysicsBuffer->WordsInBody())  > bodysize) ) {
    commitPhysicsBuffer();	// Clear m_pPhysicBuffer.
  }

  // Do we need to make a new event buffer:

  if(!m_pPhysicsBuffer) {
    newPhysicsBuffer();
  }
  // Now we can add the event:

  DAQWordBufferPtr pDest = m_pPhysicsBuffer->StartEvent();

  vector<unsigned short>::iterator pSrc = pEvent->begin();
  vector<unsigned short>::iterator pEnd = pEvent->end();

  while(pSrc != pEnd) {
    *pDest = *pSrc;
    ++pSrc;
    ++pDest;
  }

  m_pPhysicsBuffer->EndEvent(pDest);

  // Statistics:

  m_eventsReceived++;
  m_eventsByNode[0]++;		// Physics events are always from node0.
  
}
/*
   Create a scaler buffer. Scaler buffers will flush out
   event buffers for now.
*/
void
AssemblerOutputStage::makeScalerBuffer(AssembledEvent& event)
{
  // Cast to the right event type and throw up if not:

  AssembledScalerEvent *pEvent = dynamic_cast<AssembledScalerEvent*>(&event);
  if (!pEvent) {
    throw 
      BadEventTypeException(event.type(),
			    "ScalerEvent",
	        "AssemblerOutputStage::makeScalerBuffer casting generic event");
  }

  commitPhysicsBuffer();	// Start by flushing the physics buffer...

  // Create and fill the buffer:

  CNSCLScalerBuffer buffer(BUFFERSIZE);
  initializeHeader(buffer);

  buffer.SetStartTime(pEvent->getStartTime());
  buffer.SetEndTime(pEvent->getEndTime());
  buffer.PutScalerVector(pEvent->getScalers());
  buffer.SetCpuNum(pEvent->node());
  buffer.SetType(pEvent->type());
  commitBuffer(buffer);		// Handles output buffer statistics...
  
  // Statistics:

  m_eventsReceived++;
  m_eventsByNode[pEvent->node()]++;
  
}
/*
  Create a state change buffer
  state change buffers will flush out the physics event buffer in progress ...
  if there iswone.
*/
void
AssemblerOutputStage::makeStateChangeBuffer(AssembledEvent& event)
{
  AssembledStateTransitionEvent* pEvent = dynamic_cast<AssembledStateTransitionEvent*>(&event);
  if (!pEvent) {

    throw
      BadEventTypeException(event.type(),
			    "State Transition",
          "AssemblerOutputStage::makeStateChangeBuffer - casting generic event");
  }
  // Get the run number from that buffer...
  // and flush any  physics buffer.

  m_runNumber = pEvent->getRunNumber();

  commitPhysicsBuffer();

  // If a begin buffer, clear the sequence:

  if(pEvent->type() == BEGRUNBF) {
    CNSCLOutputBuffer::ClearSequence();
  }

  

  // Create and fill the output buffer.

  CNSCLControlBuffer buffer(BUFFERSIZE);

  initializeHeader(buffer);


  buffer.SetCpuNum(pEvent->node()); // These get passed through.
  buffer.PutTimeOffset(pEvent->getElapsedTime());
  buffer.PutTimestamp(pEvent->getTimestamp());
  buffer.PutTitle(pEvent->getTitle());							
  buffer.SetType(pEvent->type());
  commitBuffer(buffer);

  // Statistics:

  m_eventsReceived++;
  m_eventsByNode[pEvent->node()]++;

}
/*
   Create a documentation buffer. Documentation buffers are any
   buffer that consists of a list of strings. Documentation buffers do not
   need to flush any partial physics buffer.
*/
void
AssemblerOutputStage::makeDocumentationBuffer(AssembledEvent& event)
{
  AssembledStringArrayEvent* pEvent = 
    dynamic_cast<AssembledStringArrayEvent*>(&event);

  if (!pEvent) {
    throw
      BadEventTypeException(event.type(),
			    "Documentation Event",
         "AssemblerOutputStage::makeDocumentationBuffer - casting generic event");
  }


  // Create and fill the output buffer:

  
  CNSCLStringListBuffer* pBuffer = new CNSCLStringListBuffer(BUFFERSIZE);
  initializeHeader(*pBuffer);

  pBuffer->SetCpuNum(pEvent->node()); // these keep a node number.
  pBuffer->SetType(pEvent->type());

  vector<string>::iterator i = pEvent->begin();
  vector<string>::iterator e = pEvent->end();

  while(i != e) {
 
    if (!pBuffer->PutEntityString(*i)) {
      // Overflow...
      
      commitBuffer(*pBuffer);
      delete pBuffer;
      pBuffer = new CNSCLStringListBuffer(BUFFERSIZE);
      initializeHeader(*pBuffer);
      pBuffer->SetCpuNum(pEvent->node());
      pBuffer->SetCpuNum(pEvent->type());
      pBuffer->PutEntityString(*i);
      
    }
    i++;
  }
  commitBuffer(*pBuffer);
  delete pBuffer;

  // Statistics:

  m_eventsReceived++;
  m_eventsByNode[pEvent->node()]++;
  

}
/*
   Dispatched to if an invalid event type was encountered.

*/
void
AssemblerOutputStage::invalidEvent(AssembledEvent& event)
{
  throw CRangeError(0, VALIDBUFFERTYPES-1,
		    event.type(),
		    "AssemblerOutputStage - received an event type that cannot be handled");

}
/*

   Commits a buffer to the spectrodaq lite software.
   This means routing the underlying buffer.
   Accumulating buffer statistics.

*/
void
AssemblerOutputStage::commitBuffer(CNSCLOutputBuffer& buffer)
{
  
  int type = buffer.getBufferType();

  buffer.Route();
  m_buffersSubmitted++;
  m_buffersByType[type]++;


}

/*
  Commits the current physics buffer (if it exists).
  Note that this implies shrinking it to the correct size
  first.
*/
void
AssemblerOutputStage::commitPhysicsBuffer()
{
  if (m_pPhysicsBuffer) {
    commitBuffer(*m_pPhysicsBuffer);
    delete m_pPhysicsBuffer;
    m_pPhysicsBuffer = 0;
  }
}
/*
   Clear all the statistical counters:
*/

void
AssemblerOutputStage::clearCounters()
{
  m_eventsReceived   = 0;
  m_buffersSubmitted = 0;
  memset(m_buffersByType, 0, sizeof(m_buffersByType));
  memset(m_eventsByNode, 0, sizeof(m_eventsByNode));

}
/*
  Return true if the event type passed in is a state transition type:
*/
bool
AssemblerOutputStage::isStateTransitionEvent(int type)
{
  return ((type == BEGRUNBF)      ||
	  (type == ENDRUNBF)      ||
	  (type == PAUSEBF)       ||
	  (type == RESUMEBF));
}
/*
   Return true if the event is a scaler event:

*/
bool
AssemblerOutputStage::isScalerEvent(int type)
{
  return ((type == SCALERBF)  || (type == SNAPSCBF));
}
/*
   Return true if the event is a documentation event.
*/
bool
AssemblerOutputStage::isDocumentationEvent(int type)
{
  return ((type == PKTDOCBF)            ||
	  (type == RUNVARBF)            ||
	  (type == STATEVARBF));
}

/*
   Submit a fake state transition buffer.


   Parameters:
       interp     - The Tcl interpreter running the command that called us.
       type       - The type of state change event to create, this has been
                    vetted as valid prior to calling us.
       node       - The originating node for the event.
       body       - the event body  (see below)

   Returns:
      TCL_OK      - If the event was correctly specified and submitted.
      TCL_ERROR   - If not in which case an error message is left in the
                    command result.

   The body object must be a list that consist of:
   - the run number for the event.
   - The elapsed time in seconds from the beginning of the run (integer).
   - A timestamp in the form of [clock format [clock seconds]], that is:
     e.g.: Wed Jun 20 14:41:47 EDT 2007
     Used to populate the buffer timestamp time.
   - The title string to be associated with the event.

*/
int
AssemblerOutputStage::submitFakeStateTransition(CTCLInterpreter& interp,
					       int              type,
					       int              node,
					       CTCLObject&      body)
{


  body.Bind(interp);

  // The first thing we check is whether or body is a valid tcl list
  // we do this by attempting to extract all list elements:
  
  vector<CTCLObject>  bodyList;
  try {
    bodyList = body.getListElements();
    if (bodyList.size() != 4) {
      return  AssemblerErrors::setErrorMsg(interp, 
					   AssemblerErrors::InvalidEventBody,
					   Usage());
    }
  }
  catch (...) {
    return
      AssemblerErrors::setErrorMsg(interp, 
				   AssemblerErrors::InvalidEventBody,
				   Usage());
  }


  for (int i=0; i < bodyList.size(); i++) {
    bodyList[i].Bind(interp);
  }

  // Set strings for the easy things:


  string timestampString = bodyList[2];
  string title           = bodyList[3];


  // Now get the run number... must be a positive integer:

  int run;
  try {

    run = bodyList[0];
    if (run < 0) {
      return
	AssemblerErrors::setErrorMsg(interp,
				     AssemblerErrors::InvalidEventBody,
				     Usage());
    }
  }
  catch (...) {
    return
      AssemblerErrors::setErrorMsg(interp,
				   AssemblerErrors::InvalidEventBody,
				   Usage());
  }

  // See if we can get the elapsed time.. that's an unsigned..

  unsigned elapsedTime;
  try {
    elapsedTime = (int)bodyList[1];
  }
  catch (...) {
    return
      AssemblerErrors::setErrorMsg(interp,
				   AssemblerErrors::InvalidEventBody,
				   Usage());
  }
  // Finally attempt to decode the timestamp into a tm struct.

  struct tm timestamp;

  if (!parseTime(timestampString.c_str(), &timestamp)) {
    return
      AssemblerErrors::setErrorMsg(interp,
				   AssemblerErrors::InvalidEventBody,
				   Usage());
    
  }

  // Now stuff should succeed:

  AssembledStateTransitionEvent event(run, 
				      (enum AssembledEvent::BufferType)type);

  event.setTitle(title);
  event.setElapsedTime(elapsedTime);
  event.setTimestamp(timestamp);
  try {
    submitEvent(event);
  }
  catch(...) {
    return 
      AssemblerErrors::setErrorMsg(interp,
				   AssemblerErrors::InvalidEventBody,
				   Usage());
    
    
  }
  // It all worked!

  return TCL_OK;
}

/*
   Submit a fake scaler event to the output stage for testing..
   Parameters:
     interp    - The interpreter that is running the controllingcommand.
     type      - The even type which as already been vetted as a scaler event.
     node      - The CPU that contributed this.
     body      - Tcl object that is the body of the event.

  Returns:
     TCL_OK    - The command worked and an event has been submitted.
     TCL_ERROR - The command failed and the reason is in the command.

     The format of the body will be a list containing:
     - the interval start time.
     - the interval stop time.
     - the scalers.. rest of the list, may be zero..for an empty scaler buffer.
*/
int
AssemblerOutputStage::submitFakeScalerEvent(CTCLInterpreter&   interp,
					    int                type,
					    int                node,
					    CTCLObject&        body)
{
  // First attempt to get the elements of the event body list.

  body.Bind(interp);
  vector<CTCLObject>  bodyList;
  try {
    bodyList = body.getListElements();
    if (bodyList.size() < 2) {
      return 
	AssemblerErrors::setErrorMsg(interp,
				     AssemblerErrors::InvalidEventBody,
				     Usage());
    }
  }
  catch (...) {
    return AssemblerErrors::setErrorMsg(interp,
					AssemblerErrors::InvalidEventBody,
					Usage());
  }
  for (int i=0; i < bodyList.size(); i++) {
    bodyList[i].Bind(interp);
  }

  // The interval start and stop times are both unsigned integers (32 bits)
  /// As are any scalers...

  unsigned long startTime;
  unsigned long stopTime;
  vector<uint32_t> scalers;

  try {
    startTime = (int)bodyList[0];
    stopTime  = (int)bodyList[1];
    for (int i = 2; i < bodyList.size(); i++) {
      bodyList[i].Bind(interp);
      scalers.push_back((uint32_t)(int)bodyList[i]);
    }
  }
  catch  (...) {
    return AssemblerErrors::setErrorMsg(interp,
					AssemblerErrors::InvalidEventBody,
					Usage());    
  }
  // now we have all we need to build and dispatch the event:

  AssembledScalerEvent event(node, startTime, stopTime, 
			     (AssembledEvent::BufferType)type);
  event.addScalers(scalers);
  try {
    submitEvent(event);
  }
  catch (...) {
    return AssemblerErrors::setErrorMsg(interp,
					AssemblerErrors::InvalidEventBody,
					Usage());    

  }

  return TCL_OK;

}
/*
  Build a documentation buffer from a documentation event.
  Parameters:
    interp           - TCL Interpreter that's running the command.
    type             - Type of the event.  This has already been vetted to be
                       a documentation buffer event.
    node             - Originating node.  This gets passed into the buffer.
    body             - CTCLObject that describes the body.  See below for more
                       information.

 Returns:
    TCL_OK       - the buffer was created and routed without fail.
    TCL_ERROR    - The buffer could not be created/routed.  In that case,
                   the result object for the interpreter is set with some
		   textual information about why the operation failed.
 Body Format:
    The body object is a list of zero or more strings.  The strings are placed
    unmodified as the strings in the string event buffer.
    In this code, it is an error for there to be too many strings to fit in the
    documentation buffer.
*/
int
AssemblerOutputStage::submitFakeDocumentationEvent(CTCLInterpreter& interp,
						   int              type,
						   int              node,
						   CTCLObject&      body)
{
  body.Bind(interp);

  // Try to parse the body as a list:

  vector<CTCLObject> bodyList;
  try {
    bodyList = body.getListElements();
  }
  catch (...) {
      return 
	AssemblerErrors::setErrorMsg(interp,
				     AssemblerErrors::InvalidEventBody,
				     Usage());
  }

  // Build the event from the strings:

  AssembledStringArrayEvent event(node, 
				  (AssembledEvent::BufferType)type);
  for (int i=0; i < bodyList.size(); i++) {
    bodyList[i].Bind(interp);
    event.addString((string)bodyList[i]);
  }
  // Submit the event and let the output stage do it's duty:

  try {
    submitEvent(event);
  }
  catch (...) {
    return AssemblerErrors::setErrorMsg(interp,
					AssemblerErrors::InvalidEventBody,
					Usage());    
  }
  return TCL_OK;
}
/*
   Create a new physics event.  The physics event is made to be
   2*BUFFERSIZE (do we really need to do that.. maybe not, since
   we can check if an event fits before we put it in.).

*/
void
AssemblerOutputStage::newPhysicsBuffer()
{
  m_pPhysicsBuffer = new CNSCLPhysicsBuffer(BUFFERSIZE);
}
/*
  Initialize the header of a buffer.. to what it can be initialized to
  at buffer creation time:
*/
void
AssemblerOutputStage::initializeHeader(CNSCLOutputBuffer& buffer)
{
  buffer.SetNbitRegisters(0);	// obsolete field.
  buffer.SetLamRegisters(0);	// obsolete field.
  buffer.SetRun(m_runNumber);	// From most recent state transition.
}
