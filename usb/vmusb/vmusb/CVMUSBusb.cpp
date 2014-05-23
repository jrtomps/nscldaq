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

#include "CVMUSBusb.h"
#include "CVMUSBReadoutList.h"
#include <os.h>

#include <usb.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <stdio.h>

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

// The register offsets:

static const unsigned int FIDRegister(0);       // Firmware id.
static const unsigned int GMODERegister(4);     // Global mode register.
static const unsigned int DAQSetRegister(8);    // DAQ settings register.
static const unsigned int LEDSrcRegister(0xc);	// LED source register.
static const unsigned int DEVSrcRegister(0x10);	// Device source register.
static const unsigned int DGGARegister(0x14);   // GDD A settings.
static const unsigned int DGGBRegister(0x18);   // GDD B settings.
static const unsigned int ScalerA(0x1c);        // Scaler A counter.
static const unsigned int ScalerB(0x20);        // Scaler B data.
static const unsigned int ExtractMask(0x24);    // CountExtract mask.
static const unsigned int ISV12(0x28);          // Interrupt 1/2 dispatch.
static const unsigned int ISV34(0x2c);          // Interrupt 3/4 dispatch.
static const unsigned int ISV56(0x30);          // Interrupt 5/6 dispatch.
static const unsigned int ISV78(0x34);          //  Interrupt 7/8 dispatch.
static const unsigned int DGGExtended(0x38);    // DGG Additional bits.
static const unsigned int USBSetup(0x3c);       // USB Bulk transfer setup. 
static const unsigned int USBVHIGH1(0x40);       // additional bits of some of the interrupt vectors.
static const unsigned int USBVHIGH2(0x44);       // additional bits of the other interrupt vectors.


// Bits in the list target address word:

static const uint16_t TAVcsID0(1); // Bit mask of Stack id bit 0.
static const uint16_t TAVcsSel(2); // Bit mask to select list dnload
static const uint16_t TAVcsWrite(4); // Write bitmask.
static const uint16_t TAVcsIMMED(8); // Target the VCS immediately.
static const uint16_t TAVcsID1(0x10);
static const uint16_t TAVcsID2(0x20);
static const uint16_t TAVcsID12MASK(0x30); // Mask for top 2 id bits
static const uint16_t TAVcsID12SHIFT(4);

//   The following flag determines if enumerate needs to init the libusb:

static bool usbInitialized(false);

/////////////////////////////////////////////////////////////////////
/*!
  Enumerate the Wiener/JTec VM-USB devices.
  This function returns a vector of usb_device descriptor pointers
  for each Wiener/JTec device on the bus.  The assumption is that
  some other luck function has initialized the libusb.
  It is perfectly ok for there to be no VM-USB device on the USB 
  subsystem.  In that case an empty vector is returned.
*/
vector<struct usb_device*>
CVMUSBusb::enumerate()
{
  if(!usbInitialized) {
    usb_init();
    usbInitialized = true;
  }
  usb_find_busses();		// re-enumerate the busses
  usb_find_devices();		// re-enumerate the devices.
  
  // Now we are ready to start the search:
  
  vector<struct usb_device*> devices;	// Result vector.
  struct usb_bus* pBus = usb_get_busses();

  while(pBus) {
    struct usb_device* pDevice = pBus->devices;
    while(pDevice) {
      usb_device_descriptor* pDesc = &(pDevice->descriptor);
      if ((pDesc->idVendor  == USB_WIENER_VENDOR_ID)    &&
	  (pDesc->idProduct == USB_VMUSB_PRODUCT_ID)) {
	devices.push_back(pDevice);
      }
      
      pDevice = pDevice->next;
    }
    
    pBus = pBus->next;
  }
  
  return devices;
}

/**
 * Return the serial number of a usb device.  This involves:
 * - Opening the device.
 * - Doing a simple string fetch on the SerialString
 * - closing the device.
 * - Converting that to an std::string which is then returned to the caller.
 *
 * @param dev - The usb_device* from which we want the serial number string.
 *
 * @return std::string
 * @retval The serial number string of the device.  For VM-USB's this is a
 *         string of the form VMnnnn where nnnn is an integer.
 *
 * @throw std::string exception if any of the USB functions fails indicating
 *        why.
 */
string
CVMUSBusb::serialNo(struct usb_device* dev)
{
  usb_dev_handle* pDevice = usb_open(dev);

  if (pDevice) {
    char szSerialNo[256];	// actual string is only 6chars + null.
    int nBytes = usb_get_string_simple(pDevice, dev->descriptor.iSerialNumber,
				       szSerialNo, sizeof(szSerialNo));
    usb_close(pDevice);

    if (nBytes > 0) {
      return std::string(szSerialNo);
    } else {
      throw std::string("usb_get_string_simple failed in CVMUSBusb::serialNo");
    }
				       
  } else {
    throw std::string("usb_open failed in CVMUSBusb::serialNo");
  }

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
CVMUSBusb::CVMUSBusb(struct usb_device* device) :
    m_handle(0),
    m_device(device),
    m_timeout(DEFAULT_TIMEOUT)
{
  m_serial = serialNo(device);                  // Set the desired serial number.
  openVMUsb();
}
////////////////////////////////////////////////////////////////
/*!
    Destruction of the interface involves releasing the claimed
    interface, followed by closing the device.
*/
CVMUSBusb::~CVMUSBusb()
{
    usb_release_interface(m_handle, 0);
    usb_close(m_handle);
    Os::usleep(5000);
}

/**
 * reconnect
 *
 * re open the VM-USB.
 * this is done by closing the device and then invoking
 * openVMUSBUsb which has code common to us and
 * the construtor.
 */
void
CVMUSBusb::reconnect()
{
  usb_release_interface(m_handle, 0);
  usb_close(m_handle);
  Os::usleep(1000);			// Let this all happen
  openVMUsb();

}


//////////////////////////////////////////////////////////////////////////
/////////////////////////// List operations  ////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
/*!
    Execute a list immediately.  It is the caller's responsibility
    to ensure that no data taking is in progress and that data taking
    has run down (the last buffer was received).  
    The list is transformed into an out packet to the VMUSB and
    transaction is called to execute it and to get the response back.
    \param list  : CVMUSBReadoutList&
       A reference to the list of operations to execute.
    \param pReadBuffer : void*
       A pointer to the buffer that will receive the reply data.
    \param readBufferSize : size_t
       number of bytes of data available to the pReadBuffer.
    \param bytesRead : size_t*
       Return value to hold the number of bytes actually read.
 
    \return int
    \retval  0    - All went well.
    \retval -1    - The usb_bulk_write failed.
    \retval -2    - The usb_bulk_read failed.

    In case of failure, the reason for failure is stored in the
    errno global variable.
*/
int
CVMUSBusb::executeList(CVMUSBReadoutList&     list,
		   void*                  pReadoutBuffer,
		   size_t                 readBufferSize,
		   size_t*                bytesRead)
{
  size_t outSize;
  uint16_t* outPacket = listToOutPacket(TAVcsWrite | TAVcsIMMED,
					list, &outSize);
    
    // Now we can execute the transaction:
    
  int status = transaction(outPacket, outSize,
			   pReadoutBuffer, readBufferSize);
  
  
  
  delete []outPacket;
  if(status >= 0) {
    *bytesRead = status;
  } 
  else {
    *bytesRead = 0;
  }
  return (status >= 0) ? 0 : status;
  
}



/*!
   Load a list into the VM-USB for later execution.
   It is the callers responsibility to:
   -  keep track of the lists and their  storage requirements, so that 
      they are not loaded on top of or overlapping
      each other, or so that the available list memory is not exceeded.
   - Ensure that the list number is a valid value (0-7).
   - The listOffset is valid and that there is room in the list memory
     following it for the entire list being loaded.
   This code just load the list, it does not attach it to any specific trigger.
   that is done via register operations performed after all the required lists
   are in place.
    
   \param listNumber : uint8_t  
      Number of the list to load. 
   \param list       : CVMUSBReadoutList
      The constructed list.
   \param listOffset : off_t
      The offset in list memory at which the list is loaded.
      Question for the Wiener/Jtec guys... is this offset a byte or long
      offset... I'm betting it's a longword offset.
*/
int
CVMUSBusb::loadList(uint8_t  listNumber, CVMUSBReadoutList& list, off_t listOffset)
{
  // Need to construct the TA field, straightforward except for the list number
  // which is splattered all over creation.
  
  uint16_t ta = TAVcsSel | TAVcsWrite;
  if (listNumber & 1)  ta |= TAVcsID0;
  if (listNumber & 2)  ta |= TAVcsID1; // Probably the simplest way for this
  if (listNumber & 4)  ta |= TAVcsID2; // few bits.

  size_t   packetSize;
  uint16_t* outPacket = listToOutPacket(ta, list, &packetSize, listOffset);
  int status = usb_bulk_write(m_handle, ENDPOINT_OUT,
			      reinterpret_cast<char*>(outPacket),
			      packetSize, m_timeout);
  if (status < 0) {
    errno = -status;
    status= -1;
  }


  delete []outPacket;
  return (status >= 0) ? 0 : status;


  
}
/*!
  Execute a bulk read for the user.  The user will need to do this
  when the VMUSB is in autonomous data taking mode to read buffers of data
  it has available.
  \param data : void*
     Pointer to user data buffer for the read.
  \param buffersSize : size_t
     size of 'data' in bytes.
  \param transferCount : size_t*
     Number of bytes actually transferred on success.
  \param timeout : int [2000]
     Timeout for the read in ms.
 
  \return int
  \retval 0   Success, transferCount has number of bytes transferred.
  \retval -1  Read failed, errno has the reason. transferCount will be 0.

*/
int 
CVMUSBusb::usbRead(void* data, size_t bufferSize, size_t* transferCount, int timeout)
{
  int status = usb_bulk_read(m_handle, ENDPOINT_IN,
			     static_cast<char*>(data), bufferSize,
			     timeout);
  if (status >= 0) {
    *transferCount = status;
    status = 0;
  } 
  else {
    errno = -status;
    status= -1;
    *transferCount = 0;
  }
  return status;
}

/*! 
   Set a new transaction timeout.  The transaction timeout is used for
   all usb transactions but usbRead where the user has full control.
   \param ms : int
      New timeout in milliseconds.
*/
void
CVMUSBusb::setDefaultTimeout(int ms)
{
  m_timeout = ms;
}


////////////////////////////////////////////////////////////////////////
/////////////////////////////// Utility methods ////////////////////////
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
CVMUSBusb::transaction(void* writePacket, size_t writeSize,
		    void* readPacket,  size_t readSize)
{
    int status = usb_bulk_write(m_handle, ENDPOINT_OUT,
				static_cast<char*>(writePacket), writeSize, 
				m_timeout);
    if (status < 0) {
	errno = -status;
	return -1;		// Write failed!!
    }
    status    = usb_bulk_read(m_handle, ENDPOINT_IN,
			      static_cast<char*>(readPacket), readSize, m_timeout);
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
CVMUSBusb::addToPacket16(void* packet, uint16_t datum)
{
    uint8_t* pPacket = static_cast<uint8_t*>(packet);

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
CVMUSBusb::addToPacket32(void* packet, uint32_t datum)
{
    uint8_t* pPacket = static_cast<uint8_t*>(packet);

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
CVMUSBusb::getFromPacket16(void* packet, uint16_t* datum)
{
    uint8_t* pPacket = static_cast<uint8_t*>(packet);

    uint16_t low = *pPacket++;
    uint16_t high= *pPacket++;

    *datum =  (low | (high << 8));

    return static_cast<void*>(pPacket);
	
}
/*!
   Same as above but a 32 bit item is returned.
*/
void*
CVMUSBusb::getFromPacket32(void* packet, uint32_t* datum)
{
    uint8_t* pPacket = static_cast<uint8_t*>(packet);

    uint32_t b0  = *pPacket++;
    uint32_t b1  = *pPacket++;
    uint32_t b2  = *pPacket++;
    uint32_t b3  = *pPacket++;

    *datum = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);


    return static_cast<void*>(pPacket);
}
/*
   Reading a register is just creating the appropriate CVMUSBReadoutList
   and executing it immediatly.
*/
uint32_t
CVMUSBusb::readRegister(unsigned int address)
{
    CVMUSBReadoutList list;
    uint32_t          data;
    size_t            count;
    list.addRegisterRead(address);

    int status = executeList(list,
			     &data,
			     sizeof(data),
			     &count);
    if (status == -1) {
	char message[100];
	sprintf(message, "CVMUSBusb::readRegister USB write failed: %s",
		strerror(errno));
	throw string(message);
    }
    if (status == -2) {
	char message[100];
	sprintf(message, "CVMUSBusb::readRegister USB read failed %s",
		strerror(errno));
	throw string(message);

    }

    return data;
			     

    
}
/*
  
   Writing a register is just creating the appropriate list and
   executing it immediately:
*/
void
CVMUSBusb::writeRegister(unsigned int address, uint32_t data)
{
    CVMUSBReadoutList list;
    uint32_t          rdstatus;
    size_t            rdcount;
    list.addRegisterWrite(address, data);

    int status = executeList(list,
			     &rdstatus, 
			     sizeof(rdstatus),
			     &rdcount);

    if (status == -1) {
	char message[100];
	sprintf(message, "CVMUSBusb::writeRegister USB write failed: %s",
		strerror(errno));
	throw string(message);
    }
    if (status == -2) {
	char message[100];
	sprintf(message, "CVMUSBusb::writeRegister USB read failed %s",
		strerror(errno));
	throw string(message);

    }
    
}

/*   

    Convert an ISV which value to a register number...
    or throw a string if the register selector is invalid.

*/
unsigned int
CVMUSBusb::whichToISV(int which)
{
    switch (which) {
	case 1:
	    return ISV12;
	case 2:
	    return ISV34;
	case 3:
	    return ISV56;
	case 4:
	    return ISV78;
	default:
	{
	    char msg[100];
	    sprintf(msg, "CVMUSBusb::whichToISV - invalid isv register %d",
		    which);
	    throw string(msg);
	}
    }
}
// If the write list has already been created, this fires it off and returns
// the appropriate status:
//
int
CVMUSBusb::doVMEWrite(CVMUSBReadoutList& list)
{
  uint16_t reply;
  size_t   replyBytes;
  int status = executeList(list, &reply, sizeof(reply), &replyBytes);
  // Bus error:
  if ((status == 0) && (reply == 0)) {
    status = -3;
  }
  return status;
}

// Common code to do a single shot vme read operation:
int
CVMUSBusb::doVMERead(CVMUSBReadoutList& list, uint32_t* datum)
{
  size_t actualRead;
  int status = executeList(list, datum, sizeof(uint32_t), &actualRead);
  return status;
}

//  Utility to create a stack from a transfer address word and
//  a CVMUSBReadoutList and an optional list offset (for non VCG lists).
//  Parameters:
//     uint16_t ta               The transfer address word.
//     CVMUSBReadoutList& list:  The list of operations to create a stack from.
//     size_t* outSize:          Pointer to be filled in with the final out packet size
//     off_t   offset:           If VCG bit is clear and VCS is set, the bottom
//                               16 bits of this are put in as the stack load
//                               offset. Otherwise, this is ignored and
//                               the list lize is treated as a 32 bit value.
//  Returns:
//     A uint16_t* for the list. The result is dynamically allocated
//     and must be released via delete []p e.g.
//
uint16_t*
CVMUSBusb::listToOutPacket(uint16_t ta, CVMUSBReadoutList& list,
			size_t* outSize, off_t offset)
{
    int listLongwords = list.size();
    int listShorts    = listLongwords*sizeof(uint32_t)/sizeof(uint16_t);
    int packetShorts    = (listShorts + 3);
    uint16_t* outPacket = new uint16_t[packetShorts];
    uint16_t* p         = outPacket;
    
    // Fill the outpacket:

    p = static_cast<uint16_t*>(addToPacket16(p, ta)); 
    //
    // The next two words depend on which bits are set in the ta
    //
    if(ta & TAVcsIMMED) {
      p = static_cast<uint16_t*>(addToPacket32(p, listShorts+1)); // 32 bit size.
    }
    else {
      p = static_cast<uint16_t*>(addToPacket16(p, listShorts+1)); // 16 bits only.
      p = static_cast<uint16_t*>(addToPacket16(p, offset));       // list load offset. 
    }

    vector<uint32_t> stack = list.get();
    for (int i = 0; i < listLongwords; i++) {
	p = static_cast<uint16_t*>(addToPacket32(p, stack[i]));
    }
    *outSize = packetShorts*sizeof(short);
    return outPacket;
}

/**
 * openVMUsb
 *
 *   Open the VM-USB.  This contains code that is 
 *   common to both the constructor and reconnect.
 *
 *   We assume that m_serial is set to the
 *   desired VM-USB serial number.
 *
 *   @throw std::string on errors.
 */
void
CVMUSBusb::openVMUsb()
{
    // Since we might be re-opening the device we're going to
    // assume only the serial number is right and re-enumerate
    
    std::vector<struct usb_device*> devices = enumerate();
    m_device = 0;
    for (int i = 0; i < devices.size(); i++) {
        if (serialNo(devices[i]) == m_serial) {
            m_device = devices[i];
            break;
        }
    }
    if (!m_device) {
        std::string msg = "The VM-USB with serial number ";
        msg += m_serial;
        msg += " could not be enumerated";
        throw msg;
    }
    
    
    m_handle  = usb_open(m_device);
    if (!m_handle) {
	throw "CVMUSBusb::CVMUSB  - unable to open the device";
    }
    // Now claim the interface.. again this could in theory fail.. but.

    usb_set_configuration(m_handle, 1);
    int status = usb_claim_interface(m_handle, 
				     0);
    if (status == -EBUSY) {
	throw "CVMUSBusb::CVMUSB - some other process has already claimed";
    }

    if (status == -ENOMEM) {
	throw "CVMUSBusb::CMVUSB - claim failed for lack of memory";
    }
    // Errors we don't know about:

    if (status < 0) {
      std::string msg("Failed to claim the interface: ");
      msg += strerror(-status);
      throw msg;
    }
    usb_clear_halt(m_handle, ENDPOINT_IN);
    usb_clear_halt(m_handle, ENDPOINT_OUT);

    Os::usleep(100);
    
    // Now set the irq mask so that all bits are set..that:
    // - is the only way to ensure the m_irqMask value matches the register.
    // - ensures m_irqMask actually gets set:

    writeIrqMask(0xff);
}
