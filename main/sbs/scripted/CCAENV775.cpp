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

static const char* Copyright = "(C) Copyright Ron Fox 2002, All rights reserved";
////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "CCAENV775.h"    
#include "CIntConfigParam.h"
#include "CIntArrayParam.h"
#include "CBoolConfigParam.h"
#include "CAENcard.h"
#ifdef HAVE_STD_NAMESPACE
using namespace std;		// spectrodaq needs it here.
#endif

#include <assert.h>
#include <iostream>

// constants:

static const int CHANNELS(32);
static const int DEFAULTRANGE(500);
static const float RANGELOW(140.0);
static const float RANGEHIGH(1200.0);
static const float RANGERES(256.0);
static const float RANGECHANPERNS(RANGERES/(RANGEHIGH-RANGELOW));

static const int WAITLOOPS(10);

/*!
  Construct a CAEN V775 multievent TDC module.  The underlying card is not
  constructed until initialization time... in order to give
  the user a chance to configure the module first. This is 
  necessary because one of the key confuration parameters is
  the slot. 
     In addition to constructing our base class, it is
  necessary to register our configuration parameters:
  - range       - The Full scale range in ns.
  - commonstart - true if the module should be in common start mode, false for
                  common stop.
  
  \param rName const string& [in]  The name ofthis module.
*/
CCAENV775::CCAENV775 (const std::string& rName, CTCLInterpreter& rInterp)

  : CCAENModule(rName, rInterp)
 
{   
  // Setup our configuration parameters:
  

  AddIntParam(std::string("range"), DEFAULTRANGE);
  AddBoolParam(std::string("commonstart"), true);
  
  // For all the int params and intarray params, we need
  // to set valid ranges.  This  must be done by locating the
  // parameters and calling setRange.  Since we have just
  // inserted all these parameters, for Find not to locate
  // them is fatal and signalled via an assert.
  
  ParameterIterator i;
  CIntConfigParam*  pInt;
  CIntArrayParam*   pArray;
  
  // The full scale range must be  in the range [140,1200]
  
  i = Find("range");
  assert(i != end());
  pInt = (CIntConfigParam*)*i;
  pInt->setRange(140, 1200);
  

} 

/*!
  Destructor, destroys the underlying card.   Note that
  to delete a null pointer is a no-op so we can 
  safely delete the pointer whether it exists or not.
*/
 CCAENV775::~CCAENV775 ( )  //Destructor - Delete dynamic objects
{

}

/*!
  Performs module dependent initialization. 
  - If the module exists, delete it and remake it.
    This allows a module's slot to be reconfigured.
  - The module is configured according to the parameters.
    that have been set.
  Not being able to locate a configuration parameter 
  considered a fatal error since we put those parameters 
  in in the constructor.  This error will be signalled via
  a failed assertion.
*/
void 
CCAENV775::Initialize()   
{ 

  CCAENModule::Initialize();

  // Set the ragne value according to the linear
  // relationship I calculated from the fact that:
  //  0x1e -> 1200ns
  //  0xff -> 140ns
  //  And that since the LSB are inversely proportional to the register value,
  //  The range is linearly proportional:
  //    range = 1341 - register*(1060/225)
  //    so:
  //      register =   - 225/1060(range-1341)

  ParameterIterator  i;
  CIntConfigParam*   pInt;
  
  i = Find(std::string("range"));
  assert(i != end());
  pInt = (CIntConfigParam*) *i;
  int nRange = pInt->getOptionValue();  // ns.
  float rRange   = (float)nRange;
  float Register = 36040/(rRange + 1.3333);
  getCard()->setRange((int)Register);

  // Set the module into common start or stop  mode depending on the mode setting.

  CBoolConfigParam* pBool;
  i = Find(std::string("commonstart"));
  assert(i != end());
  pBool = (CBoolConfigParam*)(*i);
  if(pBool->getOptionValue()) {	// Common start...
    getCard()->commonStart();
  } 
  else {			// Common stop
    getCard()->commonStop();
  }

  
}  

/*!
   This function is called on a per event basis to prepare
  the module to accept the next trigger. For the CAEN 
  modules, no action is required.
*/
void 
CCAENV775::Prepare()  
{ 
  CCAENModule::Prepare();
}  


/*!  

  Called to clear data in the module prior to the
  first readout.

*/
void 
CCAENV775::Clear()  
{ 
  CCAENModule::Clear();
}

