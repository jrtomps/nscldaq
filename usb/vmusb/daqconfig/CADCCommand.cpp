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
#include "CADCCommand.h"
#include <C785.h>

using std::string;

/*!
  Construction  just delegates to the base class:
*/ 
CADCCommand::CADCCommand(CTCLInterpreter& interp,
			 CConfiguration&  config,
			 std::string      commandName) :
  CModuleCommand(interp, config, commandName)
{}

/*!
   Chain to the caller.
*/
CADCCommand::~CADCCommand()
{
}

/*!
    The object we need to create is a C785 object
*/

CConfigurableObject*
CADCCommand::createObject()
{
  return new C785();
}

/*!
   The object type is  "caen32"
*/
string
CADCCommand::getType()
{
  return string("caen32");
}
