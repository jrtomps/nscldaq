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
#include "CCAENV830Command.h"
#include <C830.h>


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
CCAENV830Command::CCAENV830Command(CTCLInterpreter& interp, CConfiguration& config,
				   string name) :
  CDAQModuleCommand(interp, config, name)
{
}
/*!
   The destructor is just needed to provide a virtual chain back to the
   base class destructor.
*/
CCAENV830Command::~CCAENV830Command()
{
}


/*!
   Return a new C830 module.
*/
CConfigurableObject*
CCAENV830Command::createObject()
{
  return new C830();
}

/*!
  Return the module type string which is caen830
*/
string
CCAENV830Command::getType()
{
  return string("caen830");
}
