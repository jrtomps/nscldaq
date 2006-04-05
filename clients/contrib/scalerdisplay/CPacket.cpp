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


#include <config.h>
#include <TCLResult.h>
#include "CPacket.h"    
#include "CIntConfigParam.h"
#include "CIntArrayParam.h"
#include "CBoolConfigParam.h"
#include <assert.h>
#include <Iostream.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

// constants:

/*!
  Construct a packet module.
  
  \param rName const string& [in]  The name ofthis module.
*/
CPacket::CPacket (const string& rName, CTCLInterpreter& rInterp)

  : CDigitizerModule(rName, rInterp)
 
{   
  // Setup our configuration parameters:
  

  AddIntParam(string("id"), -1);
  AddBoolParam(string("packetize"), false);
  

} 

/*!
  Destructor, destroys the underlying card.   Note that
  to delete a null pointer is a no-op so we can 
  safely delete the pointer whether it exists or not.
*/
 CPacket::~CPacket ( )  //Destructor - Delete dynamic objects
{

}

/*!
   Process commands in addition to config/cget:
*/
int
CPacket::operator()(CTCLInterpreter& rInterp,
		    CTCLResult&      rResult,
		    int nArgs, char** pArgs)
{
  int     argc = nArgs;
  char**  argv = pArgs;

  argc--;
  argv++;

  if(argc) {
    if(string(*argv) == (string("add"))) {
      return TCL_OK;
    }
    else if (string(*argv) == (string ("list"))) {
      return TCL_OK;
    }
    else if (string(*argv) == (string("remove"))) {
      return TCL_OK;
    }
    else {
      return CDigitizerModule::operator()(rInterp, rResult, nArgs, pArgs);
    }
  }
  else {			// Too few params.
    rResult   = "Insufficient command parameters\n";
    rResult  += Usage();
    return TCL_ERROR;
  }
}
