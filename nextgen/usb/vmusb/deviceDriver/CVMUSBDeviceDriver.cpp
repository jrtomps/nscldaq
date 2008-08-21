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
#include "CVMUSBDeviceDriver.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"
#include "CStack.h"
#include <CConfiguration.h>
#include <DataBuffer.h>
#include <Globals.h>
#include <stdint.h>
#include <unistd.h>

using namespace std;

static const size_t VMUSBBufferSize = 13*1024*sizeof(uint16_t);
static const int    DRAINTIMEOUT    = 5*1000;
static const int    FLUSHTIMEOUT    = 1*1000;
/*!
   Construct the object
   \param pInterface - Pointer to a vm-usb interface object.
*/
CVMUSBDeviceDriver::CVMUSBDeviceDriver(CVMUSB* pDevice) :
  m_pInterface(pDevice)
{}

/*!
   Return the device, for any other chunks of external code that
   may need tofiddle/diddle the VM-USB:
   \return CVMUSB*
   \retval VMUSB device pointer we use internally.
*/
CVMUSB*
CVMUSBDeviceDriver::getInterface() 
{
  return m_pInterface;
}

/*!
   Start the VM-USB autonomosly interacting with triggers
*/
void
CVMUSBDeviceDriver::usbToAutonomous()
{
  m_pInterface->writeActionRegister(CVMUSB::ActionRegister::startDAQ);
}

/*!
  Read a block of data from the VM-USB most often done when the
  VM-USB is in autonomous data taking mode but could be done any time
  the device has a chunk of data for us.
  \param pBuffer      - Pointer to a buffer into which the read data are transferred.
  \param bytesToRead  - Maximum number of bytes that can be read into pBuffer.
  \param bytesRead    - Pointer to where to write the number of bytes that were actually read
                       (could be fewer than bytesToRead).
  \param timeoutInSeconds - Number of seconds maximum to wait for the read to complete.
  \return int
  \retval 0 - Data were read, the buffer is fileld, and bytesRead is the number actually read.
  \retval -1 Read failed and errno has the reason for it.  Note that ETIMEDOUT indicates a timeout.

*/
int
CVMUSBDeviceDriver::usbReadBuffer(void*   pBuffer,
				  size_t  bytesToRead,
				  size_t* bytesRead,
				  int     timeoutInSeconds)
{
  return m_pInterface->usbRead(pBuffer, 
			       bytesToRead,
			       bytesRead,
			       timeoutInSeconds*1000);
}

/*!
   Set up for data taking.  This fetches all of the stack from the
   VM-USB, iterates through them, initializing the modules in them,
   and loading htem into the VM-USB.


   \note Implicit input is our specific version of the configuration.


*/
void
CVMUSBDeviceDriver::usbSetup()
{

  // First a final special flush of the USB FIFOS:

  usbReadTrash(FLUSHTIMEOUT);

  // Rest the device:

  m_pInterface->writeActionRegister(CVMUSB::ActionRegister::sysReset);
  m_pInterface->writeActionRegister(0);
  CStack::resetStackOffset();	// Start loads at location 0 on stack.

  // Find and load the stacks.  Stacks are objects in the configuration
  // of type uhh....stack.
  //
  CConfiguration* pC = Globals::pConfig;


  // Find the stacks that have been defined, and for each non zero length stack, 
  // initialize the modules it uses and load/enable it:

  vector<CConfiguration::ConfigItem> stacks  = pC->getObjectsOfType("stack");
  for (int  i = 0; i < stacks.size(); i++) {
    CStack* pStack = reinterpret_cast<CStack*>(stacks[i].s_pObject);
    pStack->Initialize(*m_pInterface);
    pStack->loadStack(*m_pInterface);
    pStack->enableStack(*m_pInterface);
  }
 // Set up the buffer size and mode:

  m_pInterface->writeBulkXferSetup(0 << CVMUSB::TransferSetupRegister::timeoutShift); // don't want multibuffering...1sec timeout is fine.


  // The global mode:
  //   13k buffer
  //   Single event seperator.
  //   Aligned on 16 bits.
  //   Single header word.
  //   Bus request level 4.
  //   Flush scalers on a single event.
  //
  m_pInterface->writeGlobalMode((4 << CVMUSB::GlobalModeRegister::busReqLevelShift) | 
			  CVMUSB::GlobalModeRegister::flushScalers            |
			  (CVMUSB::GlobalModeRegister::bufferLen13K << 
			   CVMUSB::GlobalModeRegister::bufferLenShift));


  // ensure that O1 is the Busy, and for the heck of it, let's make O2 AS.

  m_pInterface->writeDeviceSource(CVMUSB::DeviceSourceRegister::nimO1Busy |
			    CVMUSB::DeviceSourceRegister::nimO2VMEAS);

}
/*!
   \return size_t
   \retval 13Kwords*sizeof(uint16_t), the size of the VM-USB data buffer.
*/
size_t
CVMUSBDeviceDriver::usbGetBufferSize()
{
  return VMUSBBufferSize;
}

/*!
  Tell the VM-USB to stop responding to triggers, and go back to a mode suitable for
  one-shot operations.

*/
void
CVMUSBDeviceDriver::usbStopAutonomous()
{
  m_pInterface->writeActionRegister(CVMUSB::ActionRegister::scalerDump);

  
}
/*!
   Returns true if a buffer is the last one from a VM-USB.
   This is used by the caller to determine if a buffer completed
   the flush of the VM-USB at the end of data taking.
   \param pBuffer - Pointer to the data buffer to check.
   \return bool
   \retval true - buffer had the last flag set.
   \retval false - buffer did not have the last flag set.
*/
bool
CVMUSBDeviceDriver::isusbLastBuffer(DataBuffer* pBuffer)
{
  return pBuffer->s_rawData[0] & CVMUSB::LastBuffer;
}

/*!
    Some times at the end of a run, the VM-USB does not ever deliver a buffer
    with a last buffer flag set.  In those cases, it is necessary
    to reset the Vm-USB once more stop the module and
    do a dummy write to32 bit read and then flush any fifo data.
    This, I not so affectionately refer to a kicking the VM-USB in the groin,
    hence the member function name.

    Perhaps not all USB devices have this issue and therefore some could implement this as
    a no-op.
*/
void
CVMUSBDeviceDriver::usbGroinKick()
{
  // Reset and stop the VM-USB:

  m_pInterface->writeActionRegister(CVMUSB::ActionRegister::sysReset);
  m_pInterface->writeActionRegister(0);

  // wait a bit and then do a dummy read and final buffer flush:

  usleep(100);
  uint32_t junk;
  m_pInterface->vmeRead32(0, CVMUSBReadoutList::a32UserData, &junk);
  usbReadTrash(DRAINTIMEOUT);
}

/*!
   Flush any garbage that happens to be in the USB FIFO.

*/
void
CVMUSBDeviceDriver::usbFlushGarbage()
{
  usbReadTrash(FLUSHTIMEOUT);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Private utilities:
//

/*
   Read and throw away data from the VM-USB with the given read timeout.
*/
void
CVMUSBDeviceDriver::usbReadTrash(int milliseconds)
{
  uint8_t buffer[VMUSBBufferSize];
  size_t  junk;
  m_pInterface->usbRead(buffer, sizeof(buffer), &junk, milliseconds);

}
