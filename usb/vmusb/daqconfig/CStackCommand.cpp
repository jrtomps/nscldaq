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
#include "CStackCommand.h"
#include <CStack.h>

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
       The configuration of STACKs that will be manipulated by this command.
   \param commandName std::string
       Name of the command to register.
*/
CStackCommand::CStackCommand(CTCLInterpreter& interp,
			 CConfiguration&  config,
			 std::string      commandName) : 
  CModuleCommand(interp, config, commandName)

{
}
/*!
   Destructor is a no-op but chains to the base class which unregisters
   etc.
*/
CStackCommand::~CStackCommand()
{
}

/*!
   Create the stack object for the base class:

*/
CConfigurableObject* 
CStackCommand::createObject()
{
  return new CStack();
}
/*!
   Return the object type... 
*/
string
CStackCommand::getType()
{
  return string("stack");
}
