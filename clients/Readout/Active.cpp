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


//
// Header Files:
//

#include <config.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include "Active.h"                               
#include "ReadoutStateMachine.h"
#include <daqinterface.h>
#include <skeleton.h>
#include <string>
#include <time.h>
#include <camac.h>
#include <macros.h>
#include <daqdatatypes.h>
#include <buftypes.h>
#include <spectrodaq.h>
#include <CVMEInterface.h>
#include "CESTrigger.h"
#include "VMETrigger.h"
#include "TestTrigger.h"

#include "CAMACBusy.h"
#include "VMEBusy.h"
#include "TestBusy.h"

#include <Iostream.h>
#include <typeinfo>

#include "Readout.h"

extern DAQBuff mydaq;

#ifdef HAVE_WIENERUSBVME_INTERFACE
static const unsigned SCALEDOWN = 10;
#else
static const unsigned SCALEDOWN = 1000;	// # times to poll for events.
#endif

static const char* Copyright=
"Active.cpp: Copyright 1999 NSCL, All rights reserved\n";

static const int TESTMODE = -1;	// Special test value for UsingVME

extern ReadoutStateMachine* gpStateMachine;

extern int    UsingVME;

// Functions for class Active

//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void Enter ( RunState& rStateMachine )
//  Operation Type:
//     Action Override.
//
void 
Active::Enter(StateMachine& rStateMachine) 
{
// Performs state entry processing.
//   If the prior state was Inactive, submit
//   a Begin event
//   If the prior state was Paused, submit
//   a Resume event.
//   Set up the scaler readout timing information.
//   Clear the DAQ hardware
//   Enable the trigger.
//   Clear the computer busy.
//
//
//  Formal Parameters:
//      StateMachine& rMachine:
//          Reference to the executing state
//          machine.  This is actually an object of
//          or derived from the class RunStateMachine.
//

  ReadoutStateMachine& rRun((ReadoutStateMachine&)rStateMachine);
  string  Prior(rRun.StateToName(rRun.GetPriorState()));
  string  Paused("PAUSED");
  string  Inactive("INACTIVE");

  m_EndRunRequested = false;	// Let the run run.

  //
  //  The kind of start event emitted depends on how we got here:
  //
  if(Prior == Inactive) {
    rRun.ClearRunTime();		// Run starts from t= 0.
    rRun.NewRunSegment();
    rRun.EmitStart();
    daq_StartRun();
  }
  if(Prior == Paused) {
    rRun.NewRunSegment();
    daq_ResumeRun();
    rRun.EmitResume();
  }
  m_pTrigger->Initialize();

  // We keep internal timing information to know when to readout scalers.
  // This should be updated now:

  CVMEInterface::Lock();
  SetBusy();			// This is probably already set, but...
  UpdateInternalTime(); 
  ClearEventTrigger();		// Any pending triggers are cleared before
  EnableTrigger();		// the event trigger is enabled.
  ::initevt();
  ::iniscl();
  ::clearevt();		// Experiment specific initialization and
  ::clrscl();			// device clears and
  FrontPanelClear();		// NIMOUT clear before
  m_pBusy->ScalerClear();
  if(Prior == Inactive) {
    rRun.ClearRunTime();	// Allow for long initialization sequences!
  }
  ClearBusy();			// Dropping the computer busy.
  CVMEInterface::Unlock();

}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void Leave ( StateMachine& rMachine )
//  Operation Type:
//     Action override.
//
void 
Active::Leave(StateMachine& rMachine) 
{
// Performs state exit operations.
// The data acquisition hardware is
//  disabled, as are triggers, and the computer
//  busy is set.  
//
//  Formal Parameters:
//         StateMachine& rMachine:
//              Reference to the state machine which is
//              executing.  The reference is actually to
//              an object which is derived from or is a
//              RunStateMachine object.
//

  ReadoutStateMachine& rRun((ReadoutStateMachine&)rMachine);

  CVMEInterface::Lock();
  SetBusy();			// Disable electronics triggers.
  ClearEventTrigger();		// Clear triggers in the timing hole.
  
  rRun.SetPriorState();
  CVMEInterface::Unlock();

}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    unsigned Run ( StateMachine& rMachine )
//  Operation Type:
//     Action Override.
//
unsigned 
Active::Run(StateMachine& rMachine) 
{
//  Peforms state processing.
//  In this case events are polled for at high rate,
//  and commands are polled for at  lower rates.
//  Commands received are transformed into events and
//  passed back to the state machine which, if necessary
//  will initiate the appropriate transition.
//
//  Formal Parameters:
//         StateMachine& rMachine:
//              Reference to the state machine which is running.
//              Derived from or a RunStateMachine object.
//
// Exceptions:
  ReadoutStateMachine& rRun((ReadoutStateMachine&)rMachine);
  while(1) {

    m_pReader->ReadSomeEvents(SCALEDOWN);

    // Check for scalers and commands:

    CVMEInterface::Lock();
    if(CheckScalerTrigger(rRun)) {
      m_pBusy->ScalerSet();	// Hold computer busy while scalers are read.
      m_pReader->FlushBuffer();
      rRun.UpdateRunTime();
      rRun.EmitScaler();
      ClearScalerTrigger();
      m_pBusy->ScalerClear();  // busy, otherwise, the event will do it.
    }
    CVMEInterface::Unlock();

    if(rRun.PollForCommand()) {
      m_pReader->FlushBuffer();
      return rRun.GetCommand();
    }
    if(m_EndRunRequested) {
      m_pReader->FlushBuffer();
      CVMEInterface::Lock();
      m_pBusy->Set();
      m_pBusy->ScalerSet();
      CVMEInterface::Unlock();
      rRun.UpdateRunTime();
      return rRun.NameToEventId("END");
    }
  }
}
////////////////////////////////////////////////////////////////
// Function:
//    OnInitialize(StateMachine& rMachine)
// Operation Type:
//    Initialization hook
void
Active::OnInitialize(StateMachine& rMachine)
{
  //  Performs state specific initialization.  We assume that
  //  by now the CAMAC mapping has been set up, and CAMAC operations
  //  can be performed via the macros.h and camac.h macros.
  //  We must:
  //   1. Set the Gate generator to latched mode.


  CVMEInterface::Lock();
  if(UsingVME == TESTMODE) {
    if(!m_pTrigger) {
      m_pTrigger = new CTestTrigger;
    }
    if(!m_pBusy) {
      m_pBusy = new CTestBusy;
    }
  }
  else if(!UsingVME) {
    if(!m_pTrigger) {
      m_pTrigger = new CESTrigger(0);
    }
    if(!m_pBusy) {
      m_pBusy    = new CCAMACBusy();
    }
    m_pBusy->Initialize();
    m_pTrigger->Initialize();
    
  }

  // If we are using VME instead of CAMAC, all we have to do is
  // construct a new CNimout object and a new CCaenIO object and
  // initialize all of their outputs to zero.
  //
  else {
    if(!m_pNimout)
      m_pNimout = new CNimout(0x8000);  // don't worry, it gets deleted
    if(!m_pCaen)
      m_pCaen = new CCaenIO(0x444400); // so does this.
    if(!m_pTrigger) {
      m_pTrigger = new CVMETrigger(m_pCaen);
    }
    if(!m_pBusy) {
      m_pBusy    = new CVMEBusy(m_pNimout, m_pCaen);
    }
  }
  m_pTrigger->Initialize();
  m_pTrigger->Disable();
  m_pBusy->Initialize();
  
  if(!m_pReader) {
    m_pReader = new CReader((ReadoutStateMachine&)rMachine);
  }
  m_pReader->setTrigger(m_pTrigger);
  m_pReader->setBusy(m_pBusy);
  CVMEInterface::Unlock();
}

////////////////////////////////////////////////////////////
// Function
//    DisableTrigger()
// Operation Type:
//    Internal Utility
//    Available for derived classes.
//
void 
Active::DisableTrigger()
{
  CVMEInterface::Lock();
  m_pTrigger->Disable();
  CVMEInterface::Unlock();
}
//////////////////////////////////////////////////////////////
// Function:
//   EnableTrigger()
// Operation Type:
//    Internal Utility
//    Available for derived classes.
void
Active::EnableTrigger()
{
  m_pTrigger->Enable();
}
////////////////////////////////////////////////////////////////////
//  Function:
//    bool_t CheckEventTrigger()
//  Operation Type:
//    Internal Utility
//    Available for derived classes.
//
bool
Active::CheckEventTrigger()
{
  // Returns true if the event trigger is set.  This will be true if 
  // UsingVME is flase and either of the IT2/IT4 bits are set in the 
  // CAMAC branch highway driver for branch zero. If UsingVME is true,
  // then this will be true if the NIM input labeled 0 on the Caen v262
  // card is asserted.
  //

  return m_pTrigger->Check();
}
////////////////////////////////////////////////////////////////////
// Function:
//     bool_t CheckScaler Trigger(StateMachine& rMachine)
// Operation Type:
//     Internal Utility
//     Available to derived classes.
//
bool
Active::CheckScalerTrigger(StateMachine& rMachine)
{
  // Returns true if it's time to read a scaler trigger.
  // This is done as follows:   The current time is checked against
  // the time saved in m_LastReadoutTime.  If the difference in ms
  // is greater than the value returned from daq_GetScalerReadoutInterval,
  // return true.. note that successive calls to CheckScalerTrigger() after
  // that time interval passes will return true until ClearScalerTrigger() is
  // called.
  // Formal Parameters:
  //     StateMachine& rMachine:
  //           Reference to the state machine which is running us.  This
  //           is actually a ReadoutStateMachine& 
  //           It is used to declare ticks in the run duration.  

  time_t tNow;
  time(&tNow);
  if(tNow != m_ScalersLastChecked) {
    ReadoutStateMachine& rRun((ReadoutStateMachine&)rMachine);
    rRun.DeclareTick(tNow - m_ScalersLastChecked);
    m_ScalersLastChecked = tNow;
  }
  time_t interval = daq_GetScalerReadoutInterval();
  return ( (tNow - m_LastReadoutTime) >= interval);
}
////////////////////////////////////////////////////////////////////////
// Function:
//     void ClearScalerTrigger()
// Operation Type:
//     Internal,
//     Available in derived classes.
//
void
Active::ClearScalerTrigger()
{
  //   Clears the scaler readout trigger.  This is done by simply
  //   setting the m_LastReadoutTime member to the current time.
  //   That ensures that at least another readout time interval
  //   must pass until the next readout.
  //

  time(&m_LastReadoutTime);

}
/////////////////////////////////////////////////////////////////////
//  Function:
//      void  ClearEventTrigger()
//  Operation Type:
//      Internal,
//      Available for derived classes
//
void
Active::ClearEventTrigger()
{
  // Clears the bit register lam.
  //
  m_pTrigger->Clear();
}

////////////////////////////////////////////////////////////////////////
//  Function:
//     void ClearBusy()
//  Operation Type:
//     Internal, available to derived classes.
//
void
Active::ClearBusy()
{
  //  Clears the computer busy gate.  This involves issuing a stop
  //  Channel A of the gate generator,
  //
  m_pBusy->Clear();
}
////////////////////////////////////////////////////////////////////////
// Function:
//    void SetBusy()
// Operation Type:
//    Internal, available to derived classes.
void
Active::SetBusy()
{
  //    Sets the computer busy gate.  This involves issuing a stop channel
  //    A of the gate generator.
  
  m_pBusy->Set();
}
/////////////////////////////////////////////////////////////////////////
//
// Function:
//     void FrontPanelClear()
// Operation Type:
//     Internal, available to derived classes.
//
void
Active::FrontPanelClear()
{
  m_pBusy->ModuleClear();


}
///////////////////////////////////////////////////////////////////////////
// Function:
//   void UpdateInternalTime(StateMachine& rMachine)
// Operation Type:
//    Internal, available for derived classes.
//
void
Active::UpdateInternalTime()
{
  // Updates the internal times which control when ticks will be issued
  // to the run state machine, and when scalers will be read out.
  //

  m_ScalersLastChecked = time(&m_LastReadoutTime);

}

// EndRun - Static member function which allows readout software to request
//          an end of run.  
//   If the run is not active, this is a no-op.

void Active::EndRun()
{
  try {				// ensure we're really active...
    // Seems like these need to be static else the dynamic cast can get fooled!

    static State* pState = (gpStateMachine->GetCurrentStatePtr());
    static Active* pMe   = dynamic_cast<Active*>(pState);
    pMe->m_EndRunRequested = true;
  }
  catch (...) {
  }
}
/*!
   Sets a new trigger module for the system.  Note that
  the trigger module must have been dynamically allocated if
  it will be installed at destruction time for this object
  as the destructor will \em delete it.

  \param pNewTrigger (CTrigger* [in]):
      Pointer to the new trigger module.

*/
void
Active::SetTrigger(CTrigger* pNewTrigger) 
{
  m_pTrigger = pNewTrigger;
  m_pReader->setTrigger(pNewTrigger);
}

/*!
   Sets a new busy module for the system.  Note that
   the busy module must have been dynamically allocated
   if it will remain installed at the time the Active
   object is deleted since the destructor will \em delete
   it.
   \param pNewBusy (CBusy* [in]):
     Pointer to the new busy module.
*/
void
Active::SetBusy(CBusy* pNewBusy)
{
  m_pBusy  = pNewBusy;
  m_pReader->setBusy(pNewBusy);
}
