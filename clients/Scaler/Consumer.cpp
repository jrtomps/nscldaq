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

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";
// Class: CConsumer
// Base class for a data acquisition system 
// consumer application.
//
// Author:
//    Ron Fox
//    NSCL
//    Michigan State University
//    East Lansing, MI 48824-1321
//
//
// RF  - Modified to report to cerr sinks which could not be added to the
//       link manager.
//////////////////////////.cpp file/////////////////////////////////////////////////////

#include <config.h>
#include "Consumer.h"    				
#include <Iostream.h>
#include <NSCLBufferFactory.h>
#include <NSCLDaqBuffer.h>
#include <NSCLBufferCreator.h>
#include <NSCLStateChangeCreator.h>
#include <NSCLScalerCreator.h>
#include <NSCLPhysicsCreator.h>

#include <NSCLDaqBuffer.h>
#include <NSCLStateChangeBuffer.h>
#include <NSCLScalerBuffer.h>
#include <NSCLEventBuffer.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


static CNSCLStateChangeCreator StateChange;
static CNSCLPhysicsCreator     Physics;
static CNSCLScalerCreator      Scaler;

// Functions for class CConsumer
//////////////////////////////////////////////////////////////////////////////
//  Function:
//     CCOnsumer()
//  Operation type:
//     Constructor
//
CConsumer::CConsumer() :
  m_DaqBuffer(0),
  m_eRunState(RSUnknown),
  m_nRunNumber(-1),
  m_sTitle(string(">Unknown<"))
{

}
/////////////////////////////////////////////////////////////////////////////
//
// Function:
//   ~Consumer()
// Operation Type:
//   Destructor.
// 
CConsumer::~CConsumer()
{
  DataSourceList::iterator p=m_DataSources.begin();
  for(; p != m_DataSources.end(); p++) {
    CDataSource* pSource = (*p).second;
    if(!pSource->Disconnect()) {
      DAQURL srcurl(pSource->getURL());
      cerr << "Unable to form connection for Data source: " << endl;
#if __GNUC__ >= 3		// 3.0 or later spectrodaq not a stream..
      cerr << " URL - " << srcurl.toString().c_str()  << endl;
#else
      cerr << " URL - " << srcurl << endl;
#endif
    }
    delete pSource;
  }
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     OnBuffer(DAQWordBuffer& rBuffer)
//  Operation Type: 
//     
void CConsumer::OnBuffer(DAQWordBuffer& rBuffer)  
{
  // Called when a buffer has been received from the daq.
  // The buffer type is determined, an appropriately typed buffer is generated
  // and the appropriate On<BufferType> member is called.
  // 
  // 

  // The buffer factory kicks back the right kind of buffer:

  CNSCLBufferFactory fact;
  CNSCLDaqBuffer* pBuffer = fact.Create(rBuffer);
  m_nRunNumber = pBuffer->getRunNumber();

  switch(pBuffer->getBufferType()) {
  case PhysicsData:
    OnPhysicsBuffer(*(CNSCLEventBuffer*)pBuffer);
    m_eRunState = RSActive;
    break;
  case BeginRun:
    OnBeginBuffer(*(CNSCLStateChangeBuffer*)pBuffer);
    m_eRunState = RSActive;
    break;
  case EndRun:
    OnEndBuffer(*(CNSCLStateChangeBuffer*)pBuffer);
    m_eRunState = RSHalted;
    break;
  case PauseRun:
    OnPauseBuffer(*(CNSCLStateChangeBuffer*)pBuffer);
    m_eRunState = RSPaused;
    break;
  case ResumeRun:
    OnResumeBuffer(*(CNSCLStateChangeBuffer*)pBuffer);
    m_eRunState = RSActive;
    break;
  case IncrementalScalers:
  case SnapshotScalers:
    OnScalerBuffer(*(CNSCLScalerBuffer*)pBuffer);
    m_eRunState = RSActive;
    break;
  default:
    OnOtherBuffer(*pBuffer);	// Other buffer will need to update state.
    break;
  }
  // Release the daq buffer:

 
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     OnPhysicsBuffer(CNSCLEventBuffer& rEventBuffer)
//  Operation Type: 
//     
void CConsumer::OnPhysicsBuffer(CNSCLEventBuffer& rEventBuffer)  
{
  //  Called when a physics data buffer has arrived.
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     OnScalerBuffer(CNSCLScalerBuffer& rScalerBuffer)
//  Operation Type: 
//     
void CConsumer::OnScalerBuffer(CNSCLScalerBuffer& rScalerBuffer)  
{
  // Called when a scaler buffer is received.
  // 
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     OnBeginBuffer(CNSCLStateChangeBuffer& rStateChange)
//  Operation Type: 
//     
void CConsumer::OnBeginBuffer(CNSCLStateChangeBuffer& rStateChange)  
{
  // Called when a begin run buffer is received.

  m_sTitle = rStateChange.getTitle();
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     OnEndBuffer(CNSCLStateChangeBuffer& rStateChange)
//  Operation Type: 
//     
void CConsumer::OnEndBuffer(CNSCLStateChangeBuffer& rStateChange)  
{
  // Called when an end run buffer is recieved from the DAQ.
  
  m_sTitle = rStateChange.getTitle();
  m_eRunState = RSHalted;
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     OnPauseBuffer(CNSCLStateChangeBuffer& rStateChange)
//  Operation Type: 
//     
void CConsumer::OnPauseBuffer(CNSCLStateChangeBuffer& rStateChange)  
{
  // Called when a pause run buffer is received by the daq.
  
  m_sTitle = rStateChange.getTitle();
  m_eRunState = RSPaused;
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     OnResumeBuffer(CNSCLStateChangeBuffer& rStateChange)
//  Operation Type: 
//     
void CConsumer::OnResumeBuffer(CNSCLStateChangeBuffer& rStateChange)  
{
  // Called when a resume run buffer is recieved from the daq.
  
  m_sTitle = rStateChange.getTitle();
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:
//    OnOtherBuffer(CNSCLDaqBuffer& rBuffer)
//  Operation type:
//     overridable.
//
void
CConsumer::OnOtherBuffer(CNSCLDaqBuffer& rBuffer)
{
  // Called for unanticipated buffer type (e.g. user buffer?).


}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     AddDataSource(const string& LinkDesignator, inut nTag, 
//                   int nMask, int nDelivery)
//  Operation Type: 
//     
Bool_t 
CConsumer::AddDataSource(const string& LinkDesignator, int nTag, 
			 int nMask, int nDelivery)  
{
  // Get rid of any existing source at the same URL (for now URL's are unique).
  //
  DeleteDataSource(LinkDesignator);

  // Add the new source.
  CDataSource* pSource = new CDataSource(LinkDesignator, nTag, nMask, 
					 nDelivery);
  m_DataSources[LinkDesignator] = pSource;
  return kfTRUE;
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     DeleteDataSource(const string& LinkDesignator)
//  Operation Type: 
//     
Bool_t CConsumer::DeleteDataSource(const string& LinkDesignator)  
{
  // Deletes a previously added data source.
  

  DataSourceList::iterator p = m_DataSources.find(LinkDesignator);
  if(p != m_DataSources.end()) {
    delete (*p).second;
    m_DataSources.erase(LinkDesignator);
    return kfTRUE;
  }
  return kfFALSE;
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:
//    EnableDataTaking()
//  
void
CConsumer::EnableDataTaking()
{
  DataSourceList::iterator p = m_DataSources.begin();
  while(p != m_DataSources.end()) {
    ((*p).second)->Connect();
    p++;
  }
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:
//    DisableDataTaking()
//
void
CConsumer::DisableDataTaking()
{
  DataSourceList::iterator p = m_DataSources.begin();
  while(p != m_DataSources.end()) {
    ((*p).second)->Disconnect();
    p++;
  }
}

//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     CheckForData(struct timeval* pWaitTime, int nTag)
//  Operation Type: 
//     
void CConsumer::CheckForData(struct timeval* pWaitTime, int nTag)  
{
  // Attempts to get data from the sinks.
  // For all sources on which data has arrived, OnBuffer is called, 
  //
  // Parameters:
  //    struct timeval* pWaitFor
  //  Time to wait for a buffer before giving up.
  //  Null implies blocking, 0 implies polling.
  //    int tag:
  //      Tag to accept buffer for.
  m_DaqBuffer.SetTag(nTag);
  m_DaqBuffer.Accept(pWaitTime);
  if(m_DaqBuffer.GetLen() != 0) {
    OnBuffer(m_DaqBuffer);
    m_DaqBuffer.Release();	// Release the buffer - implicitly sets size to
				// zero to avoid Spectrodaq Deadlocks.
  }
}
//
// Should have been in-lineable
//
void 
CConsumer::setRunState(DAQRunState am_eRunState)
{ 
  m_eRunState = am_eRunState;
}

DAQRunState 
CConsumer::getRunState()
{ 
  return m_eRunState;
}


int
CConsumer::operator()(int argc, char** pargv)
{

    //
    // Set up the default buffer recognizers.  Done here because:
    //   This class represents a singleton object.
    //   This class is in all DAQConsumer programs using the framework.
    // 
    CNSCLBufferFactory::Register(PhysicsData, Physics);
    CNSCLBufferFactory::Register(BeginRun,    StateChange);
    CNSCLBufferFactory::Register(EndRun,      StateChange);
    CNSCLBufferFactory::Register(PauseRun,    StateChange);
    CNSCLBufferFactory::Register(ResumeRun,   StateChange);
    CNSCLBufferFactory::Register(IncrementalScalers, Scaler);
    CNSCLBufferFactory::Register(SnapshotScalers,    Scaler);

    return 0;
}
