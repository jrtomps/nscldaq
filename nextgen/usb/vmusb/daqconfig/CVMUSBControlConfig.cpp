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
#include "CVMUSBControlConfig.h"
#include <TclServer.h>
#include <CGDG.h>

/*!
   Add prototoypes modules for each supported device type to the
   Tcl Server.
   \param pServer - Pointer to the tcl server object.
*/
void
CVMUSBControlConfig::configure(TclServer* pServer)
{
  pServer->addPrototype(std::string("gdg"), 
			new CGDG("gdg"));
}
