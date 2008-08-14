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
#include "CCAENChainCommand.h"
#include <CCAENChain.h>

using namespace std;


/*!
   Construct the command.. we're going to delegate everything to the
   base class constructor.
   \param interp  CTCLInterpreter&  
          Reference to the interpreter that will run this command.
   \param config  CConfiguration&
          Reference to the configuration that we will populate.
   \param commandName std::string
          Name of the command to register.
      
*/
CCAENChainCommand::CCAENChainCommand(CTCLInterpreter& interp,
				     CConfiguration&  config,
				     string           commandName) :
  CModuleCommand(interp, config, commandName)
{}

/*!
   There is nothing for us to do for destruction.  Let the base classes
   deal with that.
*/
CCAENChainCommand::~CCAENChainCommand()
{}


/*! 
  Create the chain object for the base class:
*/
CConfigurableObject*
CCAENChainCommand::createObject()
{
  return new CCAENChain();
}
/*!
   Return the module type: "caenchain"
*/
string
CCAENChainCommand::getType()
{
  return string("caenchain");
}
