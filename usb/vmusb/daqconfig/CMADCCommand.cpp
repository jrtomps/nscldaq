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
#include <CMADCCommand.h>
#include <CMADC32.h>


using std::string;


/////////////////////////////////////////////////////////////////////
// Constructors and other implemented canonicals:

/*!
  Construct the madc command and register it.
  \param interp - The TCL interpreter on which the command is registered.
  \param config - Reference tothe configuration (database of known modules/stacks etc).
  \param commandName - Name of the command to register, defaults to "madc"
*/
CMADCCommand::CMADCCommand(CTCLInterpreter& interp,
			   CConfiguration&  config,
			   std::string      commandName) :
  CDAQModuleCommand(interp, config, commandName)
{}

/*!
   destructor:
*/
CMADCCommand::~CMADCCommand()
{}


/*!
   Create an object for the base class's create method:
*/
CConfigurableObject*
CMADCCommand::createObject()
{
  return new CMADC32();
}
/*!
   Return the type under which CMADC objects will be registered:
*/
string
CMADCCommand::getType()
{
  return string("madc");
}
