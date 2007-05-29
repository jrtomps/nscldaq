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


//////////////////////////CExperiment.h file//////////////////////////////////

#ifndef __CEXPERIMENT_H  
#define __CEXPERIMENT_H

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#ifndef __SPECTRODAQ_H
#include <spectrodaq.h>
#define __SPECTRODAQ_H
#endif
       
                               
#ifndef __CSTATUSMODULE_H
#include "CStatusModule.h"
#endif

#ifndef __CTRIGGER_H
#include "CTrigger.h"
#endif

#ifndef __CSCALERBANK_H
#include "CScalerBank.h"
#endif

#ifndef __CSCALER_H
#include "CScaler.h"
#endif

#ifndef __COMPOUNDEVENTSEGMENT_H
#include "CCompoundEventSegment.h"
#endif

#ifndef __CSTATETRANSITIONCOMMAND_H
#include "CStateTransitionCommand.h"
#endif

#ifndef __CRUNVARIABLE_H
#include <CRunVariableCommand.h>
#endif

#ifndef __CSTATEVARIABLECOMMAND_H
#include <CStateVariableCommand.h>
#endif

#ifndef __CDOCUMENTEDPACKETMANAGER_H
#include <CDocumentedPacketManager.h>
#endif


#ifndef __STL_VECTOR
#include <vector>
#define __STL_VECTOR
#endif

#ifndef __CNSCLPHYSICSBUFFER_H
#include <CNSCLPhysicsBuffer.h>
#endif



class CRunVariableBuffer;
class CStateVariableBuffer;
class CNSCLDocumentationBuffer;
class CScalerTrigger;
class CTriggerThread;
                                                        
/*!
   Encapsulates the details and general methods of
   the experiment specific stuff.
 */		
class CExperiment      
{ 
private:
  CNSCLPhysicsBuffer* m_EventBuffer;
  unsigned int        m_nBufferSize;

  CStatusModule* m_pStatusModule; //!< Pointer to module handling status.
  CTrigger*      m_pTrigger;	  //!< Trigger module handler. 
  CTriggerThread* m_pTThread;	   //!< Trigger checking thread.
  CScalerBank    m_Scalers;	  //!< Scaler bank (with modules). 
  CCompoundEventSegment m_EventReadout; //!< Event Readout object. 
  vector<unsigned long> m_IntervalSums; //!< Scaler sums over snapshot interval.
  unsigned long         m_LastSnapTime; //!< Time of last snapshot.
  unsigned long         m_LastScalerTime; //!< Time of last full scaler read.
  CScalerTrigger*       m_pScalerTrigger;
  unsigned int          m_nEventsAcquired;
  unsigned int 		m_nWordsAcquired;
  DAQDataStore*         m_pDataStore;
public:
  // Constructors, destructors and other cannonical operations: 
  
  CExperiment (unsigned nBufferSize = 4096); //!< Default constructor.
  CExperiment(CTrigger* pTriggerModule,
	      CEventSegment* pEventReadout,
	      CScaler*       pScalers = 0,
	      CStatusModule* pStatus = 0,
	      unsigned nBufferSize = 4096);
	      
	      
  ~ CExperiment ( );		/* Inline destructor in shlib is bad!! */

  
  //! Copy construction is forbidden:
private:
  CExperiment(const CExperiment& rhs);
public:
  //! Assignment is forbidden:
private:
  CExperiment& operator= (const CExperiment& rhs); //!< Assignment
public:
  //! Comparison makes no sense given that assignment and copy construction is forbidden
private:
  int         operator==(const CExperiment& rhs) const; //!< Comparison for equality.
  int         operator!=(const CExperiment& rhs) const;
public:
  
  // Selectors for class attributes:
public:
  
  CNSCLPhysicsBuffer* getBuffer()  {
    return m_EventBuffer;
  }
  
  CStatusModule* getStatusModule() {
    return m_pStatusModule;
  }
  CTrigger* getTrigger() {
    return m_pTrigger;
  }
  CScalerBank& getScalers() {
    return m_Scalers;
  }
  CCompoundEventSegment& getReadout() {
    return m_EventReadout;
  }
  // Mutators:
protected:  
#ifndef HIGH_PERFORMANCE

#endif /* ! HIGH_PERFORMANCE */
public:
  void setBufferSize(unsigned nBufferSize) {
    m_nBufferSize = nBufferSize;
  }
  
  // Class operations:
public:
  void Start (CStateTransitionCommand& rCommand)  ;
  void Stop (CStateTransitionCommand& rCommand)  ;
  void ReadEvent ()  ;
  void PostEvent();
  void AddEventSegment (CEventSegment* rSegment)  ;
  void RemoveEventSegment (CEventSegment* pSegment)  ;
  void RemoveEventSegment(CCompoundEventSegment::EventSegmentList::iterator i) {
    m_EventReadout.DeleteSegment(i);
  }
  void SetBusy ()  ;
  void ClearBusy ()  ;
  void EstablishTrigger (CTrigger* pTrigger)  ;
  void EstablishBusy (CStatusModule* pStatus)  ;
  void TriggerScalerReadout ()  ;
  void TriggerRunVariableBuffer ()  ;
  void TriggerStateVariableBuffer();
  void TriggerSnapshotScaler ()  ;
  void TriggerDocBuffer ()  ;
  void AddScalerModule (CScaler* pScaler)  ;
  void RemoveScalerModule (CScaler* pScaler)  ;
  void RemoveScalerModule(ScalerListIterator it) {
    RemoveScalerModule(*it);
  }
  CScalerTrigger* getScalerTrigger();
  void            setScalerTrigger(CScalerTrigger* pTrigger);
  unsigned long GetElapsedTime() const;
protected:
  void EmitStart();
  void EmitEnd();
  void EmitPause();
  void EmitResume();
  void StartTrigger();
  void StopTrigger();
  unsigned short GetRunNumber() const;
  void EmitScalerBuffer(unsigned int nBufferType, 
			vector<unsigned long>& scalers,
			unsigned long nIntervalStartTime);
  RunVariableIterator  EmitRunVariableBuffer(CRunVariableBuffer& Buffer, 
					     RunVariableIterator start,
					     RunVariableIterator stop);
  StateVariableIterator EmitStateVariableBuffer(CStateVariableBuffer& Buffer,
						StateVariableIterator& start,
						StateVariableIterator& stop);

  DocumentationPacketIterator EmitDocBuffer(DocumentationPacketIterator s,
					     DocumentationPacketIterator e,
					     CNSCLDocumentationBuffer&    b);
					     

#ifndef HIGH_PERFORMANCE
   void Overflow(DAQWordBufferPtr& header, 
	         DAQWordBufferPtr& End);
#else /* HIGH_PERFORMANCE */
   void Overflow(unsigned short*  header, 
	         unsigned short*  End);
#endif /* HIGH_PERFORMANCE */
   void setupDataStore();
   void flushData();
};

#endif


