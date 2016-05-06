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

static const char* Copyright = "(C) Ron Fox 2002, All rights reserved";
////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "CCAENV792Creator.h" 
#include "CReadableObject.h"   	
#include "CCAENV792.h"	
#include <assert.h>
#include <string>	

using namespace std;

	
/*!
   Construct an instance of a CAEN V 792 module creator.  The module
   creator can then be registered withy CModuleCommand::AddCreator.
   */
CCAENV792Creator::CCAENV792Creator ()
   : CModuleCreator("caenv792")
{   
} 

/*
   Destroys an instance of a V792 creator.
   */
 CCAENV792Creator::~CCAENV792Creator ( ) 
{
}

/*!
	Creates temporary copy objects.
	   \param rhs const CCAENV792Creator& [in]
		 Reference to copy from.
*/
CCAENV792Creator::CCAENV792Creator (const CCAENV792Creator& rhs ) 
  : CModuleCreator (rhs) 
 
{
} 

/*!
   Assigns a rhs object to *this.
      \param rhs const CCAENV792Creator& [in]
	    Reference to rhs of assignment operator.
      \return reference to *this after the copy in.
*/
CCAENV792Creator& 
CCAENV792Creator::operator= (const CCAENV792Creator& rhs)
{ 
   if (this != &rhs) {
       CModuleCreator::operator=(rhs);
   }
   return *this;
}

      //Operator== Equality Operator 
int CCAENV792Creator::operator== (const CCAENV792Creator& aCCAENV792Creator) const
{ return 
    (CModuleCreator::operator== (aCCAENV792Creator));
}

// Functions for class CCAENV792Creator

/*! 

Returns a new instance of the digitizer module 
creaetd by this creator.  The mdule is new'd into
being and therefore must be deleted by the ultimate
user.  The parameters passed in are also passed
to the module's configuration function.

\param rInterp CTCLInterpreter& [in] Reference to the interpreter running the command.
\param rResult CTCLResult& [in] Referenc to the result string 
	to return.
\param nArgs   int [in]   Number of parameters left on the line.
\param pArgs   char** [in] The parameter strings.

\return CReadableObject*  The resulting module.

The command line invoking is is of the form:
\verbatim
	module mname type config params
\endverbatim
  The caller has only eaten up the module command keyword:
  - mname is used to set the modulen name.
  - type is asserted against our type.
  - The remaining parameters are passed to the instantiated module's 
     configuration member.
*/
CReadableObject* 
CCAENV792Creator::Create(CTCLInterpreter& rInterp, CTCLResult& rResult,
					int nArgs, char** pArgs)  
{ 
	assert(nArgs >= 2);
	string mName(*pArgs);       // Module name.
	nArgs --;
	pArgs++;
	assert(*pArgs == getModuleType());
	nArgs--;
	pArgs++;
	
	CCAENV792* pModule= new CCAENV792(mName, rInterp);
	if(nArgs) {
	  int status = pModule->Configure(rInterp, rResult, nArgs, pArgs);
	  if(status != TCL_OK) { // If configure fails it will fill in rResult
	    delete pModule;	 // but we need to return an error. 
	    pModule = (CCAENV792*)NULL;
	  }
	}
	return pModule;
	
}  

/*!  Function: 	
   string  Help() 
 Operation Type:
    
Purpose: 	

Returns a string describing the module type and
whatever else the module driver author wants to display
about that module type in response to the module -help
command.

*/
string  
CCAENV792Creator::Help()  
{ 
	return string("Creates a CAEN V 792 module.");
}
