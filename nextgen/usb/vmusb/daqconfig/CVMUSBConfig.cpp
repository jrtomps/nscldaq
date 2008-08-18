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
#include "CVMUSBConfig.h"
#include <CConfiguration.h>
#include <Globals.h>
#include <CADCCommand.h>
#include <CCAENChainCommand.h>
#include <CCAENV830Command.h>
#include <CMADCCommand.h>
#include <CMADCScalerCommand.h>
#include <CMarkerCommand.h>
#include <CNADC2530Command.h>
#include <CScalerCommand.h>
#include <CSIS3804Command.h>
#include <CStackCommand.h>

/*!
   Create a configuration object and stock it with all the commands needed to support the
   the VM-USB accessible modules.  
   To add new hardware support you must:
   - Add a device support module in the../deviceDriver directory.
   - Add a device command in this directory.
   - Modify this function to create an instance of your device command class and
     register that in the configuration object created here.

  \return CConfiguration*
  \retval A pointer to the configuration created.

  \note The configuration pointer is also stored in Globals::pConfig so that other
        elements of the program can find it.

   \note If the configuration object exists it will be destroyed first.  This ensures
         that a call to CVMUSBConfig::Create will create a brand new configuration
	 object.
*/
CConfiguration*
CVMUSBConfig::create()
{
  if (exists()) {
    destroy();
  }

 
  CConfiguration* p = new CConfiguration();
  Globals::pConfig  = p;

  configure(p);
  return p;
}

/*!
  Load a configuration with commands needed to read a VMUSB configuration file.
  \param pConfig - Pointer to tyhe configuration to uh...configure.

*/
void
CVMUSBConfig::configure(CConfiguration* pConfig)
{
  CConfiguration* p = pConfig;

  //  This section stocks the configuration with commands.
  //  Add code here to add new commands.
  //

  CTCLInterpreter& interp(*(p->getInterpreter()));

  p->addCommand(*(new CADCCommand(interp, *p)));        // adc command.
  p->addCommand(*(new CCAENChainCommand(interp, *p)));  // caenchain command.
  p->addCommand(*(new CCAENV830Command(interp, *p)));   // v830 command.
  p->addCommand(*(new CMADCCommand(interp, *p)));       // madc command.
  p->addCommand(*(new CMADCScalerCommand(interp, *p))); // madcscaler command.
  p->addCommand(*(new CMarkerCommand(interp, *p)));     // marker command.
  p->addCommand(*(new CNADC2530Command(interp, *p)));   // hytec command.
  p->addCommand(*(new CScalerCommand(interp, *p)));     // sis3820 command.
  p->addCommand(*(new CSIS3804Command(interp, *p)));    // sis3804 command.
  p->addCommand(*(new CStackCommand(interp, *p)));      // stack command



 
} 
/*!
    Destroy any existing configuration.  The configuration object is responsible
    for destroying its command objects.
*/
void
CVMUSBConfig::destroy()
{
  if (exists()) {
    delete Globals::pConfig;
    Globals::pConfig = reinterpret_cast<CConfiguration*>(0);
  }
}


/*!
   \return bool
   \retval true  - A configuration exsists.
   \retval false - No configuration exists.
*/
bool
CVMUSBConfig::exists()
{
  return (Globals::pConfig != reinterpret_cast<CConfiguration*>(0));
}
