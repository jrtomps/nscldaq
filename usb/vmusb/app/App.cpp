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
#include "App.h"

#include <CVMUSB.h>
#include <CVMUSBDeviceDriver.h>
#include <CAcquisitionThread.h>
#include <CVMUSBConfig.h>
#include <CVMUSBControlConfig.h>
#include <DataBuffer.h>
#include <CBufferQueue.h>

#include <usb.h>
#include <iostream>
#include <stdlib.h>

static const uint32_t BufferSize(13*1024);


/*!
   - Select which VM-USB to use.
   - Create a CVMUSB object that communicates with that interface.
   - Create a device driver object.
   - Get the acquisition thread singleton object and bind the device driver to it.

   \param specification - specifies the desired interface.  If blank, the first
          interface enumerated is used.  If not, there must be an interface with a 
	  matching serial string, and that one is used.  If there is not a matching
	  serial number, an error is thrown.
*/

void
App::selectInterface(std::string specification)
{
  std::vector<struct usb_device*> vmUSBs = CVMUSB::enumerate();

  // There may not be any of them!!!

  if (vmUSBs.size() == 0) {
    cerr << "There are no VM-USBs detected on the system!\n";
    exit(EXIT_FAILURE);
  }

  struct usb_device* pDevice = reinterpret_cast<struct usb_device*>(0);

  // Figure out the right usb_device pointer.

  if (specification == std::string("")) {
    pDevice = vmUSBs[0]; 
  }
  else {
    for (int i=0; i < vmUSBs.size(); i++) {
      if (CVMUSB::getSerial(vmUSBs[i]) == specification) {
	pDevice = vmUSBs[i];
	break;
      }
    }
  }
  // pDevice is null if there is no matching device, or a valid USB pointer
  // from which a CVMUSB can be constructed.

  if (!pDevice) {
    std::cerr << "None of the VM-USB devices on the system has the serial number " 
	      << specification << std::endl;
    std::cerr << "Here is a list of the VM-USB serial numbers I found:\n";
    for (int i=0; i < vmUSBs.size(); i++) {
      std::cerr << CVMUSB::getSerial(vmUSBs[i]) << std::endl; 
    }
    exit(EXIT_FAILURE);
  }
  // Create the device driver:

  CVMUSB*              pVMUSB = new CVMUSB(pDevice);
  CVMUSBDeviceDriver*  pDriver= new CVMUSBDeviceDriver(pVMUSB);

  // Create the acq thread and bind the device driver to it:

  CAcquisitionThread* pReadout = CAcquisitionThread::getInstance();
  pReadout->setDriver(pDriver);


}
/*!
   Setup a configuration so that it will contain all of the commands needed
   to process a DAQ configuration file for the VM-USB supported devices.
   \param configuration - reference to the pre-built configuration.
*/
void
App::setupConfiguration(CConfiguration& config)
{
  CVMUSBConfig::configure(&config);
}
/*!
  Set up a Tcl server so that it will be able to process commands in a
  control configuration file.
  \param server - reference to the server.
*/
void
App::setupTclServer(TclServer& pServer)
{
  CVMUSBControlConfig::configure(&pServer);
}

/*!
   Create the buffer pool for the system.
   This is device dependent, because the VM-USB and CC-USB e.g. use different
   sized buffers
*/
void
App::createBuffers()
{
  for (int i =0; i < 32; i++) {
    DataBuffer* pBuffer = createDataBuffer(BufferSize);
    gFreeBuffers.queue(pBuffer);
  }
}
/////////////////////////////////////////////////////////////////////////////

/*!
  The program entry point.  we simply need to crate an App object and
  invoke it's main() entry.
*/
int
main(int argc, char** argv)
{
  App theApp;
  return theApp.main(argc, argv);
}


void* gpTCLApplication(0);
