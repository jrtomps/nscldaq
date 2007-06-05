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

   
//////////////////////////CExperiment.cpp file////////////////////////////////////

#include <config.h>
#include <netdb.h>
#include <netinet/in.h>
#include "CVMEInterface.h"
#ifndef HIGH_PERFORMANCE
#include "Exception.h"
#endif /* ! HIGH_PERFORMANCE */
#include "CExperiment.h"                  
#ifdef HIGH_PERFORMANCE
#include <Exception.h>
#endif /* HIGH_PERFORMANCE */
#include "CStateTransitionCommand.h"
#include "CBeginCommand.h"
#include "CEndCommand.h"
#include "CReadoutMain.h"
#include "CDuplicateSingleton.h"
#include "CNoSuchObjectException.h"
#include "CApplicationSerializer.h"
#include "CNSCLPhysicsBuffer.h"
#include "CNSCLScalerBuffer.h"
#include "CNSCLControlBuffer.h"
#include "CStateTransitionCommand.h"
#include "CRunVariableBuffer.h"
#include "CStateVariableBuffer.h"
#include "CNSCLDocumentationBuffer.h"
#include "CDocumentedPacketManager.h"
#include "CDocumentedPacket.h"
#include "CInterpreterCore.h"
#include "CInterpreterShell.h"


#include "CTimer.h"
#include "CScalerTrigger.h"

#include "CRunState.h"

#include "CRunVariableCommand.h"
#include "CRunVariable.h"

#include "CStateVariableCommand.h"
#include "CStateVariable.h"

#include "buftypes.h"
#include "buffer.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;		// need this here for spectrodaq.
#endif

#include <DesignByContract.h>
#include <spectrodaq.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <typeinfo>
#include <Iostream.h>
#include <algorithm>
#include <tcl.h>



using namespace DesignByContract;

extern CReadoutMain MyApp;

//! Manifest constants.

static const unsigned int nTriggerDwellTime = 10; //!< ms trigger holds mutexs
static const unsigned int nTriggersPerPoll  =  10; //!< triggers between time
                                                   // checks.
static const unsigned int msPerClockTick = 100;   //!< Time between run time
				                   // ticks.
static const unsigned int SECOND     = 1000000;    //!< usec /seconds.
static const unsigned int MILISECOND = 1000; //!< usec/msec.


/*!
   >>>LOCAL CLASS TO CEXPERIMENT<<< Encapsulates the thread which
   checks for triggers.  This is a polling thread.  Assumption is that
   the caller knows what it's doing and does not double start a thread!!.

   */
class CTriggerThread : public DAQThread
{
  CExperiment* m_pExperiment;	//!< Points to our experiment object.
  CTrigger*    m_pTrigger;	//!< Points to the trigger check object
  bool         m_Exiting;	//!< True if asked to exit (stop called).
  DAQThreadId    m_Id;		//!< Our thread ID if running.
  unsigned int m_msHoldTime;	//!< ms to hold the global mutex.
  unsigned int m_nTriggerdwell; //!< triggers betweeen time polls.
public:
  CTriggerThread(CExperiment* pExp, CTrigger* pTrig);

  void Start();			// Start execution.
  void Stop();			// Stop execution.
protected:
  virtual int operator()(int argc, char** argv);
  virtual void MainLoop();

};
//! Construct a trigger thread object.
CTriggerThread::CTriggerThread(CExperiment* pExp, CTrigger* pTrig) :
  m_pExperiment(pExp),
  m_pTrigger(pTrig),
  m_Exiting(false),
  m_msHoldTime(nTriggerDwellTime),
  m_nTriggerdwell(nTriggersPerPoll)
{}
//! Start trigger thread.
void
CTriggerThread::Start()
{
  m_Exiting = false;
  daq_dispatcher.Dispatch(*this, 0, 0);
  m_Id = getId();
}
/*! Schedule stop of trigger thread.
    \note This function assumes the calling thread is not 
          the trigger thread and that the caller owns the 
	  global serialization mutex (both reasonable assumptions).
*/
void
CTriggerThread::Stop()
{
  if(m_Exiting) return;	// Already exiting or done.

  DAQThreadId id( m_Id);
  m_Exiting = true;

  // We have the mutex, we need to release it so the triger thread
  // can get it to run... we'll re-acquire after the join at the same level.
  //
  CApplicationSerializer& mutex(*(CApplicationSerializer::getInstance()));

  unsigned nLockLevel = mutex.getLockLevel();
  mutex.UnLockCompletely();

  Join();			// Wait for trigger to exit.

  for(unsigned i =0; i < nLockLevel; i++) {
    mutex.Lock();
  }
}
//! Entry point for thread.. just calls MainLoop(), and returns 0.
//
int
CTriggerThread::operator()(int argc, char** argv)
{
  try {
    MainLoop();
  } 
  catch(char* pReason) {
    cerr << "Main loop of trigger failed: " << pReason << endl;
  }
  catch(string& rReason) {
    cerr << "Main loop of trigger failed (string thrown): " << rReason << endl;
    CApplicationSerializer::getInstance()->UnLockCompletely();
  }
  catch (CException& rException) {
    cerr << "Main loop of trigger failed: " <<
      rException.ReasonText() << " while: " << rException.WasDoing() << endl;
  }
  catch(...) {
    cerr << "Main loop of trigger caught an exception " << endl;
    CApplicationSerializer::getInstance()->UnLockCompletely();
  }
  return 0;
}

/*! 
  Main loop for trigger thread:
  The balancing act that's done in this loop is to keep trigger response
  latency low while not starving other threads such as the clock or interactive
  threads of execution. 
  - Latency is minimized by holding the application 
    serialization mutex, but holding that forever starves other application 
    threads, so roughly every m_msHoldTime ms, the mutex is released and
    then re-attached (if there are people waiting for the mutex, they'll
    get to execute, and we'll go to the tail of the queue).
  - gettimeofday(2) requires execution time, and could increase trigger
    response latency. Therefore, the time we've held the application 
    serialization mutex is only measured if either:
    # We have checked for triggers  500 times
    # We have processed m_nTriggerdwell event triggers.
 
  The assumption here is that triggers take 100's of usec to process.
  At every mutex release, m_Exiting is checked, and if set, we return
  to our caller who is responsible for exiting our thread.
 */
void
CTriggerThread::MainLoop() 
{
  CScalerTrigger* pScaler = m_pExperiment->getScalerTrigger();
  while(!m_Exiting) {
    struct timeval mutexstart;
    struct timeval mutexend;
    struct timezone tz;		// Unused but required for gettimeofday(2).
    int dwell;
    //
    // Lock the mutex and process triggers for the dwell time.
    // The trigger is checked several times to amortize gettimeofday().
    //
    sched_yield();		// Let other threads run.
    CApplicationSerializer::getInstance()->Lock();
    gettimeofday(&mutexstart, &tz);
    do {
      int triggers=0;
      for(int i = 0; i < 500; i++) {
	if((*m_pTrigger)()) {	// Read an event...
	  m_pExperiment->ReadEvent();
	  if((triggers++) >= m_nTriggerdwell) break; // Check elapsed time.
	}
      }
      // Now try the scaler trigger:

      if((*pScaler)()) {
	m_pExperiment->TriggerScalerReadout();
      }

      // If we've held the mutex for longer than m_msHoldTime,
      // release the mutex so that other threads get a chance to run.
      //
      gettimeofday(&mutexend, &tz);
      int secdif = mutexend.tv_sec - mutexstart.tv_sec;
      mutexend.tv_usec += SECOND*secdif;
      dwell = (mutexend.tv_usec - mutexstart.tv_usec)/MILISECOND;
    } while(dwell < m_msHoldTime);
    CApplicationSerializer::getInstance()->UnLock();
  }
}



/*!
   Default constructor.  This is called when declarations of the form e.g.:
   -  CExperiment  object;
   are performed.
*/
CExperiment::CExperiment (unsigned nBufferSize) :
  m_EventBuffer(0),		// Spectrodaq might not be up yet.
  m_nBufferSize(nBufferSize),
  m_pStatusModule(0),
  m_pTrigger(0),
  m_pTThread(0),
  m_LastSnapTime(0),
  m_LastScalerTime(0),
  m_pScalerTrigger(0),
  m_nEventsAcquired(0),
  m_nWordsAcquired(0),
  m_pDataStore(0)
{
  setupDataStore();
}
/*!
  Parameterized constructor.  Allows the experiment to be created as needed for
  an initial minimal set.  The experiment can still be 'edited' later on by adding
  segments to the event and scalers to the initial module.
  \param pTriggerModule - Derived from CTrigger provides a trigger module to the
                          experiment.  Trigger modules are required to allow the
                          experiment readout to respond to physics events. Usually this
			  is a CVMETrigger or a CCAMACTrigger
  \param pEventReadout  - Provides a single event segment which is read out
                          in response to a physics trigger. Additional event segments
			  can be inserted into the experiment as it contains a 
			  CCompoundEventSegment (see CCompoundEventSegment::AddSegment()). 
			  These additional segments will be read
			  out in order of insertion.
  \param pScalers = 0  - Provides a scaler module which is added to the experiment's 
                         scaler bank.  Additional scaler modules can be added to the
			 experiment after construction (See CScalerBank::AddScalerModule()).
  \param pStatus = 0   - Provides a status module for the experiment.  The status module
                         provides end of event clears, busy clears and notification of
			 when the scaler readout is beginning and ending. Usually this is a
			 CVMETrigger or a CCAMACTrigger.
   \param nBufferSize = 4096 - Number of words in a buffer.

  \note The objects pointed to by these parameters must remain in scope for the duration
  of their use as they are not copied into the experiment object, but instead referred to via
  these pointers.  If objects go out of scope, the experiment will likely behave strangely or
  simply fail in unpredictable ways.
  */
CExperiment::CExperiment(CTrigger*      pTriggerModule,
			 CEventSegment* pEventReadout,
			 CScaler*       pScalers,
			 CStatusModule* pStatus,
			 unsigned       nBufferSize) :
  m_EventBuffer(0),		// Spectrodaq may  not be booted yet.
  m_nBufferSize(nBufferSize*2),
  m_pStatusModule(pStatus),
  m_pTrigger(pTriggerModule),
  m_pTThread(0),
  m_LastSnapTime(0),
  m_LastScalerTime(0),
  m_pScalerTrigger(0),
  m_nEventsAcquired(0),
  m_nWordsAcquired(0),
  m_pDataStore(0)
{
  setupDataStore();

  if(pScalers) m_Scalers.AddScalerModule(pScalers);
  m_EventReadout.AddSegment(pEventReadout);
}

/*! 
   Destructor.
*/
CExperiment::~CExperiment() {}

// Functions for class CExperiment

/*!
    Called to initiate data taking.  This is done
    either as a transition from inactive to active
    or as a transition from paused to active.
    The trigger is initialized and the trigger
    thread is started.  The timer is also 
    started.

	\param rCommand - The command which is responsible for
                  starting data taking. 
    \note 
    - The dynamic type of the rCommand object is used to determine
      if this state change is a resume or a start. 
    - The caller is assumed to have validated this state transition, so we don't
      attempt this ourselves.

*/
void 
CExperiment::Start(CStateTransitionCommand& rCommand)  
{


  assert(m_pScalerTrigger);	// Must have established a scaler trigger.

  // Wrap the entire function in a try catch block to nail the 
  // exceptions that may come up:
  //

  try {

    // Execute pre-actions:
    
    rCommand.ExecutePreFunction();
    
    // Figure out what kind of buffer to emit and emit it.
    
    try {
      CBeginCommand& rBegin(dynamic_cast<CBeginCommand&>(rCommand)); // Throws if resume.
      MyApp.getClock().Reset();	// Reset the run elapsed time.
      EmitStart();		// and emit a begin buffer.
      m_LastScalerTime = 0;	// Neither scalers have been readout yet this run.
      m_LastSnapTime   = 0;
      
      // Set the starttime variable:
      
      CInterpreterShell* pShell = CReadoutMain::getInstance()->getInterpreter();
      CInterpreterCore*  pCore  = pShell->getInterpreterCore();
      time_t epochTime = time(NULL);
      string sTime(ctime(&epochTime));
      pCore->setStartTime(sTime);
      
      // Reset the statistical counters:
      
      m_nEventsAcquired = 0;
      m_nWordsAcquired  = 0;
      
      pCore->setEvents(m_nEventsAcquired);
      pCore->setWords(m_nWordsAcquired);
      
    }
    catch (bad_cast& rbad) {
      m_LastSnapTime = 0;	// Snaps will not have been read out at resume.
      EmitResume();		// Emit a resume without zeroing the run elapsed time.
    }
    
    // Emit documentation and runstate variable buffers.
    
    TriggerDocBuffer();
    TriggerRunVariableBuffer();
    TriggerStateVariableBuffer();
    
    // Prepare the hardware for readout:
    
    try {
      m_EventReadout.Initialize();	// Initialize the event readout...
      m_Scalers.Initialize();
      m_Scalers.Clear();
      m_EventReadout.Clear();	// Clear digitizers prior to start.
    }
    catch (DesignByContractException& rContractViolation) {
      string msg(rContractViolation);
      
      cerr << "Interface contract violation: " << msg << endl;
      cerr << "Detected in user's code called by CExperiment::Start" << endl;
    }
    
    
    // Start the trigger process and clock.
    
    StartTrigger();
    MyApp.getClock().Start(msPerClockTick, nTriggerDwellTime/2);
    m_pScalerTrigger->Initialize();
    ClearBusy();
    
    
    
    //<----------- At this point we can potentially take data.
    // Execute post-actions:
    
    
    rCommand.ExecutePostFunction();
  } 
  catch (string reason) {
    cerr << "CExperiment::Start caught a string exception : " << reason << endl;
  }
  catch (char* reason) {
    cerr << "CExperiment::Start caught a char* exception : " << reason << endl;
  }
  catch (CException& reason) {
    cerr << "CExperiment::Start caught a CSNCLException: "
	 << reason.ReasonText() << " while " << reason.WasDoing() << endl;
  }
  catch (...) {
    cerr << "CExperiment::Start caught an unexpected type of exception\n";
  }
}  

/*!
    Schedules a transition from active to
    either inactive or paused.  The trigger
    module is informed that it should stop running
    without giving any more event triggers.
    The appropriate end actions are performed.
    

	\param  rCommand refers to the Tcl command
	        object which makes the run inactive.

*/
void 
CExperiment::Stop(CStateTransitionCommand& rCommand)  
{
  // Wrap the entire body of the Stop function in a try catch block
  // to attempt to describe any problems encountered.
  try {
    // Do the pre actions.
    
    rCommand.ExecutePreFunction();
    
    
    // Emit documentation and variable list buffers.
    
    TriggerDocBuffer();
    TriggerRunVariableBuffer();
    TriggerStateVariableBuffer();  
    m_pScalerTrigger->Cleanup();
    
    // Stop the trigger and clock processes.  Note that the stop trigger function
    // synchronizes with the exit of the trigger thread.
    
    StopTrigger();		//<--------- At this point we can't take data.
    TriggerScalerReadout();	// Closing scaler buffers.
    
    // Figure out what kind of event this is and emit the appropriate buffer
    //  (End or Pause).
    
    try {
      CEndCommand& rend(dynamic_cast<CEndCommand&>( rCommand));
      EmitEnd();
    }
    catch (bad_cast& rbad) {
      EmitPause();
    }
    
    // Do the post actions.
    
    MyApp.getClock().Stop();
    rCommand.ExecutePostFunction();
  } 
  catch (string reason) {
    cerr << "CExperiment::Stop caught a string exception: " << reason << endl;
  }
  catch (char* reason) {
    cerr << "CExperiment::Stop caught a char* exception: " << reason << endl;
  }
  catch (CException& reason) {
    cerr << "CExperiment::Stop caught a CException: "
	 << reason.ReasonText() << " while " << reason.WasDoing() << endl;
  }
  catch (...) {
    cerr << "CExperiment::Stop caught an unexpected exception type\n";
  }
}


  
/*!
    Reads an event in response to an event trigger.
    The event is read into the current position within the
    event buffer.  If necessary (the worst case entity won't fit in the
    buffer), the event buffer is routed.

*/
void 
CExperiment::ReadEvent()  
{

  // For sure spectrodaq is booted and the event buffer can be created
  // at this time.
  //
  if(!m_EventBuffer) {
    CNSCLOutputBuffer::IncrementSequence();
    m_EventBuffer = new CNSCLPhysicsBuffer(m_nBufferSize * 2);
  }
  
#ifndef HIGH_PERFORMANCE
  DAQWordBufferPtr ptr(m_EventBuffer->StartEvent());
  DAQWordBufferPtr hdr = ptr;
#else /* HIGH_PERFORMANCE */
  unsigned short* ptr(m_EventBuffer->StartEvent());
  unsigned short*  hdr = ptr;
#endif /* HIGH_PERFORMANCE */
   
  CVMEInterface::Lock();
  try {
    ptr = m_EventReadout.Read(ptr);
    m_EventReadout.Clear();
  }
  catch (DesignByContractException& rContractViolation) {
    string msg(rContractViolation);
    cerr << "Interface contract violation: " << msg << endl;
    cerr << "Detected in CExperiment::ReadEvent" << endl;
    cerr << "Any partially acquired event will be discarded\n";
    ptr = hdr;
  }
  catch (string reason) {
    cerr << "Event Segment reads threw a string exception: " << reason 
	 << " any partial event read will be discarded "     << endl;
    ptr = hdr;			// Ensure the event is retracted.
  }
  catch (char* reason) {
    cerr << "Event segment reads threw a char* exception: "
	 << reason << " any partial event read will be discarded\n";
    ptr = hdr;
  }
  catch (CException& rexcept) {
    cerr << "Event segment reads threw a CException : "
	 << rexcept.ReasonText() << " while " << rexcept.WasDoing() <<endl;
    cerr << "Any partial event data read will be discarded\n";
    ptr = hdr;
  }
  catch (...) {
    cerr << "Event segment reads threw a unexpected exception type\n";
    cerr << "Any partial event data read will be discarded\n";
    ptr = hdr;
  }


#ifdef HIGH_PERFORMANCE

#endif /* HIGH_PERFORMANCE */
  PostEvent();
  CVMEInterface::Unlock();

  m_nEventsAcquired++;
#ifndef HIGH_PERFORMANCE
  m_nWordsAcquired += ptr.GetIndex() - hdr.GetIndex();
  if(ptr.GetIndex() > m_nBufferSize) {
#else /* HIGH_PERFORMANCE */
  int nEventSize = ptr - hdr + 1;
  m_nWordsAcquired += nEventSize;
  if((m_EventBuffer->WordsInBody() + nEventSize) > (m_nBufferSize - sizeof(bheader)/sizeof(unsigned short))) {
#endif /* HIGH_PERFORMANCE */
     
     Overflow(hdr, ptr);
     
     // Update the Tcl statistic vars:
     
     CInterpreterShell* pShell = 
	        CReadoutMain::getInstance()->getInterpreter();
     CInterpreterCore*  pCore  = pShell->getInterpreterCore();
     pCore->setEvents(m_nEventsAcquired);
     pCore->setWords(m_nWordsAcquired);
 
     
  } else {
    if (ptr == hdr) {
#ifndef HIGH_PERFORMANCE
      m_EventBuffer->RetractEvent(ptr);	// No data read actually.
    } 
#else /* HIGH_PERFORMANCE */
      m_EventBuffer->RetractEvent(ptr);
    }
#endif /* HIGH_PERFORMANCE */
    else {
      m_EventBuffer->EndEvent(ptr);
    }
  }

}  

/*!
    Performs all operations which are required
    after the event has been read out.


*/
void 
CExperiment::PostEvent()  
{
  
  // 
  // Clear the busy 

  ClearBusy();

}  

/*!
    Adds an event segment to the readout chain.
    Events are read out in segments.  This
    member ads a segment to the chain of segments
    which are read out in response to an event trigger.

	\param pSegment - points to the event segment to add
	                  to the readout chain.

    \note  The event segment pointer is copied rather than the event
           segement, so it must remain in scope either for the life of the
	   experiment or until removed from the list.

*/
void 
CExperiment::AddEventSegment(CEventSegment* rSegment)  
{
  m_EventReadout.AddSegment(rSegment);
}  

/*!
    Removes an event segment from
    the set of event segments read out
    by this experiment. 

	\param  pSegment - Pointer to the segment to remove.
	\exception CNoSuchObjectException - throw if the segment does not exist.

*/
void 
CExperiment::RemoveEventSegment(CEventSegment* pSegment)  
{
  m_EventReadout.DeleteSegment(pSegment); // Throws the exception for us.
}  

/*!
    Sets the status module to indicate the
    computer is now busy.


*/
void 
CExperiment::SetBusy()  
{
  m_pStatusModule->GoBusy();
}  

/*!
    Facades for the status module.
    Sets the status module to indicate
    the computer is now not busy.


*/
void 
CExperiment::ClearBusy()  
{
  m_pStatusModule->ModuleClear();
  m_pStatusModule->GoClear();
}  

/*!
    Establishes the trigger module.
    This supplies a trigger object which
    will be run as a thread to check for
    events.


*/
void 
CExperiment::EstablishTrigger(CTrigger* pTrigger)  
{
  m_pTrigger = pTrigger;
}  



/*!
    Establishes a module to manage
    the busy.  The busy module has two
    NIM pulsed outputs.  One indicates
    the computer is going busy for software reasons.
    The other indicates that the computer is going
    not busy for any reason.

	\param CStatusModule* pStatus

*/
void 
CExperiment::EstablishBusy(CStatusModule* pStatus)  
{
  m_pStatusModule = pStatus;
}  

/*!
    Called to initiate a scaler readout.
    The current buffer is routed a new buffer
    allocated, filled with scaler data and routed,
    and another new buffer is created for subsequent
    event readout data.


*/
void 
CExperiment::TriggerScalerReadout()  
{

  // If there is an event data buffer with events in it
  // commit it:

  if(m_EventBuffer) {
    
    if(m_EventBuffer->getEntityCount()) {
      m_EventBuffer->SetRun(GetRunNumber());
      m_EventBuffer->SetCpuNum(getCpu());
      m_EventBuffer->Resize(m_nBufferSize);
      m_EventBuffer->Route();
    }
    delete m_EventBuffer;
    m_EventBuffer = 0;
  }

  //
  vector<unsigned long> scalers;
  try {
    scalers = m_Scalers.Read();
    m_Scalers.Clear();
  }
  catch (DesignByContractException& rViolation) {
    string msg(rViolation);
    cerr << "Interface contract violation: " << msg << endl;
    cerr << "Detected in user code called by CExperiment::TriggerScalerReadout\n";
    cerr << "Any partial scaler reads are discarded\n";
    scalers.erase(scalers.begin(), scalers.end());
  }
  catch (string msg) {
    cerr << "Scaler reads threw a string exception: " << msg << endl;
    cerr << "Any scalers read wil be discarded\n";
    scalers.erase(scalers.begin(), scalers.end());
  }
  catch (char* msg) {
    cerr << "Scaler reads threw a char* exception: " << msg << endl;
    cerr << "Any scalers read wil be discarded\n";
    scalers.erase(scalers.begin(), scalers.end());
  }
  catch (CException& exception) {
    cerr << "Scaler reads threw a CException: " << exception.ReasonText()
	 << " while " << exception.WasDoing() << endl;
    cerr << "Any scalers read will be discarded\n";
    scalers.erase(scalers.begin(), scalers.end());    
  }
  catch (...) {
    cerr << "Scaler reads threw an unexpected exception type\n";
    cerr << "Any scalers read will be discarded\n";
    scalers.erase(scalers.begin(), scalers.end());  
  }

  CNSCLScalerBuffer buffer(m_nBufferSize);

  // If a snapshot scaler has been readout, the values
  // just read must be added to their sums.
  //
  if((!m_IntervalSums.empty()) && (m_LastSnapTime != 0)) {
    int nelements = (scalers.size() <= m_IntervalSums.size()) ? scalers.size() : 
                                                                m_IntervalSums.size();
    for(int i =0; i < nelements; i++) {
      scalers[i] += m_IntervalSums[i];
      m_IntervalSums[i] = 0;
    }
  }
  // Format the buffer and adjust the times:
  //

  int now = GetElapsedTime()/10; // Seconds.
  buffer.PutScalerVector(scalers);
  buffer.SetStartTime(m_LastScalerTime);
  buffer.SetEndTime(now);
  buffer.SetCpuNum(getCpu());
  buffer.SetRun(GetRunNumber());
  buffer.Route();		// No sequence increment for scalers.

  m_LastSnapTime = 0;
  m_LastScalerTime = now;

 
}  

/*!
    Triggers the production of a run variable buffer.
    - A new buffer is allocated (existing buffer left alone).
    - Run data are filled into the buffer.
    - The buffer is routed.


*/
void 
CExperiment::TriggerRunVariableBuffer()  
{
  CRunVariableCommand* 
    Vars(MyApp.getInterpreter()->getInterpreterCore()->getRunVariables());
  
  RunVariableIterator i = Vars->begin();

  //
  // Multiple buffers may be required:

  while(i != Vars->end()) {
    CRunVariableBuffer buf(m_nBufferSize);
    i = EmitRunVariableBuffer(buf, i, Vars->end());
    buf.SetRun(GetRunNumber());
    buf.SetCpuNum(getCpu());
    buf.Route();		// Increment else SpecTcl's eff is wrong.
  }
  
}  
void
CExperiment::TriggerStateVariableBuffer()
{
  CStateVariableCommand* 
    Vars(MyApp.getInterpreter()->getInterpreterCore()->getStateVariables());
  StateVariableIterator  i = Vars->begin();
  StateVariableIterator  e = Vars->end();
  // Multiple buffers may be required:
  
  while(i != Vars->end()) {
    CStateVariableBuffer buf(m_nBufferSize);
    i = EmitStateVariableBuffer(buf, i, e);
    buf.SetRun(GetRunNumber());
    buf.SetCpuNum(getCpu());
    buf.Route();		//Increment else spectcl eff. is wrong
  }

}
/*!
    Triggers a snapshot scaler readout.
    Action is similar to that of scaler readout,
    however the existsing event buffer is 
    maintained and this buffer is sent around it.

*/
void 
CExperiment::TriggerSnapshotScaler()  
{
  
  CNSCLScalerBuffer buffer(m_nBufferSize);
  vector<unsigned long> scalers;
  try {
    scalers = m_Scalers.Read();
    m_Scalers.Clear();
  }
  catch( DesignByContractException& rContractViolation) {
    string msg(rContractViolation);
    cerr << "Interface contract violation: " << msg << endl;
    cerr << "Detected in user code called by CExperiment::TriggerSnapshotScaler\n";
    cerr << "Any partially read scalers will be discarded\n";
    scalers.erase(scalers.begin(), scalers.end());
  }
  catch (string msg) {
    cerr << "CExperiment::TriggerSnapshotScaler caught a string exception reading out the scalers:\n";
    cerr << msg << endl;
    cerr << "Any partially read scalers will be discarded\n";
    scalers.erase(scalers.begin(), scalers.end());


  }
  catch (char* msg) {
    cerr << "CExperiment::TriggerSnapshotScaler caught a char* exception reading out scalers:\n";
    cerr << msg << endl;
    cerr << "Any partially read scalers will be discarded\n";
    scalers.erase(scalers.begin(), scalers.end());
  }
  catch (CException& exception) {
    cerr << "CExperiment::TriggerSnapshotScaler caught an NSCLException reading out the scalers:\n";
    cerr << exception.ReasonText() << " while " << exception.WasDoing() << endl;
    cerr << "Any partially read scalers will be discarded\n";
    scalers.erase(scalers.begin(), scalers.end());
  }
  catch (...) {
    cerr << "CExperiment::CTriggerSnapshotScaler caught an exception of an unanticipated type while reading scalers\n";
    cerr << "Any partially read scalers will be discarded\n";
    scalers.erase(scalers.begin(), scalers.end());
  }
  
  // Sum the scalers into the snapshot scaler totals vector.
  // If necessary, the scaler vector is extended.

  for(int i = 0; i < scalers.size(); i++) {
    if(i < m_IntervalSums.size()) {
      m_IntervalSums[i] += scalers[i];
    }
    else {
      m_IntervalSums.push_back(scalers[i]);
    }
  }
  // Now emit the scaler buffer, and adjust the times:

  buffer.PutScalerVector(scalers);
  buffer.SetType(SNAPSCBF);
  buffer.SetStartTime(m_LastSnapTime);
  buffer.SetEndTime(m_LastSnapTime = GetElapsedTime()/10);

  buffer.SetRun(GetRunNumber());
  buffer.SetCpuNum(getCpu());
  buffer.Route();
		     
}  

/*!
    Creates a buffer or potentially a set of buffers,
    fills them with documentation
    information about the registered packet types.
    Buffers are routed out of band with any physics
    event buffer which might be being filled.

*/
void 
CExperiment::TriggerDocBuffer()  
{
  CDocumentedPacketManager* pManager = CDocumentedPacketManager::getInstance();
  DocumentationPacketIterator  i = pManager->begin();
  while(i != pManager->end()) {
    CNSCLDocumentationBuffer buf(m_nBufferSize);

    i = EmitDocBuffer(i, pManager->end(), buf);
    buf.SetRun(GetRunNumber());
    buf.SetCpuNum(getCpu());
    buf.Route();		// Increment else spectcl's smapling eff. is wrong
  }
  
}  

/*!
    Adds a scaler module to the readout subsystem.
    

	\param pScaler Pointer to the module to add.
*/
void 
CExperiment::AddScalerModule(CScaler* pScaler)  
{
  m_Scalers.AddScalerModule(pScaler);
}  

/*!
    Removes a scaler module from the readout
    system.

	\param  pScaler Pointer to the module to remove.

*/
void 
CExperiment::RemoveScalerModule(CScaler* pScaler)  
{
  m_Scalers.DeleteScalerModule(pScaler);
}
/*!
    Retrieves the current value of the scaler trigger pointer.
*/
CScalerTrigger*
CExperiment::getScalerTrigger()
{
  return m_pScalerTrigger;
}
/*!
   Sets a new scaler trigger module.
   
*/
void
CExperiment::setScalerTrigger(CScalerTrigger* pTrigger)
{
  CReadoutMain* pApp   = CReadoutMain::getInstance();
  CRunState*    pState = pApp->getRunState();
  if (pState->getState() != CRunState::Inactive) {
    throw 
      string("Attempting to set the scaler trigger when state is not inactive");
  }
  else {
    m_pScalerTrigger = pTrigger;
  }
}
/*!
  Emits the start of run buffer.  A start of run 
  buffer contains a standard buffer header as well 
  as:
  # Run Title - up to 79 characters of title information followed by a null.
  # A long word of zero indicating that the buffer was emitted at the beginning of the run.
  # A time structure containing the fields:
    - month - month numbered from 1 - jan to 12 - dec.
    - day   - day within the month numbered from 1 - 31
    - year  - e.g. 2002
    - hours - Hour within the day numbered from 0 = midnight to 23 = 11pm.
    - min   - Minutes within the hour numbered from 0 to 59.
    - sec   - Seconds within the minut numbered from 0 to 59.
    - tenths - Tenths of a second within the second  which 
               contains 0 for all Unix implementations since tod information
               is only accessible to the second.
  */
void
CExperiment::EmitStart()
{
  CNSCLOutputBuffer::ClearSequence(); // Begin run always has zero for sequence number.

  // Need a bunch of stuff:
  // Title run number, time offset and time of day.

  CNSCLControlBuffer buffer(m_nBufferSize);
  buffer.PutTitle(MyApp.getTitle());
  buffer.PutTimeOffset(GetElapsedTime());
  buffer.SetRun(GetRunNumber());
  buffer.SetCpuNum(getCpu());
  buffer.SetType(BEGRUNBF);
  buffer.Route();

}
/*!
   Emits an end run buffer.  End run bufers look just like Start run buffers
   (see EmitStart()), however the elapsed time value is nonzero.
*/
void 
CExperiment::EmitEnd()
{
  CNSCLControlBuffer buffer(m_nBufferSize);
  buffer.PutTitle(MyApp.getTitle());
  buffer.PutTimeOffset(GetElapsedTime()/10);
  buffer.SetRun(GetRunNumber());
  buffer.SetCpuNum(getCpu());
  buffer.SetType(ENDRUNBF);
  buffer.Route();

  flushData();


}
/*!
  Emits a pause run buffer.  Pause run buffers also look like Begin buffers with
  nonzero time.  They indicate a temporary halt in data taking.
*/
void
CExperiment::EmitPause()
{
  CNSCLControlBuffer buffer(m_nBufferSize);
  buffer.PutTitle(MyApp.getTitle());
  buffer.PutTimeOffset(GetElapsedTime()/10);
  buffer.SetRun(GetRunNumber());
  buffer.SetCpuNum(getCpu());
  buffer.SetType(PAUSEBF);
  buffer.Route();
  flushData();
  
}
/*!
  Emits a resume run buffer.  Resume run buffers are like end run buffers with nonzero time.
  They indicate  a resumption of data taking.
*/
void
CExperiment::EmitResume()
{
  CNSCLControlBuffer buffer(m_nBufferSize);
  buffer.PutTitle(MyApp.getTitle());
  buffer.PutTimeOffset(GetElapsedTime()/10);
  buffer.SetRun(GetRunNumber());
  buffer.SetCpuNum(getCpu());
  buffer.SetType(RESUMEBF);
  buffer.Route();

}
/*!
   Starts the event trigger.  This involves creating a thread which checks 
   to see if a trigger is present.  The thread will call our
   ReadEvent function in its own context.  The thread will synchronize
   by holding the global sync mutex for several passes through its main
   loop.
  
   \exception  CDuplicateSingleton - If the thread object already exists.


   */
void
CExperiment::StartTrigger()
{
  if(m_pTThread) {		// Should be null...else running.
    throw CDuplicateSingleton("Creating trigger thread",
			      "TriggerThreadObject");
  }
  m_pTThread = new CTriggerThread(this, m_pTrigger);
  m_pTThread->Start();

}
/*!
  Stops the event trigger.  This prevents future events from being 
  acquired.  Note that the trigger is processed in a separate thread.
  This function will block until the trigger thread has exited.

  \note There may not be a trigger thread (paused -> inactive transition e.g.)
  calling StopTrigger in this case is a no-op.

*/
void
CExperiment::StopTrigger()
{
  if(m_pTThread) {
    m_pTThread->Stop();
    delete m_pTThread;
    m_pTThread = (CTriggerThread*)NULL;   
  }
}
/*!
   Creates a Run Variable buffer.  Run variables buffers contain a set
   of run variables in the form varname=value\0. 

   \param Buffer - reference to a CRunVariableBuffer which will be filled
                   with formatted run variable strings.
   \param start  - A RunVariableIterator which indicates the first 
                  run variable to insert in the buffer.
   \param end   - A RunVariableIterator pointing past the last variable to
                  format into the buffer.  

   \return  The final iterator:
   -  end if all entities fit in the buffer. 
   -  the first entity which did not fit in the buffer if the buffer filled
      before inserting all entities.

 */
RunVariableIterator
CExperiment::EmitRunVariableBuffer(CRunVariableBuffer& rBuffer,
				   RunVariableIterator start,
				   RunVariableIterator end)
{
  while(start != end) {
    string item = 
      (start->second)->FormatForBuffer(m_nBufferSize*sizeof(short) - 
				       sizeof(bheader)-2);
    if(!rBuffer.PutEntityString(item)) break; // Won't fit if break.
    start++;
  }
  return start;			// Return the next entity.
}
/*!
   Emits a state variable buffer.   State variable buffers are filled
   with strings of the form "name=value\0" and are always an even number
   of bytes long.
   \param rBuffer - Reference to the CStateVariableBuffer being filled.
   \param start   - A StateVariableIterator which indicates the first variable
                   to format and insert in the buffer.
   \param end     - A State variable iterator indicating past the last 
                    variable to format and insert in the buffer.
    \return The final iterator:
       - end if all variables have been formatted and inserted.
       - the first entry which did not fit in the buffer if the buffer
         filled before inserting all entities.
 */
StateVariableIterator
CExperiment::EmitStateVariableBuffer(CStateVariableBuffer& rBuffer,
			       StateVariableIterator& start,
			       StateVariableIterator& end)
{
  while(start != end) {
    CStateVariable* pv = start->second;

    string item = pv->FormatForBuffer((m_nBufferSize*sizeof(short) - 
				       sizeof(bheader)-2));
    if(!rBuffer.PutEntityString(item)) break;
    start++;
  }
  return start;
}
/*!
   Emits a documentation buffer.  Documentation buffers list the set of
   documented packet ids.  Packet Ids are used to identify the sub-contents
   of events in the physics data buffers.  Making a packet id documented 
   provides useful documentation information.
   \param s   - Iterator `pointing' at the first packet documentation
                object to be put into the buffer.
   \param e   - Iterator `pointing' past the last packet documentation object
                to be put into the buffer.
   CNSCLDocumentationBuffer& rBuffer - Refers to the buffer into which
               the documentation objects will be formatted.

   \return - The iterator past the last item formatted:
   - e if all items were formatted into the buffer.
   - else an iterator to the first item to put into the next buffer.

 */
DocumentationPacketIterator
CExperiment::EmitDocBuffer(DocumentationPacketIterator s,
			   DocumentationPacketIterator e,
			   CNSCLDocumentationBuffer& b)
{
  while(s != e) {
    string item = (*s)->Format();
    if(!b.PutEntityString(item)) break;
    s++;
  }
  return s;
}
/*!
    Handles buffer overflows.  The actual event buffer is created at double the
    size of the system buffer size.  Data buffers then get tight packed by 
    filling them with events until the used part of the buffer is larger than the
    system buffer size.  At that point, a second buffer is created and the
    overflowed event is copied into that.  The original buffer is then resized
    and routed.

    \param header - DAQWordBufferPtr& [in] - Pointer to the first word of the 
                                     overflowing event.
   \param end   - DAQWordBufferPtr[in]& - Pointer past the last word of the event
                                that overflows the buffer.
*/
void
#ifndef HIGH_PERFORMANCE
CExperiment::Overflow(DAQWordBufferPtr& header,
		      DAQWordBufferPtr& end)
#else /* HIGH_PERFORMANCE */
CExperiment::Overflow(unsigned short*  header,
		      unsigned short*  end)
#endif /* HIGH_PERFORMANCE */
{
   
   // Copy the overflowing event to a new buffer:
   
   CNSCLOutputBuffer::IncrementSequence();
   CNSCLPhysicsBuffer* pNewBuffer = new CNSCLPhysicsBuffer(m_nBufferSize*2);
#ifndef HIGH_PERFORMANCE
   DAQWordBufferPtr    pDest      = pNewBuffer->StartEvent();
   DAQWordBufferPtr    pSrc       = header;
   DAQWordBufferPtr    pEnd       = end;
   while(pSrc != pEnd) {
      *pDest = *pSrc;
      ++pDest; ++pSrc;                // Preinccrement is fastest.
   }
   pNewBuffer->EndEvent(pDest);
#else /* HIGH_PERFORMANCE */
   unsigned short*    pDest      = pNewBuffer->StartEvent();
   unsigned short*    pSrc       = header;
   unsigned short*    pEnd       = end;
   unsigned short     nWords    = (end - header);
   memcpy(pDest, pSrc, nWords * sizeof(unsigned short));
   pNewBuffer->EndEvent(pDest + nWords);
#endif /* HIGH_PERFORMANCE */
   
   // Retract the event from the old buffer and route it:
   
   m_EventBuffer->RetractEvent(header);
   m_EventBuffer->SetRun(GetRunNumber());
   m_EventBuffer->SetCpuNum(getCpu());
   m_EventBuffer->Resize(m_nBufferSize);
   m_EventBuffer->Route();
   delete m_EventBuffer;
   
   // And put the new event buffer in place as the class member.:
   
   m_EventBuffer = pNewBuffer;

}
/*!
   Get the run number from the run state variables.
*/
unsigned short 
CExperiment::GetRunNumber() const
{

  // Locate the state variable command object as it containst the
  // database of state variables.


  CReadoutMain* pReadout        = CReadoutMain::getInstance();
  CInterpreterShell* pShell     = pReadout->getInterpreter();
  CInterpreterCore*  pCore      = pShell->getInterpreterCore();
  CStateVariableCommand& rState(*(pCore->getStateVariables()));

  // Now look for the state variable named "run" as that has the run number.

  StateVariableIterator i = rState.find(string("run"));
  if(i == rState.end()) {
    throw CNoSuchObjectException("Getting the run number variable object",
				 "run");
  }

  return atoi((i->second)->Get(TCL_GLOBAL_ONLY));

}
/*
  :Get the CPU node number from the run state variables:
*/
unsigned short
CExperiment::getCpu() const
{

  // Locate the state variable command object as it containst the
  // database of state variables.


  CReadoutMain* pReadout        = CReadoutMain::getInstance();
  CInterpreterShell* pShell     = pReadout->getInterpreter();
  CInterpreterCore*  pCore      = pShell->getInterpreterCore();
  CStateVariableCommand& rState(*(pCore->getStateVariables()));

  // Now look for the state variable named "run" as that has the run number.

  StateVariableIterator i = rState.find(string("cpu"));
  if(i == rState.end()) {
    throw CNoSuchObjectException("Getting the run number variable object",
				 "run");
  }

  return atoi((i->second)->Get(TCL_GLOBAL_ONLY));
}
/*!
   Get the elapsed run time.  
   \return long elapsed run time in .1 seconds.

*/
unsigned long
CExperiment::GetElapsedTime() const
{
  // Note that CTimer::GetElapsedTime's units are ms.

  return (MyApp.getClock().GetElapsedTime()) / 100;
}
//
// Set up the data store:
//

void
CExperiment::setupDataStore()
{
  m_pDataStore = &(DAQDataStore::instance());

  // This try/catch block deals with an issue in g++-3.x
  // with exceptions not always propagating correctly out of
  // a shared lib that we see in ReadoutMain.cpp
  //
  int port;
  struct servent* serviceInfo = getservbyname("sdlite-buff",
					      "tcp");
  if (serviceInfo) {
    port = ntohs(serviceInfo->s_port);
  } 
  else {
    port = 2701;
  }

					      
  

  m_pDataStore->setSourcePort(port);
}
// Flush data in buffers.
void
CExperiment::flushData()
{
  
  BufferedRecordWriter* pWriter = m_pDataStore->getSource();
  pWriter->flush();
}
