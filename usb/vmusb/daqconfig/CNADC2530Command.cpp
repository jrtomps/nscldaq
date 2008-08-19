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
#include "CNADC2530Command.h"
#include <CNADC2530.h>

using std::string;


////////////////////////////////////////////////////////////////////////////////////
//   Constructors and implemented canonical functions


/*!
   Construct the command and register it.  The base constructor does this by default.
   \param interp   : CTCLInterpreter& 
       Tcl interpreter on which the command is registered.
   \param config   : CConfiguration&
       The configuration of ADC's etc. that are manipulated by this command.
   \param commandName : std::string
       Name of the command to register.  This defaults to "hytec"

*/
CNADC2530Command::CNADC2530Command(CTCLInterpreter&   interp,
				   CConfiguration&    config,
				   std::string        commandName) :
  CDAQModuleCommand(interp, config, commandName)
{
}
/*!  
   The constructor only exists to maintain a chain of destructors back to the ultimate base class.
*/
CNADC2530Command::~CNADC2530Command()
{
}


/*!
  Return a new CNADC2530 object to the base class.
*/
CConfigurableObject*
CNADC2530Command::createObject()
{
  return new CNADC2530();
}

/*!
  Return the device type of the module 
*/
string
CNADC2530Command::getType()
{
  return string("hytec2530");
}
