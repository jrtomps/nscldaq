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
#include "CSBSVMEInterface.h"
#include "CSBSVmeAddressRange.h"
#include "CSBSVmeDMATransfer.h"
#include "CSBSPio.h"

#include "CSBSVmeException.h"


#include <iostream>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

/*!
   Construct a VME interface. 
   We will open an sbs device on the specified crate for
   BT_DEV_A32.  Note that various creations will temporarily
   modify the attributes of this unit so that other address
   modifiers can be used.
   \param crate : unsigned short
      VME crate number (interface number) within the SBS interface
      set to open.

    \throw CSBSVmeException in the event of errors.
*/
CSBSVMEInterface::CSBSVMEInterface(unsigned short crate)
{
  // Get the interface name for the open:

  char name[100];
  char* result = bt_gen_name(crate, BT_DEV_A32, name, sizeof(name));
  if (result != name) {
    throw CSBSVmeException(BT_ENXIO,
			   "CSBSVMEInterface::construction getting devname");
  }
  // Now open the device and save the handle in m_handle.

  bt_error_t status = bt_open(&m_handle,
			      name,
			      BT_RDWR);
  if(status != BT_SUCCESS) {
    throw CSBSVmeException(status,
			   "CSBSVMEInterface::construction opening device");
  }

}
/*!
  Destroy a VME interface.  Note that if this is done prior to destroying
  all of the objects created by the interface, those objects may not work
  because their underlying handles have been closed out from underneath them.

  TODO:  Objects could reference count back to this and this case could be detected
         resulting in a throw of some sort.
*/
CSBSVMEInterface::~CSBSVMEInterface()
{
  bt_error_t status = bt_close(m_handle);
  if (status != BT_SUCCESS) {
    throw CSBSVmeException(status,
			   "CSBSVMEInterface::construction closing device");
  }
}
/*!
   canMap  - returns true indicating that this kind of interface can do
   memory maps.
*/
bool
CSBSVMEInterface::canMap() const
{
  return true;
}
/*!
   hasListProcessor - returns false indicating that we don't have any
   intelligence in the remote VME crate.
*/
bool
CSBSVMEInterface::hasListProcessor() const
{
  return false;
}
/*!
   hasDMABlockTransfer - returns true indicating that we have a DME block
   transfer engine.
*/
bool
CSBSVMEInterface::hasDMABlockTransfer() const
{
  return true;
}

/*!
   Called when the VME subsystem has been locked to this thread/process.
   We call bt_lock with no delay as the unit should not be locked by any 
   other users, and throw if we can't.
*/
void
CSBSVMEInterface::onLock()
{
  bt_error_t status = bt_lock(m_handle, BT_NO_WAIT);
  if (status != BT_SUCCESS) {
    throw CSBSVmeException(status,
			   "CSBSVMEInterface::onLock - could not obtain interface lock");
  }
}
/*!
  Called when the VME subsystem has been unlocked.  We call bt_unlock
  to do any unlock of the underlying hardware/driver.
  We throw an error if we can't unlock.
*/
void
CSBSVMEInterface::onUnlock()
{
  bt_error_t status = bt_unlock(m_handle);
  if (status != BT_SUCCESS) {
    throw CSBSVmeException(status,
			   "CSBSVMEInterface::onUnlock - could not unlock");
  }
}

/*!
  Return the device type:
  "SBSBit3PCIVME"
*/
string
CSBSVMEInterface::deviceType() const
{
  return string("SBSBit3PCIVME");
}
/*!
   Return the device handle cast to void... probably really not const?
*/
void*
CSBSVMEInterface::getDeviceHandle() const
{
  return static_cast<void*>(m_handle);
}

/*!  
    Create a vme address range and return it to the caller.
    \param addressModifier : unsigned short
       Address modifier used when accessing this range.
    \param baseAddress : unsigned long
       Base addresss of the map.
    \param bytes  : size_t
       Number of bytes in the map.

     \return CVMEAddressRange*
     \retval Pointer to what is actually a CSBSVmeAddressRange object.
             the user must destroy this to unmap the range.

*/
CVMEAddressRange*
CSBSVMEInterface::createAddressRange(unsigned short addressModifier,
				     unsigned long  baseAddress,
				     size_t         bytes)
{
  return dynamic_cast<CVMEAddressRange*>(new CSBSVmeAddressRange(m_handle,
								 addressModifier,
								 baseAddress,
								 bytes));
}
/*!
   Create and return a pointer to a 'PIO' device.
*/
CVMEPio*
CSBSVMEInterface::createPioDevice()
{
  return static_cast<CVMEPio*>(new CSBSPio(m_handle));
}
/*!
   Create and return a list pointer.
*/
CVMEList*
CSBSVMEInterface::createList()
{
  return static_cast<CVMEList*>(NULL);
}
/*!
    Create and return a dma transfer object.
*/
CVmeDMATransfer* 
CSBSVMEInterface::createDMATransfer(unsigned short addressModifier,
				    CVMEInterface::TransferWidth  width,
				    unsigned long  base,
				    size_t         units)
{
  return static_cast<CVmeDMATransfer*>(new CSBSVmeDMATransfer(m_handle,
							      addressModifier,
							      width, base, units));
}
/*!
   Get the part number of the local card of the system.  This is
   the card in the PCI slot.
*/
bt_devdata_t
CSBSVMEInterface::getLocalPartNumber()
{
  bt_devdata_t partNumber;
  bt_error_t   status = bt_get_info(m_handle, BT_INFO_LOC_PN, &partNumber);
  checkStatus(status, "CSBSVMEInterface::getLocalPartNumber bt_getinfo");

  return partNumber;
}
/*!
    Get the part number of the remote card of the system.  This is
    the VME side.
*/
bt_devdata_t
CSBSVMEInterface::getRemotePartNumber()
{
  bt_devdata_t partNumber;
  bt_error_t   status = bt_get_info(m_handle, BT_INFO_REM_PN, &partNumber);
  checkStatus(status, "CSBSVMEInterface::getRemotePartNumber bt_getinfo");

  return partNumber;
}
/*!
   The VME side of the interface can have local memory.  
   This function returns the number of bytes of local memory.
   Note that this size can be 0.
*/
bt_devdata_t 
CSBSVMEInterface::getMemorySize()
{
  bt_devdata_t size;
  bt_error_t   status = bt_get_info(m_handle, BT_INFO_LM_SIZE, &size);
  checkStatus(status,   "CSBSVMEInterface::getMemorySize bt_getinfo");

  return size;
}

/*!
     Set the device driver trace flags.  The trace flags are defined
     in btngpci.h and have prefixes like BT_TRC_
     They govern how much information is sent by the driver to the console
     and the system logging facility.
     \param traceFlags : bt_devdata_t
        The trace flags.
*/
void
CSBSVMEInterface::setDriverTraces(bt_devdata_t traceFlags)
{
  bt_error_t status = bt_set_info(m_handle, BT_INFO_TRACE, traceFlags);

  checkStatus(status, "CSBSVMEInterface::setDriverTraces bt_set_info");
}
/*!
   Get the driver trace flags. See above for information about the
   driver trace flags.
*/
bt_devdata_t
CSBSVMEInterface::getDriverTraces()
{
  bt_devdata_t traceFlags;
  bt_error_t status = bt_get_info(m_handle, BT_INFO_TRACE, &traceFlags);
  return traceFlags;
}

/*!
   Reset the VME bus.  This cannot be tested since there's no
   programmtically, for sure thing to look at.
*/
void
CSBSVMEInterface::reset()
{
  bt_reset(m_handle);
}
/*!
   Return the last adapter error, this must be cleared using
   clearErfror().
   This cannot be tested since I don't have a good way to force errors.
*/
bt_error_t
CSBSVMEInterface::checkError()
{
  return bt_chkerr(m_handle);
}

/*!
    Clear errors on the device.
*/
void
CSBSVMEInterface::clearError()
{
  bt_clrerr(m_handle);
}

/*!
   Return the string version of error text passed in.
*/
string
CSBSVMEInterface::errorText(bt_error_t status)
{
  char error[1000];

  bt_strerror(m_handle, status, "", error, sizeof(error));
  
  return string(error);
	     
}
/////////////////////////////////////////////////////////////////

// Utiltities:
//     


// Check status and throw with message if not successful.

void CSBSVMEInterface::checkStatus(bt_error_t status,
				   const char* failureMessage)
{
  if (status != BT_SUCCESS) {
    throw CSBSVmeException(status,
			   failureMessage);
  }
}
