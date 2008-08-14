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
#include "CScalerCommand.h"
#include <C3820.h>


using std::string;


////////////////////////////////////////////////////////////////////////
///////////////////////////////// Canonicals ///////////////////////////
////////////////////////////////////////////////////////////////////////

/*!
   Construct the command.
   \param interp : CTCLInterpreter&
      Reference to the interpreter on which the command will be registered
   \param config : CConfiguration&
       Configuration we will be maintaining.
*/
CScalerCommand::  CScalerCommand(CTCLInterpreter& interp, CConfiguration& config) :
  CModuleCommand(interp, config, string("sis3820"))
{
}
/*!
   The destructor is just needed to provide a virtual chain back to the
   base class destructor.
*/
CScalerCommand::~CScalerCommand()
{
}


/*!
   Create the CSIS3820 object for the base class:
*/
CConfigurableObject*
CScalerCommand::createObject()
{
  return new C3820();
}

/*!
  Return the type of the object
*/
string
CScalerCommand::getType()
{
  return string("sis3820");
}
