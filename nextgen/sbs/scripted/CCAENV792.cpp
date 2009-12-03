/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

static const char* Copyright = "(C) Copyright Michigan State University 1977, All rights reserved";
////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "CCAENV792.h" 
#include "CAENcard.h"
#include "CIntConfigParam.h"
#include "CBoolConfigParam.h"
#include "CIntArrayParam.h"

using namespace std;


#include <spectrodaq.h>
#include <assert.h>
#include <iostream>

static const int CHANNELS(32);
static const int WAITLOOPS(10);
/*!
  Constructs a CAEN V792 readout object.
  The constructor does not actually create a CAENcard
  object.  This is left to the Initialize member.
  Doing this two phase creation allows the user a chance
  to fully set the configuration parameters of the module 
  before creating the module.  This is actually necessary
  sinced the ADC slot is one of the configuration parameters.
  
  The main purpose of this function is to register the
  configuration parsing objects.

  We need to register the Iped parameter and set its range:
*/
CCAENV792::CCAENV792 (const string & rName, CTCLInterpreter& rInterp)
  : CCAENModule(rName, rInterp)
{
  AddIntParam(string("Iped"), 0);
  ParameterIterator i; 
  CIntConfigParam* pInt;

  i = Find(string("Iped"));
  assert(i != end());

  pInt = (CIntConfigParam*)*i;
  pInt->setRange(0,255);		// One byte worth of data.
} 

/*!
   Destroys the module.  We delete CCAENcard object.
  If the card object has not yet been created, it will be
  null and the deletion will be a no-op.
*/
CCAENV792::~CCAENV792 ( )  //Destructor - Delete dynamic objects
{

}


// Functions for class CCAENV792

/*! 
  Called once when data acquisition starts for a run.
This function deletes any existing CAENcard object and
creates a new one.  The card object is then configured
according the the parameters maintained by the configuration
subsystem.

  Any failure to find a configuration parameter is
considered fatal and is signalled via a failed assertion.
*/
void 
CCAENV792::Initialize()  
{
  CCAENModule::Initialize();	// Delegate to the base class.
				// Later if we have card specific
				// parameter we can process them
				// here too.
  ParameterIterator i; 
  CIntConfigParam* pInt;

  i = Find(string("Iped"));	// We know this can't fail.
  pInt = (CIntConfigParam*)*i;
  CAENcard* pCard = getCard();
  pCard->setIped(pInt->getOptionValue());

  

} 

/*! 
  Called on a per event basis to prepare the card to
accept the next trigger.  For the CAEN V792, this is a
no-op.

*/
void 
CCAENV792::Prepare()  
{
  CCAENModule::Prepare();
}  

/*!  Function: 	

  Reads an event from the card.  
\param rBuffer DAQWordBufferPtr& [modified]
    Pointer like object to the target data buffer.
    data will be read from the ADC to the location
  'pointed to' rBuffer.

\return DAQWordBufferptr updated buffer pointer
    after the read.

\note  The module is silently ignored if the card
    has not been instantiated yet.
*/
void
CCAENV792::Read(DAQWordBufferPtr& rBuffer)  
{ 
  CCAENModule::Read(rBuffer);
}  

/*! 
  Called on a per event basis to clear the module after reading
an event. Probably nothing has to be done here, but 
to be safe, we clear any pending data.

*/
void 
CCAENV792::Clear()  
{ 
  CCAENModule::Clear();
}
