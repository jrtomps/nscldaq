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
static const char* Copyright = "(C) Copyright Michigan State University 1977, All rights reserved";
/*! \class CCAENV785Creator   
	Implementation of CCAENV785Creator.  See the .h file for more information.
*/

////////////////////////// FILE_NAME.cpp ////////////////////////////////////////
#include <config.h>
#include "CCAENV785Creator.h"   
#include "CReadableObject.h"
#include "CCAENV785.h"


#include <assert.h>

using namespace std;

 				
/*!
    Constructor: Creates a module creator for CAEN v785 modules.  We just need
    to do base class construction, specifying our keyword as "caenv785"
    
*/
CCAENV785Creator::CCAENV785Creator (const char* recognize) :
	CModuleCreator(recognize)
{   
    
} 

/*!
	Destructor: Nothing needs ot be done for this class.
*/
 CCAENV785Creator::~CCAENV785Creator ( )  
{
}
/*!
   Copy constructor used to create temporaries, e.g. for pass by value parameters.
   \param rhs const CCAEN785Creator& [in] 
	   reference to the object to copy construct into this.
*/   
CCAENV785Creator::CCAENV785Creator (const CCAENV785Creator& rhs) 
  : CModuleCreator(rhs) 
{
  
} 

 /*!
   Assignment operator.  This member handles a = b operations where a is this and b is 
   another  CCAENV785Creator object.
   \param rhs  const CCAENV785creator& [in]
	   the right hand object of the assignment.
   \return Reference to this.
 */
CCAENV785Creator& 
CCAENV785Creator::operator= (const CCAENV785Creator& rhs)
{ 

   if(this != &rhs) {
      CModuleCreator::operator=(rhs);
   }
   return *this;
}

 
/*!
   Equality comparison of this to rhs.
   \param rhs const CCAENV785Creator& [in]  The item to compare this to.
   \return int:
      - 0  If this is not equal to rhs.
      - nonzero If this is equal to rhs.
      
*/
int 
CCAENV785Creator::operator== (const CCAENV785Creator& rhs) const
{ 
   return (CModuleCreator::operator==(rhs));
}
/*! 
   Inequality comparison.
   Equality comparison of this to rhs.
   \param rhs const CCAENV785Creator& [in]  The item to compare this to.
   \return int:
      - 0  If this is  equal to rhs.
      - nonzero If this is not equal to rhs.
      
*/
int
CCAENV785Creator::operator!=(const CCAENV785Creator& rhs) const
{
  return !(operator==(rhs));
}

// Functions for class CCAENV785Creator

/*!

Returns a new instance of the digitizer module 
creaetd by this creator.  The mdule is new'd into
being and therefore must be deleted by the ultimate
user.  The parameters passed in are also passed
to the module's configuration function.

\param rinterp CTCLInterpreter& [in] 
	 The interpreter on which the creation command is being run.
\param rResult CTCLResult& [in]
	 The result string in which any error message will be returned.
\param nArgs  int [in] The number of parameters remaining on the command line.
\param pArgs  char** [in] The parameters themselves (text strings).

\note
The form of the command is :
\verbatim
module name caenv785 ?cfgopt? ...
\endverbatim
The caller has eaten up the module command, but left us the rest.  There should therefore be at least 2 parameters (all configuration parameters are optional).

\return CReadableObject*   A pointer to the newly created module.  Note that if a
   failure occurs, the function will either assert, or return a null pointer depending on
   the error.

*/
CReadableObject* 
CCAENV785Creator::Create(CTCLInterpreter& rInterp, 
				     CTCLResult& rResult, 
				     int nArgs, char** pArgs)  
{ 
   assert(nArgs >= 2);     // need name and type at least.
   
   CReadableObject* pModule = new CCAENV785(string(*pArgs), rInterp);
   nArgs -= 2;
   pArgs += 2;
   
   // If necessary, configure the module:
   
   if(nArgs) {
     int status = pModule->Configure(rInterp, rResult, 
				     nArgs, pArgs); 
     if(status != TCL_OK) {	// When configure fails,
       delete pModule;		// It fills in the result string.
       pModule= (CCAENV785*)NULL;
     }
   }
				 
   return pModule;
}  

/*!

Returns a string describing the module type and
whatever else the module driver author wants to display
about that module type in response to the module -help
command.

*/
string  
CCAENV785Creator::Help()  
{ 
   return string("Creates a CAEN V785 module.");
}
