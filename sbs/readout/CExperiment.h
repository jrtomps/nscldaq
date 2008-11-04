#ifndef __CEXPERIMENT_H
#define __CEXPERIMENT_H
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

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

// Forwared definitions:

class RunState;			
class CScalerBank;
class CCompoundEventSegment;
class CBusy;
class CEventTrigger;
class CTriggerLoop;
class CRingBuffer;
class CEventSegment;
class CScaler;

/*!
  This class controls the experiment.  The experiment consists of two major
  components, scaler lists and events.  Both are organized as
  Composites which are the root of a tree of modules.

  The experiment holds the ring buffer pointer/handle and 
  takes care of moving data from local buffers into the ring buffer.
  Event data are acquired in a local buffer whose size can be set, and which
  must be big enough to hold a full event.


*/
class CExperiment // (final).
{
  // local data:

private:
  CRingBuffer*           m_pRing;	//!< Where event data eventually goes.
  RunState*              m_pRunState;   //!< state information about the run.
  CScalerBank*           m_pScalers;    //!< The scalers to read.
  CCompoundEventSegment* m_pReadout;    //!< The event segment root.
  CBusy*                 m_pBusy;       //!< The busy module.
  CEventTrigger*         m_pEventTrigger; //!< trigger for events.
  CEventTrigger*         m_pScalerTrigger; //!< scaler trigger.
  CTriggerLoop*          m_pTriggerLoop; //!< Thread that runs the trigger.

  size_t                 m_nDataBufferSize; //!< current event buffer size.

  uint32_t               m_nLastScalerTime; //

  // Canonicals:

public:
  CExperiment(std::string ringName,
	      size_t      eventBufferSize = 4096);
  ~CExperiment();		// This is a final class.
private:
  CExperiment(const CExperiment&);
  CExperiment& operator=(const CExperiment&);
  int operator==(const CExperiment&) const;
  int operator!=(const CExperiment&) const;

  // Member functions:

public:
  void   setBufferSize(size_t newSize);
  size_t getBufferSize() const;
  void   Start(bool resume=false);
  void   Stop(bool pause=false);
  void   AddEventSegment(CEventSegment*    pSegment);
  void   RemoveEventSegment(CEventSegment* pSegment);
  void   AddScalerModule(CScaler*    pModule);
  void   RemoveScalerModule(CScaler* pModule);

  void   EstablishTrigger(CEventTrigger* pEventTrigger);
  void   setScalerTrigger(CEventTrigger* pScalerTrigger);
  void   EstablishBusy(CBusy*            pBusyModule);
  void   ReadEvent();
  void   TriggerScalerReadout();
  void   DocumentPackets();
  void   ScheduleRunVariableDump();

};

#endif
