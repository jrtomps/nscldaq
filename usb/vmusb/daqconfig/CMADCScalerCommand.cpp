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
#include <CMADCScalerCommand.h>
#include <CMADCScaler.h>



using std::string;

/////////////////////////////////////////////////////////////////////
// Constructors and other implemented canonicals:

/*!
  Construct the madc command and register it.
  \param interp - The TCL interpreter on which the command is registered.
  \param config - Reference tothe configuration (database of known modules/stacks etc).
  \param commandName - Name of the command to register, defaults to "madc"
*/
CMADCScalerCommand::CMADCScalerCommand(CTCLInterpreter& interp,
			   CConfiguration&  config,
			   std::string      commandName) :
  CModuleCommand(interp, config, commandName)

{}

/*!
   destructor:
*/
CMADCScalerCommand::~CMADCScalerCommand()
{}

/*!
   Create a new scaler module for the base class:
*/
CConfigurableObject* 
CMADCScalerCommand::createObject()
{
  return new CMADCScaler();
}
/*!
  Return the type under which these modules are registered.
*/
string
CMADCScalerCommand::getType()
{
  return string("madcscaler");
}
