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
#include "CVMUSBReadoutList.h"
#include <usb.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <unistd.h>

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

// Bits in the list target address word:

static const uint16_t TAVcsID0(1); // Bit mask of Stack id bit 0.
static const uint16_t TAVcsSel(2); // Bit mask to select list dnload
static const uint16_t TAVcsWrite(4); // Write bitmask.
static const uint16_t TAVcsIMMED(8); // Target the VCS immediately.
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
CVMUSB::enumerate()
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
CVMUSB::CVMUSB(struct usb_device* device) :
    m_handle(0),
    m_device(device),
    m_timeout(DEFAULT_TIMEOUT)
{
    m_handle  = usb_open(m_device);
    if (!m_handle) {
	throw "CVMUSB::CVMUSB  - unable to open the device";
    }
    // Now claim the interface.. again this could in theory fail.. but.

    usb_set_configuration(m_handle, 1);

    int status = usb_claim_interface(m_handle, 0);
    if (status == -EBUSY) {
	throw "CVMUSB::CVMUSB - some other process has already claimed";
    }
    if (status == -ENOMEM) {
	throw "CVMUSB::CMVUSB - claim failed for lack of memory";
    }
    usleep(5000);

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
    
    pOut = static_cast<char*>(addToPacket16(pOut, 5)); // Select Register block for transfer.
    pOut = static_cast<char*>(addToPacket16(pOut, 10)); // Select action register wthin block.
    pOut = static_cast<char*>(addToPacket16(pOut, value));

    // This operation is write only.

    int outSize = pOut - outPacket;
    int status = usb_bulk_write(m_handle, ENDPOINT_OUT, 
				outPacket, outSize, m_timeout);
    if (status < 0) {
	string message = "Error in usb_bulk_write, writing action register ";
	message == strerror(-status);
	throw message;
    }
    if (status != outSize) {
	throw "usb_bulk_write wrote different size than expected";
    }
}

////////////////////////////////////////////////////////////////////////
/*!
   Read the firmware id register.  This is register 0.
   Doing this is a matter of building a CVMUSBReadoutList to do the
   job and then submitting it for immediate execution.

   \return uint16_t
   \retval The value of the firmware Id register.

*/
uint32_t 
CVMUSB::readFirmwareID()
{
    return readRegister(FIDRegister);
}

////////////////////////////////////////////////////////////////////////

/*!
   Write the global mode register (GMODERegister).
  \param value :uint16_t 
     The 16 bit global mode register value.
*/
void
CVMUSB::writeGlobalMode(uint16_t value)
{
    writeRegister(GMODERegister, static_cast<uint32_t>(value));
}

/*!
    Read the global mode register (GMODERegistesr).
    \return uint16_t  
    \retval the value of the register.
*/
uint16_t 
CVMUSB::readGlobalMode()
{
    return static_cast<uint16_t>(readRegister(GMODERegister));
}

/////////////////////////////////////////////////////////////////////////

/*!
   Write the data acquisition settings register.
  (DAQSetRegister)
  \param value : uint32_t
     The value to write to the register
*/
void
CVMUSB::writeDAQSettings(uint32_t value)
{
    writeRegister(DAQSetRegister, value);
}
/*!
  Read the data acquisition settings register.
  \return uint32_t
  \retval The value read from the register.
*/
uint32_t
CVMUSB::readDAQSettings()
{
    return readRegister(DAQSetRegister);
}
//////////////////////////////////////////////////////////////////////////
/*!
  Write the LED source register.  All of the LEDs on the VMUSB
  are essentially user programmable.   The LED Source register determines
  which LEDS display what.  The register is made up of a bunch of
  3 bit fields, invert and latch bits.  These fields are defined by the
  constants in the CVMUSB::LedSourceRegister class.  For example
  CVMUSB::LedSourceRegister::topYellowInFifoFull ored in to this register
  will make the top yellow led light when the input fifo is full.


  \param value : uint32_t
     The value to write to the LED Src register.
*/
void
CVMUSB::writeLEDSource(uint32_t value)
{
    writeRegister(LEDSrcRegister, value);
}
/*!
   Read the LED Source register.  
   \return uint32_t
   \retval The current value of the LED source register.
*/
uint32_t
CVMUSB::readLEDSource()
{
    return readRegister(LEDSrcRegister);
}
/////////////////////////////////////////////////////////////////////////
/*!
   Write the device source register.  This register determines the
   source of the start for the gate and delay generators. What makes
   scalers increment and which signals are routed to the NIM out
   lemo connectors.
   \param value :uint32_t
      The new value to write to this register.   This is a bitmask that
      consists of code fields along with latch and invert bits.
      These bits are defined in the CVMUSB::DeviceSourceRegister
      class.  e.g. CVMUSB::DeviceSourceRegister::nimO2UsbTrigger1 
      ored into value will cause the O2 to pulse when a USB initiated
      trigger is produced.
*/
void
CVMUSB::writeDeviceSource(uint32_t value)
{
    writeRegister(DEVSrcRegister, value);
}
/*!
   Read the device source register.
   \return uint32_t
   \retval current value of the register.
*/
uint32_t
CVMUSB::readDeviceSource()
{
    return readRegister(DEVSrcRegister);
}
/////////////////////////////////////////////////////////////////////////
/*!
     Write the gate width and delay for delay and gate generator A
     \param value : uint32_t 
       The gate and delay generator value.  This value is split into 
       two fields, the gate width and delay.  Note that the width is
       further modified by the value written to the DGGExtended register.
*/
void
CVMUSB::writeDGG_A(uint32_t value)
{
    writeRegister(DGGARegister, value);
}
/*!
   Read the register controlling the delay and fine width of 
   Delay and Gate register A.
   \return uint32_t
   \retval  register value.
*/
uint32_t
CVMUSB::readDGG_A()
{
    return readRegister(DGGARegister);
}
/*!
  Write the gate with and delay for the B dgg. See writeDGG_A for
  details.
*/
void
CVMUSB::writeDGG_B(uint32_t value)
{
    writeRegister(DGGBRegister, value);
}
/*!
   Reads the control register for the B channel DGG.  See readDGG_A
   for details.
*/
 uint32_t
 CVMUSB::readDGG_B()
{
    return readRegister(DGGBRegister);
}

/*!
   Write the DGG Extension register.  This register was added
   to provide longer gate widths.  It contains the high order
   bits of the widths for both the A and B channels of DGG.
*/
void
CVMUSB::writeDGG_Extended(uint32_t value)
{
    writeRegister(DGGExtended, value);
}
/*!
   Read the value of the DGG extension register.
*/
uint32_t
CVMUSB::readDGG_Extended()
{
    return readRegister(DGGExtended);
}

//////////////////////////////////////////////////////////////////////////
/*!
   Read the A scaler channel
   \return uint32_t
   \retval the value read from the A channel.
*/
uint32_t
CVMUSB::readScalerA()
{
    return readRegister(ScalerA);
}
/*!
   Read the B scaler channel
*/
uint32_t
CVMUSB::readScalerB()
{
    return readRegister(ScalerB);
}

//////////////////////////////////////////////////////////////////////////
/*!
   Write the count extract register.  This register is a mask that
   determines which bits of a datum read from the VME are a count
   of the transfers to use for a subsequent block transfer.

   \param value : uint32_t
      new mask value.
*/
void
CVMUSB::writeCountExtractMask(uint32_t value)
{
    writeRegister(ExtractMask, value);
}
uint32_t
CVMUSB::readCountExtractMask()
{
    return readRegister(ExtractMask);
}
/////////////////////////////////////////////////////////////////////

/*!
    Write an interrupt service vector register.  The
    ISV's allow the application to specific a list to be associated
    with specific VME interrupts.  Each ISV register
    contains a pair of ISV specifications. See the
    CVMSUSB::ISVregister class for bit/mask/shift definitions in this
    register.
    \param which : int
      Specifies which of the ISV registers to write.  This is a value
      between 1 and 4.  1 is ISV12, 2 ISV34 ... 4 ISV78.
    \param value : uint32_t
       The new value to write to the register.

    \throw string  - if the which value is illegal.
*/
void
CVMUSB::writeVector(int which, uint32_t value)
{
    unsigned int regno = whichToISV(which);
    writeRegister(regno, value);
}

/*!
  Read an interrupt service vector register.  See writeVector for
  a discussion that describes these registers.
  \param which : int
      Specifies the ISV register to to read.
  \return int
  \retval the register contents.
 
  \throw string  - if thwhich is illegal.
*/
uint32_t 
CVMUSB::readVector(int which)
{
    unsigned int regno = whichToISV(which);
    return readRegister(regno);
}

//////////////////////////////////////////////////////////////////////////

  
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
CVMUSB::executeList(CVMUSBReadoutList&     list,
		   void*                  pReadoutBuffer,
		   size_t                 readBufferSize,
		   size_t*                bytesRead)
{
    int listLongwords = list.size();
    int listShorts    = listLongwords*sizeof(uint32_t)/sizeof(uint16_t);
    int packetShorts    = (listShorts + 3);
    uint16_t* outPacket = new uint16_t[packetShorts];
    uint16_t* p         = outPacket;
    
    // Fill the outpacket:

    p = static_cast<uint16_t*>(addToPacket16(p, TAVcsWrite | TAVcsIMMED));  // Write for immed execution
    p = static_cast<uint16_t*>(addToPacket32(p, listShorts+1));

    vector<uint32_t> stack = list.get();
    for (int i = 0; i < listLongwords; i++) {
	p = static_cast<uint16_t*>(addToPacket32(p, stack[i]));
    }

    // Now we can execute the transaction:

    int status = transaction(outPacket, packetShorts*sizeof(uint16_t),
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
///////////////////////////////////////////////////////////////////////
/*!
    write the bluk transfer setup register.  This register
    sets up a few of the late breaking data taking parameters
    that are built to allow data to flow through the USB more
    effectively.  For bit/mask/shift-count definitions of this
    register see CVMUSB::TransferSetup.
    \param value : uint32_t
      The value to write to the register.
*/
void
CVMUSB::writeBulkXferSetup(uint32_t value)
{
    writeRegister(USBSetup, value);
}
/*!
   Read the bulk transfer setup register.
*/
uint32_t
CVMUSB::readBulkXferSetup()
{
    return readRegister(USBSetup);
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
CVMUSB::transaction(void* writePacket, size_t writeSize,
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
CVMUSB::addToPacket32(void* packet, uint32_t datum)
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
    char* pPacket = static_cast<char*>(packet);

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
CVMUSB::readRegister(unsigned int address)
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
	sprintf(message, "CVMUSB::readRegister USB write failed: %s",
		strerror(errno));
	throw string(message);
    }
    if (status == -2) {
	char message[100];
	sprintf(message, "CVMUSB::readRegister USB read failed %s",
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
CVMUSB::writeRegister(unsigned int address, uint32_t data)
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
	sprintf(message, "CVMUSB::writeRegister USB write failed: %s",
		strerror(errno));
	throw string(message);
    }
    if (status == -2) {
	char message[100];
	sprintf(message, "CVMUSB::writeRegister USB read failed %s",
		strerror(errno));
	throw string(message);

    }
    
}

/*   

    Convert an ISV which value to a register number...
    or throw a string if the register selector is invalid.

*/
unsigned int
CVMUSB::whichToISV(int which)
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
	    sprintf(msg, "CVMUSB::whichToISV - invalid isv register %d",
		    which);
	    throw string(msg);
	}
    }
}
