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
#include "CMarkerCommand.h"
#include <CMarker.h>


using std::string;


//////////////////////////////////////////////////////////////////////////////
///////////////////// Implemented Canonicals /////////////////////////////////
//////////////////////////////////////////////////////////////////////////////



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
CMarkerCommand::CMarkerCommand(CTCLInterpreter&   interp,
			       CConfiguration&    config,
			       std::string        commandName) :
  CDAQModuleCommand(interp,config, commandName)
{}

/*!
   Destructor:
*/
CMarkerCommand::~CMarkerCommand()
{
}


/*!
   Create a new marker object and return it to the base class:
*/
CConfigurableObject*
CMarkerCommand::createObject()
{
  return new CMarker();
}
/*!
   Return the type of module this should be entered into the config with.
*/
string
CMarkerCommand::getType()
{
  return string("marker");
}
