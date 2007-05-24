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



//  ReadoutStateMachine.h:
//
//    This file defines the ReadoutStateMachine class.
//
// Author:
//    Ron Fox
//    NSCL
//    Michigan State University
//    East Lansing, MI 48824-1321
//    mailto:fox@nscl.msu.edu
//
//  Copyright 1999 NSCL, All Rights Reserved.
//
/////////////////////////////////////////////////////////////

#ifndef __READOUTSTATEMACHINE_H  //Required for current class
#define __READOUTSTATEMACHINE_H
                               //Required for base classes
#ifndef __STATEMACHINE_H
#include "StateMachine.h"
#endif                               
                               //Required for 1:1 associated classes
#ifndef __DAQINTERFACE_H
#include "daqinterface.h"
#endif

#ifndef MTYPES
#include "buffer.h"
#endif

#ifndef _STL_VECTOR_H
#include <vector>
#define _STL_VECTOR_H
#endif


#ifndef __DAQ_SPECTRODAQ_H
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include <spectrodaq.h>
#define __DAQ_SPECTRODAQ_H
#endif

#ifndef __DAQTYPES_H
#include <daqdatatypes.h>
#endif

#ifndef __CRT_TIME_H
#include <time.h>
#ifndef __CRT_TIME_H         // In case time.h has this gaurd too.
#define __CRT_TIME_H
#endif
#endif

/*!
 Specialization of StateMachine to handle
 a data acquisition run.  The constructor of
 this statemachine sets up the states and
 reads in the transition diagram from file.

*/
class ReadoutStateMachine  : public StateMachine        
{
private:
  unsigned            m_PriorState;	 // Previous state.
  time_t              m_nSegmentStart;   // When this run segment started. 
  unsigned            m_nPriorDuration;  // How long all prior run have run.
  unsigned            m_nElapsedTime;	 // Total   seconds  into the run
  unsigned            m_nLastScalerRead; // Last time scaler readout done.
  unsigned            m_nLastSnapShot;   // List time snapshot scalers read.
  unsigned long       m_nSequenceNumber; // Buffer sequence number.
  STD(vector)<UINT32> m_nPartialSums;    // Partial scaler sums
  void*               m_pCommandChannel; // Channel for command data.
  State*              m_pInactive;
  State*              m_pActive;
  State*              m_pPaused;
  State*              m_pExiting;
  DAQDataStore*       m_pDataStore; /* Spdaqlite. */

public:
  ReadoutStateMachine ();
  virtual  ~ ReadoutStateMachine ( )       //Destructor
  { OnCleanup(0); }
  
			//Copy constructor: Not allowed.
private:
  ReadoutStateMachine (const ReadoutStateMachine& aReadoutStateMachine );
			//Operator= Assignment Operator

  ReadoutStateMachine 
     operator= (const ReadoutStateMachine& aReadoutStateMachine);
                                

			//Operator== Equality Operator
			//Update to access 1:M part class attributes
			//Update to access 1:1 associated class attributes
			//Update to access 1:M associated class attributes      
  int operator== (const ReadoutStateMachine& aReadoutStateMachine);
  //
  //  Selectors (read access):
  //

  unsigned getPriorState() const
  {
    return m_PriorState;
  }
  unsigned getElapsedTime() const
  {
    return m_nElapsedTime;
  }
  const void* getCommandChannel() const
  {
    return m_pCommandChannel;
  }
  unsigned long getSequenceNumber() const
  {
    return m_nSequenceNumber;
  }


  // Selectors (write):  These are available to derived classes:

protected:
  void setPriorState (unsigned am_PriorState)
  { 
    m_PriorState = am_PriorState;
  }
  void setElapsedTime (unsigned am_nElapsedTime)
  { 
    m_nElapsedTime = am_nElapsedTime;
  }
  void setCommandChannel (void* am_pCommandChannel)
  { 
    m_pCommandChannel = am_pCommandChannel;
  }

  //  Functional operations:
public:
  bool PollForCommand () ;
  unsigned GetCommand ()  ;
  unsigned GetPriorState ()  ;
  void SetPriorState ()  ;
  void EmitPause ()  ;
  void EmitResume ()  ;
  void EmitStart ()  ;
  void EmitStop ()  ;
  void EmitScaler ()  ;
  void EmitSnapshot ()  ;
  DAQWordBuffer* GetBuffer ()  ;
  DAQWordBuffer* GetBuffer(unsigned nWords);
  void           RouteBuffer(DAQWordBuffer* pBuffer);
  unsigned GetRunTime ()  ;
  void ClearRunTime ()  ;
  void DeclareTick (unsigned  nTicks=1)  ;

  void NextSequence() { m_nSequenceNumber++; }
  void ResetSequence() {m_nSequenceNumber = 0; }

  // Overrides from the base class

  virtual   void OnInitialize ()  ;
  virtual   void OnIllegalTransition (unsigned nCurrentState, 
				      unsigned nEvent)  ;
  virtual   void OnCleanup (unsigned  nState)  ;

  // Buffer formatting assistance:

  DAQWordBufferPtr GetBody(DAQWordBuffer* pBuffer); // Returns ptr to body.
  void FormatHeader(DAQWordBuffer* pBuffer, 
		    UINT16 nWords, UINT16 nType, UINT16 nEntities);
  void UpdateRunTime();
  void     NewRunSegment();	//!< Diddle clock for Begin or Resume.

  // Internal members:

private:
  unsigned CommandToEvent(RunControlCmd& command);
  virtual bool   IsControlBufferType(INT16 nType); // Derived classes
  virtual bool   IsScalerBufferType(INT16 nType);  // can Add types
  void     GetTime(bftime* t);
  void     EmitControlBuffer(INT16 nBufferType);
  void     EmitScalerBuffer(INT16 nBufferType, unsigned nLastTime);
};

#endif



