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

// Class: CConsumer                     //ANSI C++
//
// Base class for a data acquisition system 
// consumer application.  Note that a user will need to 
// derive a class from this to make a useful application.
//
// Author:
//     Ron Fox
//     NSCL
//     Michigan State University
//     East Lansing, MI 48824-1321
//     mailto: fox@nscl.msu.edu
// 
// (c) Copyright NSCL 1999, All rights reserved Consumer.h
//

#ifndef __CONSUMER_H  //Required for current class
#define __CONSUMER_H


#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
                            //Required for base classes
#ifndef __SPECTRODAQ_H
#include <spectrodaq.h>
#define __SPECTRODAQ_H
#endif
                               
#ifndef __DATASOURCE_H
#include "DataSource.h"
#endif

#ifndef __STL_STRING
#include <string>
#define __STL_STRING
#endif

#ifndef __STL_MAP
#include <map>
#define __STL_MAP
#endif                                   

class CNSCLDaqBuffer;
class CNSCLStateChangeBuffer;
class CNSCLScalerBuffer;
class CNSCLEventBuffer;

typedef STD(map)<STD(string), CDataSource*> DataSourceList;
typedef enum {
  RSUnknown,
  RSActive,
  RSPaused,
  RSHalted
} DAQRunState;
                            
class CConsumer  : public DAQROCNode        
{                       
			
  DAQWordBuffer m_DaqBuffer;	// Buffer into which to receive data.
  DAQRunState m_eRunState;	// Current run state
  int m_nRunNumber;		// Current Run number.
  STD(string) m_sTitle;		// Current run title.        
  DataSourceList m_DataSources;	// Current set of data sources.


protected:

public:

   // Constructors and other cannonical operations:

  CConsumer ();
  virtual ~ CConsumer ( );  // Destructor 
private:
  CConsumer (const CConsumer& aCConsumer );
  CConsumer& operator= (const CConsumer& aCConsumer);
  int operator== (const CConsumer& aCConsumer) const;
public:
  int operator() (int argc, char** pargv);

  DAQRunState getRunState();
  int getRunNumber() const
  { return m_nRunNumber;
  }
  STD(string) getTitle() const
  { return m_sTitle;
  }
                       
                       
  // Mutators
protected:

  void setRunState(DAQRunState am_eRunState);
  void setRunNumber (const int am_nRunNumber)
  { m_nRunNumber = am_nRunNumber;
  }
  void setTitle (const STD(string) am_sTitle)
  { m_sTitle = am_sTitle;
  }

public:

  
  Bool_t AddDataSource (const STD(string)& LinkDesignator, int nTag, 
			int nMask=ALLBITS_MASK, int nDelivery=COS_RELIABLE);
  Bool_t DeleteDataSource (const STD(string)& LinkDesignator)    ;

  void EnableDataTaking();
  void DisableDataTaking();

  void CheckForData (struct timeval* pWaitTime=NULL, int nTag=0)    ;
  
protected:
  
  virtual   void OnBuffer (DAQWordBuffer& rBuffer)   ;
  virtual   void OnPhysicsBuffer (CNSCLEventBuffer& rEventBuffer)    ;
  virtual   void OnScalerBuffer (CNSCLScalerBuffer& rScalerBuffer)   ;
  virtual   void OnBeginBuffer (CNSCLStateChangeBuffer& rStateChange)   ;
  virtual   void OnEndBuffer (CNSCLStateChangeBuffer& rStateChange)   ;
  virtual   void OnPauseBuffer (CNSCLStateChangeBuffer& rStateChange)   ;
  virtual   void OnResumeBuffer (CNSCLStateChangeBuffer& rStateChange)   ;
  virtual   void OnOtherBuffer(CNSCLDaqBuffer& rBuffer);

private:

};

#endif

