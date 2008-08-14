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
#include "CSIS3804Command.h"
#include <C3804.h>

using std::string;

////////////////////////////////////////////////////////////////////////
/////////////////// Canonicals that are implemented ////////////////////
////////////////////////////////////////////////////////////////////////

/*!
   Construct the command and register it (base constructor does this
   by default.
   \param interp : CTCLInterpreter&
       Tcl interpreter on which the command will be registered.
   \param config : CConfiguration& config
       The configuration of ADCs that will be manipulated by this command.
   \param commandName std::string
       Name of the command to register.
*/
CSIS3804Command::CSIS3804Command(CTCLInterpreter& interp,
				 CConfiguration&  config) :
  CModuleCommand(interp, config, string("sis3804"))
{
}

/*!
  Destructor is a no-op but chains to the base class which unregisters
  etc. etc.
*/
CSIS3804Command::~CSIS3804Command()
{
}


/*!
   Create a CSIS3804 object for the base class.
*/
CConfigurableObject*
CSIS3804Command::createObject()
{
  return new C3804();
}

/*!
  Return the object type.
*/
string
CSIS3804Command::getType()
{
  return string("sis3804");
}
