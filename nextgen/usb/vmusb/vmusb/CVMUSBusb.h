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




#ifndef __CVMUSBUSB_H
#define __CVMUSBUSB_H

#ifndef __CVMUSB_H
#include "CVMUSB.h"
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __CRT_SYS_TYPES_H
#include <sys/types.h>
#ifndef __CRT_SYS_TYPES_H
#define __CRT_SYS_TYPES_H
#endif
#endif


//  The structures below are defined in <usb.h> which is included
//  by the implementation and can be treated as opaque by any of our
//  clients (they are in fact opaque in usb.h if memory servers.

struct usb_device;
struct usb_dev_handle;


// Forward Class definitions:

class CVMUSBReadoutList;

/*!
   This class is part of the support package for the Wiener/JTEC VM-USB 
   USB to VME interface.  This class is intended to be used in conjunction
   with CVMUSBReadoutList.
   CVMUSB is used to directly manipulate the controller and to perform
   single shot VME operations.   CVMEReadoutList is intended to be used
   to compose lists of VME operations that can either be run immediately
   by calling executeList or downloaded for triggered operation when
   data taking is turned on via loadList.

   The class is instantiated on a usb_device.  The list of usb_devices
   that correspond to VM-USB's is gotten via a call to the static function
   CVMUSB::enumerate().

 

*/

class CVMUSBusb : public CVMUSB 
{

    // Class member data.
private:
    struct usb_dev_handle*  m_handle;	// Handle open on the device.
    struct usb_device*      m_device;   // Device we are open on.
    int                     m_timeout; // Timeout used when user doesn't give one.
    uint16_t                m_irqMask; // interrupt mask shadow register.

    // Static functions.
public:
    static std::vector<struct usb_device*> enumerate();
    static std::string serialNo(struct usb_device* dev);

    // Constructors and other canonical functions.
    // Note that since destruction closes the handle and there's no
    // good way to share usb objects, copy construction and
    // assignment are forbidden.
    // Furthermore, since constructing implies a usb_claim_interface()
    // and destruction implies a usb_release_interface(),
    // equality comparison has no useful meaning either:

    CVMUSBusb(struct usb_device* vmUsbDevice);
    virtual ~CVMUSBusb();		// Although this is probably a final class.

    // Disallowed functions as described above.
private:
    CVMUSBusb(const CVMUSBusb& rhs);
    CVMUSBusb& operator=(const CVMUSBusb& rhs);
    int operator==(const CVMUSBusb& rhs) const;
    int operator!=(const CVMUSBusb& rhs) const;
public:

    // Register I/O operations.
public:
    void     writeActionRegister(uint16_t value);


    int readFirmwareID();

    void     writeGlobalMode(uint16_t value);
    int  readGlobalMode();






    void     writeDAQSettings(uint32_t value);
    uint32_t readDAQSettings();




    void    writeLEDSource(uint32_t value);
    int     readLEDSource();


    void     writeDeviceSource(uint32_t value);
    int      readDeviceSource();


    void     writeDGG_A(uint32_t value);
    uint32_t readDGG_A();

    void     writeDGG_B(uint32_t value);
    uint32_t readDGG_B();

    void     writeDGG_Extended(uint32_t value);
    uint32_t readDGG_Extended();

    uint32_t readScalerA();
    uint32_t readScalerB();


    void     writeVector(int which, uint32_t value);
    int      readVector(int which);


    void     writeIrqMask(uint8_t mask);
    int      readIrqMask();

    void     writeBulkXferSetup(uint32_t value);
    int      readBulkXferSetup();



    

    // VME transfer operations (1 shot)

    int vmeWrite32(uint32_t address, uint8_t aModifier, uint32_t data);
    int vmeRead32(uint32_t address, uint8_t aModifier, uint32_t* data);



    int vmeWrite16(uint32_t address, uint8_t aModifier, uint16_t data);
    int vmeRead16(uint32_t address, uint8_t aModifier, uint16_t* data);

    int vmeWrite8(uint32_t address, uint8_t aModifier, uint8_t data);
    int vmeRead8(uint32_t address, uint8_t aModifier, uint8_t* data);

    int vmeBlockRead(uint32_t baseAddress, uint8_t aModifier,
		     void* data,  size_t transferCount, size_t* countTransferred);
    int vmeFifoRead(uint32_t address, uint8_t aModifier,
         	    void* data, size_t transferCount, size_t* countTransferred);

    

    // Support for immediate counted VME variable block transfer operations:
    // See comments prior to CVMEReadoutList::addBlockCountMask

    int vmeReadBlockCount8(uint32_t address,  uint32_t mask, uint8_t amod);
    int vmeReadBlockCount16(uint32_t address, uint32_t mask, uint8_t amod);
    int vmeReadBlockCount32(uint32_t address, uint32_t mask, uint8_t amod);


    int vmeVariableBlockRead(uint32_t address, uint8_t amod, 
			     void* data, size_t maxCount, size_t* countTransferred);
    int vmeVariableFifoRead(uint32_t address, uint8_t amod,  
			    void* data, size_t maxCount,  size_t* countTransferred);


    
    // List operations.

public:
    int executeList(CVMUSBReadoutList& list,
		    void*               pReadBuffer,
		    size_t              readBufferSize,
		    size_t*             bytesRead);
    
    int loadList(uint8_t                listNumber,
		 CVMUSBReadoutList&    list,
		 off_t                  listOffset = 0);
      

    // Once the interface is in DAQ auntonomous mode, the application
    // should call the following function to read acquired data.

    int usbRead(void* data, size_t bufferSize, size_t* transferCount,
		int timeout = 2000);

    // Other administrative functions:

    void setDefaultTimeout(int ms); // Can alter internally used timeouts.

    // Register bit definintions.

public: 

private:
    int transaction(void* writePacket, size_t writeSize,
		    void* readPacket,  size_t readSize);

    void* addToPacket16(void* packet,   uint16_t datum);
    void* addToPacket32(void* packet,   uint32_t datum);
    void* getFromPacket16(void* packet, uint16_t* datum);
    void* getFromPacket32(void* packet, uint32_t* datum);
    void  writeRegister(unsigned int address, uint32_t data);
    uint32_t readRegister(unsigned int address);
    unsigned int whichToISV(int which);
    int   doVMEWrite(CVMUSBReadoutList& list);
    int   doVMERead(CVMUSBReadoutList&  list, uint32_t* datum);
    uint16_t* listToOutPacket(uint16_t ta, CVMUSBReadoutList& list, size_t* outSize,
			      off_t offset = 0);
};

#endif
