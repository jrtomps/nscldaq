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


////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "CScalerModule.h"    				
#include <TCLInterpreter.h>

using namespace std;


/*!
   Construct a scaler module.
   All of the work is done by our base class:
   \param rName  const string& [in]:
      A reference to the name that will be
      given to the module.
   \param rInterp CTCLInterpreter& [in]
      A reference to the interpreter that will be used
      to parse the module configuration commands.
*/
CScalerModule::CScalerModule (const string& rName, 
			      CTCLInterpreter& rInterp) :
   CReadableObject(rName, rInterp)
{   
    
} 
CScalerModule::~CScalerModule ( )  //Destructor - Delete dynamic objects
{
}
