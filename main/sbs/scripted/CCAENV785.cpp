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
#include "CCAENV785.h" 
#include "CAENcard.h"
#include "CIntConfigParam.h"
#include "CBoolConfigParam.h"
#include "CIntArrayParam.h"

using namespace std;

#include <assert.h>
#include <iostream>

/*!
  Constructs a CAEN V785 readout object.
  The constructor does not actually create a CAENcard
  object.  This is left to the Initialize member.
  Doing this two phase creation allows the user a chance
  to fully set the configuration parameters of the module 
  before creating the module.  This is actually necessary
  sinced the ADC slot is one of the configuration parameters.
  
  The main purpose of this function is to register the
  configuration parsing objects.
*/
CCAENV785::CCAENV785 (const string & rName, CTCLInterpreter& rInterp, int nChannels)
  : CCAENModule(rName, rInterp, nChannels)
{

} 

/*!
   Destroys the module.  We delete CCAENcard object.
  If the card object has not yet been created, it will be
  null and the deletion will be a no-op.
*/
CCAENV785::~CCAENV785 ( )  //Destructor - Delete dynamic objects
{

}


// Functions for class CCAENV785

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
CCAENV785::Initialize()  
{
  CCAENModule::Initialize();	// Delegate to the base class.
				// Later if we have card specific
				// parameter we can process them
				// here too.

} 

/*! 
  Called on a per event basis to prepare the card to
accept the next trigger.  For the CAEN V785, this is a
no-op.

*/
void 
CCAENV785::Prepare()  
{
  CCAENModule::Prepare();
}  


/*! 
  Called on a per event basis to clear the module after reading
an event. Probably nothing has to be done here, but 
to be safe, we clear any pending data.

*/
void 
CCAENV785::Clear()  
{ 
  CCAENModule::Clear();
}
