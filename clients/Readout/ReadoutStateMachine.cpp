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




//////////////////////////.cpp file///////////////////////////////////////////
//
// Header Files:
//


#include <config.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include "ReadoutStateMachine.h"                               

#include "Active.h"		// Include state definitions.
#include "Inactive.h"
#include "Paused.h"
#include "Exiting.h"
#include "camac.h"
#include "cmdio.h"
#include "buffer.h"


#include <Iostream.h>
#include <daqinterface.h>
#include <buftypes.h>
#include <time.h>
#include <assert.h>
#include <Fstream.h>
#include <time.h>
#include <skeleton.h>
#include <spectrodaq.h>
#include <stdio.h>
#include <string>
#include <Exception.h>

#ifdef HAVE_COPYRIGHT_NOTICE
#include <CopyrightNotice.h>
#endif
static const char* Copyright= 
"ReadoutStateMachine.cpp: Copyright 1999 NSCL, All rights reserved\n";

#include "Readout.h"
extern DAQBuff mydaq;
extern void*     nCamacFd;

int UsingVME=0;			// Provide this for people with old readout.

///////////////////////////////////////////////////////////////////////////
//  Local static utility functions... get the words from a long word 
//    WORD1 - Gets the low order word from a longword.
//    WORD2 - Gets the high order word from a longword.
//
inline UINT16 
WORD1(UINT32 l)
{
  return (UINT16)(l & 0xffff);
}
inline UINT16 
WORD2(UINT32 l)
{
  return (UINT16)((l >> 16) & 0xffff);
}

inline void 
CopyIn(DAQWordBufferPtr& p, void* src, unsigned nWords)
{
  UINT16* pSrc = (UINT16*)src;
  for(int i = 0; i < nWords; i++) {
    *p = *pSrc++;
    ++p;			// For objects preinc is faster (no copies).
  }
}

// Functions for class ReadoutStateMachine

/////////////////////////////////////////////////////////////////////////////
//
// Function:
//   ReadoutStateMachine()
// Operation Type:
//   Default Constructor
//
ReadoutStateMachine::ReadoutStateMachine() :
    m_PriorState(0), 
    m_nSegmentStart(0),
    m_nPriorDuration(0),
    m_nElapsedTime(0),  
    m_nLastScalerRead(0),
    m_nLastSnapShot(0),
    m_nSequenceNumber(0),
    m_nPartialSums(0),
    m_pCommandChannel(0),  
    m_pInactive(0),
    m_pActive(0),
    m_pPaused(0),
    m_pExiting(0)
{
  // This try/catch block deals with an issue in g++-3.x
  // with exceptions not always propagating correctly out of
  // a shared lib that we see in ReadoutMain.cpp
  //
  try {
#ifdef HAVE_COPYRIGHT_NOTICE
    // Copyright notice here so that it can't be removed from readoutmain.
    
    CopyrightNotice::Notice(cerr, "Readout", "2.0", "2002");
    CopyrightNotice::AuthorCredit(cerr, "Readout", "Ron Fox", 
				  "Jason Venema", "Chris Maurice"
				  (char*)NULL);
#endif
    // Set up the state machine, read in the transition table:
    
    m_pInactive = new Inactive;
    m_pActive   = new Active;
    m_pPaused   = new Paused;
    m_pExiting  = new Exiting;
    
    AddState(m_pInactive, string("INACTIVE"));
    AddState(m_pActive,   string("ACTIVE"));
    AddState(m_pPaused,   string("PAUSED"));
    AddState(m_pExiting,  string("EXITING"));
    
    char filename[100];
    sprintf(filename, "%s/etc/runstate.tran", INSTDIR);
    
    ifstream fTransitionTable(filename);
    assert(!(!fTransitionTable));
    assert(ReadTransitionTable(fTransitionTable));
  }
  catch (const char* msg) {
    cerr << "Caught const char* exception: " << msg << endl;
    throw;
  }
  catch (char* msg) {
    cerr << "Caught char* exception: " << msg << endl;
    throw;
  }
  catch (string& msg) {
    cerr << "Caught string exception: " << msg << endl;
    throw;
  }
  catch (CException &e) {
    cerr << "Caught an NSCL Exception: " << e.ReasonText() << endl;
    cerr << "While: " << e.WasDoing() << endl;
    throw;
  }
  catch (...) {
    cerr << "ReadoutStateMachine constructor is rethrowing some exception\n";
    throw;
  }
} 


//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    bool_t PollForCommand (  )
//  Operation Type:
//     Command Interface
//
bool 
ReadoutStateMachine::PollForCommand() 
{
// Checks for a pending  command on
// the command path:
//
// Returns:
//    true - A command is pending.
//    false- There are no pending commands.
//

  assert(m_pCommandChannel);	// Ensure that the command channel is valid.
  return (bool)daq_CheckForCommand(m_pCommandChannel);


}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    unsigned GetCommand (  )
//  Operation Type:
//     Command Interface.
//
unsigned 
ReadoutStateMachine::GetCommand() 
{
// Returns the transition event implied by the next
// command on m_pCommandChannel, the command
// channel for the process. Note that if there is no pending
// command, the thread will block until one is available.
// Use PollForCommand() to test for a pending command.
// 
// Returns:
//     unsigned   - Identifier of the event which corresponds
//                        to the command received on the channel.
//                        This identifier is the same as the event ids
//                        in the state machine transition table.
//       

  assert(m_pCommandChannel);
  RunControlCmd cmd = daq_GetCommand(m_pCommandChannel);
  return CommandToEvent(cmd);

}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    unsigned GetPriorState (  )
//  Operation Type:
//     selector.
//
unsigned 
ReadoutStateMachine::GetPriorState() 
{
// Returns the identifier of the previous state.
// This is just the value of m_PriorState
//
//
// Exceptions:  

  return m_PriorState;
}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void SetPriorState (  )
//  Operation Type:
//     Mutator
//
void 
ReadoutStateMachine::SetPriorState() 
{
// Sets m_PriorState to the current state.
// This is usually called from a state's Leave() member.
//
// Exceptions:  

  m_PriorState = GetCurrentState();
}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void EmitPause (  )
//  Operation Type:
//     EventBuffer interface.
//
void 
ReadoutStateMachine::EmitPause() 
{
// Emits a  Pause run event.
// This event also allows Routing to take place on
// any data formatted into the routing buffer so far.

  EmitControlBuffer(PAUSEBF);
}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void EmitResume (  )
//  Operation Type:
//     Buffer Interface
//
void 
ReadoutStateMachine::EmitResume() 
{
// Emits a resume event.  This event is immediately
// routed.
// Exceptions:  

  // Must maintain run elapsed times, but snapshots etc. get cleared.

  UpdateRunTime();
  unsigned nElapsed = m_nElapsedTime;

  m_nElapsedTime = m_nLastScalerRead = m_nLastSnapShot = nElapsed;
  EmitControlBuffer(RESUMEBF);

}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void EmitStart (  )
//  Operation Type:
//     Buffer Interface.
//
void 
ReadoutStateMachine::EmitStart() 
{
// Emits a  Start event.  This event is immediately
// routed.
// Exceptions:  


  ClearRunTime();
  EmitControlBuffer(BEGRUNBF);
  
}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void EmitStop (  )
//  Operation Type:
//     Buffer Interface.
//
void 
ReadoutStateMachine::EmitStop() 
{
// Emits a Stop run event.  This event, and
// any event data in the buffer are immediately
// routed.
// Exceptions:  

  EmitControlBuffer(ENDRUNBF);
}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void EmitScaler (  )
//  Operation Type:
//     Buffer Interface
//
void 
ReadoutStateMachine::EmitScaler() 
{
// A scaler event is emitted and routed along
// with any data which happens to be in the event
// buffer already.
//
  EmitScalerBuffer(SCALERBF, m_nLastScalerRead);
  m_nLastScalerRead = m_nLastSnapShot = m_nElapsedTime;

}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void EmitSnapshot (  )
//  Operation Type:
//     Buffer Interface.
//
void 
ReadoutStateMachine::EmitSnapshot() 
{
// A snapshot scaler event is emitted.
// the event and any other data in the buffer
// is immediately routed.
//
// Exceptions:  

  EmitScalerBuffer(SNAPSCBF, m_nLastSnapShot);
  m_nLastSnapShot = m_nElapsedTime;
}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   

//  Operation Type:
//     Selector.
//
/*!
    Get a buffer from the data acquisition system routing pool.
    The buffer gotten will be 2x the system buffering size.  This is done
    to support tight packing of events in buffers.  Events are packed until
    the used space is > buffersize at which time the event that overflows
    the buffer is copied into a fresh buffer, the buffer is contracted to
    the acquisition systemn buffering size, and routed.

    \return DAQWordBuffer* - pointer to the received buffer.
    */

DAQWordBuffer*
ReadoutStateMachine::GetBuffer(unsigned nWords) 
{

  DAQWordBuffer* pBuffer;
  
  mydaq.SetProcessTitle("Readout - Get");
  pBuffer =  new DAQWordBuffer(nWords);
  pBuffer->SetTag(DAQ_EVENTS);	// Tag by default as event data.
  mydaq.SetProcessTitle("Readout");
  return pBuffer;


}
DAQWordBuffer*
ReadoutStateMachine::GetBuffer()
{
  return GetBuffer(daq_GetBufferSize() * 2);
}
/*!
  Routes a buffer.  Peforms the following:
  - Contracts the buffer to the size of the system buffer.
  - Submits the buffer to the routing engine.
  - Deletes the buffer object.

 */
void
ReadoutStateMachine::RouteBuffer(DAQWordBuffer* pBuffer)
{

  pBuffer->Resize(daq_GetBufferSize(), true); // Resize back to correct size.
  pBuffer->Route();		// Submit to the routing engine.
  delete pBuffer;

}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    unsigned GetRunTime (  )
//  Operation Type:
//     selector.
//
unsigned 
ReadoutStateMachine::GetRunTime() 
{
// Returns the number of seconds since the <
// start of the run.
//
// Exceptions:  

  return m_nElapsedTime;
}
/*!
   Computes the current elapsed run time and stores it in m_nElapsedTime
   This is done by figuring out how long this segment has been running and
   adding it to the elapsed time of all prior segments.  

   A segment is defined as a period of active running in the run (e.g. a 
   begin to pause time period).
   
   Key member variables are:
   - m_nElapsedTime  [out]  Total length of run.
   - m_nPriorDuration [in]  Sum of the length of all active periods of this
                            run.
   - m_nSegmentStart  [in]  When the current segment started.

   Assumptions:
      time_t is equivalent to unsigned. 

*/
void
ReadoutStateMachine::UpdateRunTime() 
{
  time_t tNow;
  time(&tNow);			// This is the current time.
  unsigned nSegmentLength = tNow - m_nSegmentStart;
  m_nElapsedTime          = m_nPriorDuration + nSegmentLength;
  
}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void ClearRunTime (  )
//  Operation Type:
//     Mutator
//
void 
ReadoutStateMachine::ClearRunTime() 
{
// Reset the elapsed time in the run to 0.
//
// Exceptions:  
  time_t Now;
  time(&Now);

  m_nSegmentStart   = Now;
  m_nPriorDuration  = 0;
  m_nElapsedTime    = 0;
  m_nLastScalerRead = 0;
  m_nLastSnapShot   = 0;
  if(!m_nPartialSums.empty()) {
    for(size_t i = 0; i < m_nPartialSums.size(); i++) {
      m_nPartialSums[i] = 0;
    }
  }
}
/*!
  Does the time book keeping necessary to begin a new run segment.
  This is called on a resume or a begin.  (Begin just does a ClearRunTime()
  first).
  
  Actions:
  - m_nSegmentStart gets set to now.
  - m_nPriorDuration gets set to m_nElapsedTime

  Assumptions:
    m_nElapsedTime was brought up to date when the run was paused or is 
                   0 because a ClearRunTime was just done.
 */
void
ReadoutStateMachine::NewRunSegment()
{
  time_t Now;
  time(&Now);

  m_nPriorDuration = m_nElapsedTime;
  m_nSegmentStart  = Now;
  
}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void DeclareTick ( unsigned  nTicks=1 )
//  Operation Type:
//     Mutator.
//
/*!
   Indicates the passage of active run time to the
   state machine.
    
   Formal Parameters:
   \param nTicks = 1:
            Number of ticks to signal. In fact this parameter is now 
	    obsolete.
*/
void 
ReadoutStateMachine::DeclareTick(unsigned  nTicks) 
{


  UpdateRunTime();  		// This is more accurate than incrementally
                                // accumulating time and errors.
}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void OnInitialize (  )
//  Operation Type:
//     Hook override.
//
void 
ReadoutStateMachine::OnInitialize() 
{
// Initializes the run time envrionment.  In
// this case, this involves making connections
//  to the command channel and the buffer manager.
//

  m_pCommandChannel = daq_OpenControlPath(); // Open the control path.

  
  // Now initialize the base class state machine, that should
  // initialize the states.

  StateMachine::OnInitialize();
  ClearRunTime();

  prompt(stdout, "RunCtl> ");
}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void OnIllegalTransition ( unsigned nCurrentState, unsigned nEvent )
//  Operation Type:
//     
//
void 
ReadoutStateMachine::OnIllegalTransition(unsigned nCurrentState, unsigned nEvent) 
{
// Called when an illegal transition is
// attempted.  We give a bit of information
// about what was attempted, as well as
// identifying ourselves.
//
// Formal Parameters:
//      unsigned nCurrentState:
//          State of the system at the point the
//           illegal transition was attempted.
//      unsigned nEvent:
//           Event which would cause the
//           illegal transition.
//
// Exceptions:  

  cerr << "ReadoutStateMachine: ";
  StateMachine::OnIllegalTransition(nCurrentState, nEvent);

}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void OnCleanup ( unsigned  nState )
//  Operation Type:
//     Callback
//
void 
ReadoutStateMachine::OnCleanup(unsigned  nState) 
{
// Should be called by states if they are about to
//  exit the program.  This function performs any
//  cleanup required by the ReadoutStateMachine
//  prior to program exit.
//
// Formal Parameters:
//     unsigned nState:
//           Id of state attempting to exit.

  if(m_pCommandChannel)daq_CloseControlPath(m_pCommandChannel);
  delete m_pInactive;
  delete m_pActive;
  delete m_pPaused;
  delete m_pExiting;



  StateMachine::OnCleanup(nState);

}
//////////////////////////////////////////////////////////////////////
// Function:
//    bool_t IsControlBufferType(INT16 nType)
//    bool_t IsScalerBufferType(INT16  nType)
// Operation type:
//    Internal buffer type validators,
//    available to derived classes.
//
bool
ReadoutStateMachine::IsControlBufferType(INT16 nType)
{
  // Returns true if nType is a valid control buffer data type:

  switch(nType) {
  case BEGRUNBF:
  case ENDRUNBF:
  case PAUSEBF:
  case RESUMEBF:
    return true;
  default:
    return false;
  }
}
bool
ReadoutStateMachine::IsScalerBufferType(INT16 nType)
{
  // Returns true if nType is a valid scaler buffer type:

  switch(nType) {
  case SCALERBF:
  case SNAPSCBF:
    return true;
  default:
    return false;
  }
}
//////////////////////////////////////////////////////////////////////
//  Function:
//     void GetTime(bfTime* t)
//  Operation Type:
//     Internal, available for derived classes.
//
void
ReadoutStateMachine::GetTime(bftime* t)
{
  time_t nNow;
  tm*   sNow;

  time(&nNow);			// seconds since epoch.
  sNow = localtime(&nNow);	// Broken apart in to the useful pieces.

  // Note the Unix month goes 0-11, not 1-12.  We shift to get compatibility
  // with NSCL buffers.
  // In addition, UNIX years are 1900 based, so that must be added in.

  t->month   = sNow->tm_mon+1;	// Now just copy the fields we need into
  t->day     = sNow->tm_mday;   // the buffer time structure.
  t->year    = sNow->tm_year+1900;
  t->hours   = sNow->tm_hour;
  t->min     = sNow->tm_min;
  t->sec     = sNow->tm_sec;
  t->tenths  = 0;		// Unix doesn't provide ticks here.

}
//////////////////////////////////////////////////////////////////////
// Function:
//     void EmitControlBuffer(INT16 nBufferType)
// Operation Type:
//     Internal, available for derived classes
//
void
ReadoutStateMachine::EmitControlBuffer(INT16 nBufferType)
{
 
  // This internal function emits a control buffer given the actual
  // buffer type.
  //

  // Preconditions:
  // nBufferType is in the set of legal control buffer types.
  
  UpdateRunTime();

  assert(IsControlBufferType(nBufferType));

  DAQWordBuffer*      Buf = GetBuffer(daq_GetBufferSize());
  DAQWordBufferPtr   pBuf = GetBody(Buf);
  DAQWordBufferPtr pStart = pBuf; // Hold pointer to word count.
  Buf->SetTag(DAQ_STATE);

  //
  //  Now fill in the rest of the pause run:
  //
  ctlbody Body;
  memset(&Body, 0, sizeof(Body)); // Zero out the title.
  strncpy(Body.title, daq_GetTitle(), sizeof(Body.title)-1);
  Body.sortim = m_nElapsedTime;
  GetTime(&(Body.tod));
  CopyIn(pBuf, &Body, sizeof(Body)/sizeof(UINT16));

  FormatHeader(Buf, pBuf.GetIndex() - pStart.GetIndex(),
	       nBufferType, 0);
  mydaq.SetProcessTitle("Readout Route");
  Buf->Route();
  mydaq.SetProcessTitle("Readout");
  delete Buf;
  
}
//////////////////////////////////////////////////////////////////////
// Function:
//    void EmitScalerBuffer(INT16 nBufferType)
// Operation:
//    Internal, available for derived classes
//
void
ReadoutStateMachine::EmitScalerBuffer(INT16 nBufferType, unsigned nLastTime)
{
  //  Reads out the scalers and emits a scaler buffer containing that
  //  data.
  //    Parameters:
  //          INT16 nBufferType:
  //                Type of scaler buffer to readout.
  //          unsigned nLastTime:
  //                Time of the last readout.
  //
  UpdateRunTime();
  int nScalers = (int)daq_GetScalerCount();

  if(m_nLastSnapShot == 0) {	        // First readout...
    if(nScalers > m_nPartialSums.size())
      m_nPartialSums.reserve(nScalers);	// Ensure vector is big enough.
    for(size_t i = 0; i < nScalers; i++) {
      m_nPartialSums[i]  = 0;	        // Zero partial sums.
    }
  }
  // Determine the last readout time... it depends on the buffer type:

  UINT32* pThisReadout = new UINT32[nScalers]; // Read to here.
  assert(pThisReadout);		               // >>>BUGBUGBUG<<< 
  UINT16 nwds = readscl(pThisReadout, nScalers); // Read the scalers.
  clrscl();

  // If snapshot scaler, then we sum pThisReadout into the sums, if 
  // not then add the sums into the pThisReadout array.  Regardless,
  // the pThisReadout array is what's stuck in the buffer:
  //
  if(nBufferType == SCALERBF) {
    for(size_t i = 0; i < nScalers; i++) {
      pThisReadout[i] += m_nPartialSums[i];
      m_nPartialSums[i]   = 0;
    }
  } 
  else {
    for(size_t i = 0; i < nScalers; i++) {
      m_nPartialSums[i] += pThisReadout[i];
    }
  }
  // Now build the buffer from m_nPartialSums and all the other decorations
  
  DAQWordBuffer*       Buf = GetBuffer(daq_GetBufferSize());
  DAQWordBufferPtr   pBuf =  GetBody(Buf);
  DAQWordBufferPtr pStart = pBuf;
  Buf->SetTag(DAQ_STATE);	// This buffer is tagged as a control buffer.

  *pBuf++  = WORD1(m_nElapsedTime);
  *pBuf++  = WORD2(m_nElapsedTime);
  pBuf    += 3;			// 3 unused words.

  *pBuf++  = WORD1(nLastTime);
  *pBuf++  = WORD2(nLastTime);	// Insert the times (longword data).
  pBuf    += 3;			// 3 more unused words.

  // Next is the scaler data.

  CopyIn(pBuf, pThisReadout, nScalers*(sizeof(UINT32)/sizeof(UINT16)));

  FormatHeader(Buf, pBuf.GetIndex() - pStart.GetIndex(), 
	       nBufferType, nScalers);

  mydaq.SetProcessTitle("Readout Routing");
  Buf->Route();
  mydaq.SetProcessTitle("Readout");

  // Done with the buffers.

  delete Buf;
  delete []pThisReadout;

}
//////////////////////////////////////////////////////////////////////
// Function:
//    unsigned CommandToEvent(RunControlCommand& command)
// Operation:
//    interface translation
//
unsigned
ReadoutStateMachine::CommandToEvent(RunControlCmd& command)
{
  // Translates the command enumerator to a valid event id for our
  // state machine.  If there is no match, returns the NOOP event.
  //
  // Formal Parameters:
  //     RunControlCommand& command:
  //         The command received from the run control stream.
  //
  
  // First map the command enumerator to the Event name:

  string sEventName;
  switch(command) {
  case rctl_Begin:
    sEventName = "BEGIN";
    break;
  case rctl_End:
    sEventName = "END";
    break;
  case rctl_Pause:
    sEventName = "PAUSE";
    break;
  case rctl_Resume:
    sEventName = "RESUME";
    break;
  case rctl_Exit:
    sEventName = "EXIT";
    break;
  default:
    cerr << "Invalid command code received by ReadoutStateMachine" << endl;
  case rctl_Scaler:
  case rctl_Snapshot:
  case rctl_NoOp:
    sEventName = "NOOP";
  }
  // Now get the ID. if there's no match, get the ID of NOOP:

  int nEventId = NameToEventId(sEventName);
  if(nEventId < 0) {
    cerr << "No matching event for command code received by "
         << "ReadoutStateMachine" << endl;

    nEventId = NameToEventId("NOOP");
  }

  return (unsigned)nEventId;
}
////////////////////////////////////////////////////////////////////////////
//
// Function:
//     DAQWordBufferPtr GetBody(DAQWordBuffer* pBuffer)
// Operation type:
//     Utility.
//
DAQWordBufferPtr
ReadoutStateMachine::GetBody(DAQWordBuffer* pBuffer)
{
  // Returns a DAQWordBufferPtr which refers to the body part of 
  // a buffer.  

  DAQWordBufferPtr p;
  p  = pBuffer->operator&();
  p += sizeof(bheader)/sizeof(INT16);
  return p;
}
////////////////////////////////////////////////////////////////////////////
//
// Function:
//     void FormatHeader(DAQWordBuffer* pBuffer, 
//		         UINT16 nWords, UINT16 nType, UINT16 nEntities);
// Operation type:
//   Utility.
//
void
ReadoutStateMachine::FormatHeader(DAQWordBuffer* pBuffer, 
				  UINT16 nWords, 
				  UINT16 nType, 
				  UINT16 nEntities)
{
  // Formats the header of a buffer given the stuff which varies on a 
  // buffer to buffer basis.
  //
  // Formal Parameters:
  //    DaqWordBuffer* pBuffer:
  //      Pointer to the DAQ buffer.
  //    UINT16 nWords:
  //      Number of words stuffed in the buffer body.
  //    UINT16 nType:
  //      Type of the buffer.
  //    UINT16 nEntities:
  //      Number of 'things' in the buffer.
  //

  DAQWordBufferPtr p = pBuffer->operator&();
  *p++ = (INT16)(nWords + (sizeof(bheader)/sizeof(INT16))); // Size of buffer.
  *p++ = nType;			// Type of buffer.
  *p++ = 0;			// For now checksum is not computed.
  *p++ = daq_GetRunNumber();
  unsigned long seq = getSequenceNumber();
  CopyIn(p, &seq, 2);
  *p++ = nEntities;
  *p++ = 1;			// Lam count.
  *p++ = 1;			// CPU number.
  *p++ = 1;			// Number of bit registers.
  *p++ = BUFFER_REVISION;	// Buffer revision level.
  *p++ = 0x0102;		// Word Signature.
  INT32 lsig = 0x01020304;
  CopyIn(p, &lsig, sizeof(INT32)/sizeof(INT16));
}

