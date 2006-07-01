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

// Implementation of the CVMUSB class.

#include "CVMUSB.h"
#include <usb.h>
#include <errno.h>
#include <string.h>

using namespace std;

// Constants:

// Identifying marks for the VM-usb:

static const short USB_WIENER_VENDOR_ID(0x16dc);
static const short USB_VMUSB_PRODUCT_ID(0xb);

// Bulk transfer endpoints

static const int ENDPOINT_OUT(2);
static const int ENDPOINT_IN(0x86);

// Timeouts:

static const int DEFAULT_TIMEOUT(2000);	// ms.

/////////////////////////////////////////////////////////////////////
/*!
  Enumerate the Wiener/JTec VM-USB devices.
  This function returns a vector of usb_device descriptor pointers
  for each Wiener/JTec device on the bus.  The assumption is that
  some other luck function has initialized the libusb.
  It is perfectly ok for there to be no VM-USB device on the USB 
  subsystem.  In that case an empty vector is returned.
*/
vector<usb_device*>
CVMUSB::enumerate()
{
    usb_find_busses();		// re-enumerate the busses
    usb_find_devices();		// re-enumerate the devices.

    // Now we are ready to start the search:

    vector<usb_device*> devices;	// Result vector.
    usb_bus* pBus = usb_get_busses();

    while(pBus) {
	usb_device* pDevice = pBus->devices;
	while(pDevice) {
	    usb_device_descriptor* pDesc = &(pDevice->descriptor);
	    if ((pDesc->idVendor  == USB_WIENER_VENDOR_ID)    &&
		(pDesc->idProduct == USB_WIENER_PRODUCT_ID)) {
		devices.push_back(pDevice);
	    }

	    pDevice = pDevice->next;
	}

	pBus = pBus->next;
    }

    return devices;
}
////////////////////////////////////////////////////////////////////
/*!
  Construct the CVMUSB object.  This involves storing the
  device descriptor we are given, opening the device and
  claiming it.  Any errors are signalled via const char* exceptions.
  \param vmUsbDevice   : usb_device*
      Pointer to a USB device descriptor that we want to open.

  \bug
      At this point we take the caller's word that this is a VM-USB.
      Future implementations should verify the vendor and product
      id in the device structure, throwing an appropriate exception
      if there is aproblem.

*/
CVMUSB::CVMUSB(usb_device* device) :
    m_handle(0),
    m_device(device),
    m_timeout(DEFAULT_TIMEOUT)
{
    m_handle  = usb_open(m_device);
    if (!m_handle) {
	throw "CVMUSB::CVMUSB  - unable to open the device";
    }
    // Now claim the interface.. again this could in theory fail.. but.

    int status = usb_claim_interface(m_handle, 0);
    if (status == -EBUSY) {
	throw "CVMUSB::CVMUSB - some other process has already claimed";
    }
    if (status == -ENOMEM) {
	throw "CVMUSB::CMVUSB - claim failed for lack of memory";
    }
}
////////////////////////////////////////////////////////////////
/*!
    Destruction of the interface involves releasing the claimed
    interface, followed by closing the device.
*/
CVMUSB::~CVMUSB()
{
    usb_release_interface(m_handle, 0);
    usb_close(m_handle);
}
     
*/
////////////////////////////////////////////////////////////////////
/*!
    Writing a value to the action register.  This is really the only
    special case for this code.  The action register is the only
    item that cannot be handled by creating a local list and
    then executing it immediately.
    Action register I/O requires a special list, see section 4.2, 4.3
    of the Wiener VM-USB manual for more information
    \param value : uint16_t
       The register value to write.
*/
void
CVMUSB::writeActionRegister(uint16_t value)
{
    char outPacket[100];


    // Build up the output packet:

    char* pOut = outPacket;
    
    pOut = addToPacket16(pOut, 5); // Select Register block for transfer.
    pOut = addToPacket16(pOut, 10); // Select action register wthin block.
    pOut = addToPacket16(pOut, value);

    // This operation is write only.

    int outSize = pOut - outPacket;
    int status = usb_bulk_write(m_handle, ENDPOINT_OUT, 
				outPacket, outSize, m_timeout);
    if (status < 0) {
	string message = "Error in usb_bulk_write, writing action register ";
	message == strerror(-status);
	throw message.c_str();
    }
    if (status != outSize) {
	throw "usb_bulk_write wrote different size than expected";
    }
}
////////////////////////////////////////////////////////////////////////
/*!
   Read the action register.  On errors a const char* exception is thrown.
   The action register is the only register that demands special treatment
   as it is only accessible via a usb register read operation.  All other
   registers can be read by building a local list and executing it
   immediately.
*/
uint16_t
CVMUSB::readActionRegister()
{
    char outPacket[100];
    char inPacket[100];
    
    // Build up the request packet:

    char* pPacket = static_cast<char*>(outPacket);
    pPacket = addToPacket16(pPacket, 1); // Read op on registerblock.
    pPacket = addToPacket16(pPacket, 10); // Action register address.
    int size= (pPacket - outPacket);

    int status = transaction(outPacket, size,
			     inPacket, sizeof(inPacket));
    if (status == -1) {
	char*   reason = strerror(errno);
	string  message= "CVMUSB::readActionRegister write failed - ";
	message += reason;
	throw message.c_str();

    }
    if (status == -2) {
	char*  reason = strerror(errno);
	string message= "CVMUSB::readActionRegister read failed - ";
	message += reason;
	throw message.c_str();
    }

    uint16_t registerValue;
    getFromPacket16(inPacket, &registerValue);
    return registerValue;
    

	
}


////////////////////////////////////////////////////////////////////////
/*
   Utility function to perform a 'symmetric' transaction.
   Most operations on the VM-USB are 'symmetric' USB operations.
   This means that a usb_bulk_write will be done followed by a
   usb_bulk_read to return the results/status of the operation requested
   by the write.
   Parametrers:
   void*   writePacket   - Pointer to the packet to write.
   size_t  writeSize     - Number of bytes to write from writePacket.
   
   void*   readPacket    - Pointer to storage for the read.
   size_t  readSize      - Number of bytes to attempt to read.

   Returns:
     > 0 the actual number of bytes read into the readPacket...
         and all should be considered to have gone well.
     -1  The write failed with the reason in errno.
     -2  The read failed with the reason in errno.

   NOTE:  The m_timeout is used for both write and read timeouts.

*/
int
CVMUSB::transaction(void* writePacket, size_t writeSize,
		   void* readPacket,  size_t readSize)
{
    int status = usb_bulk_write(m_handle, ENDPOINT_OUT,
				writePacket, writeSize, m_timeout);
    if (status < 0) {
	errno = -status;
	return -1;		// Write failed!!
    }
    status    = usb_bulk_read(m_handle, ENDPOINT_IN,
			      readPacket, readSize, m_timeout);
    if (status < 0) {
	errno = -status;
	return -2;
    }
    return status;
}


////////////////////////////////////////////////////////////////////////
/*
   Build up a packet by adding a 16 bit word to it;
   the datum is packed low endianly into the packet.

*/
void*
CVMUSB::addToPacket16(void* packet, uint16_t datum)
{
    char* pPacket = static_cast<char*>(packet);

    *pPacket++ = (datum  & 0xff); // Low byte first...
    *pPacket++ = (datum >> 8) & 0xff; // then high byte.

    return static_cast<void*>(pPacket);
}
/////////////////////////////////////////////////////////////////////////
/*
  Build up a packet by adding a 32 bit datum to it.
  The datum is added low-endianly to the packet.
*/
void*
CVMUSB::addToPacket32(void* packet, unit32_t datum)
{
    char* pPacket = static_cast<char*>(packet);

    *pPacket++    = (datum & 0xff);
    *pPacket++    = (datum >> 8) & 0xff;
    *pPacket++    = (datum >> 16) & 0xff;
    *pPacket++    = (datum >> 24) & 0xff;

    return static_cast<void*>(pPacket);
}
/////////////////////////////////////////////////////////////////////
/* 
    Retrieve a 16 bit value from a packet; packet is little endian
    by usb definition. datum will be retrieved in host byte order.
*/
void*
CVMUSB::getFromPacket16(void* packet, uint16_t* datum)
{
    char* pPacket = static_cast<char*>(packet);

    uint16_t low = *pPacket++;
    uint16_t high= *pPacket++;

    *datum =  (low | (high << 8));

    return static_cast<void*>(pPacket);
	
}
/*!
   Same as above but a 32 bit item is returned.
*/
void*
CVMUSB::getFromPacket32(void* packet, uint32_t* datum)
{
    char* pPacket = static_cat<char*>(packet);

    uint32_t b0  = *pPacket++;
    uint32_t b1  = *pPacket++;
    uint32_t b2  = *pPacket++;
    uint32_t b3  = *pPacket++;

    *datum = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);


    return static_cast<void*>(pPacket);
}
