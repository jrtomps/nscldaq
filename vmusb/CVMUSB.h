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


#ifndef __CVMUSB_H
#define __CVMUSB_H

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
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
class CVMUSB 
{

    // Class member data.
private:
    usb_dev_handle*  m_handle;	// Handle open on the device.
    usb_device*      m_device;  // Device we are open on.

    // Static functions.
public:
    static vector<usb_device*> enumerate();

    // Constructors and other canonical functions.
    // Note that since destruction closes the handle and there's no
    // good way to share usb objects, copy construction and
    // assignment are forbidden.
    // Furthermore, since constructing implies a usb_claim_interface()
    // and destruction implies a usb_release_interface(),
    // equality comparison has no useful meaning either:

    CVMUSB(usb_device* vmUsbDevice);
    virtual ~CVMUSB();		// Although this is probably a final class.

    // Disallowed functions as described above.
private:
    CVMUSB(const CVMUSB& rhs);
    CVMUSB& operator=(const CVMUSB& rhs);
    int operator==(const CVMUSB& rhs) const;
    int operator!=(const CVMUSB& rhs) const;
public:

    // Register I/O operations.
public:
    void     writeActionRegister(uint16_t value);
    uint16_t readActionRegister();

    uint32_t readFirmwareID();

    void     writeGlobalMode(uint16_t value);
    uint16_t readGlobalMode();

    void     writeDAQSettings(uint32_t value);
    uint32_t readDAQSettings(uint32_t value);

    void     writeLEDSource(uint32_t value);
    uint32_t readLEDSource();

    void     writeDeviceSource(uint32_t value);
    uint32_t readLEDSource();

    void     writeDGG_A(uint32_t value);
    uint32_t readDGG_A();

    void     writeDGG_B(uint32_t value);
    uint32_t readDGG_B();

    void     writeDGG_Extended(uint32_t value);
    uint32_t readDGG_Extended();

    uint32_t readScalerA();
    uint32_t readScalerB();

    void     writeCountExtractMask(uint32_t value);
    uint32_t readCountExtractMask();

    void     writeVector(uint32_t value);
    uint32_t readVector();

    void     writeBulkXferSetup(uint32_t value);
    uint32_t readBulkXferSetup();


    

    // VME transfer operations (1 shot)

    int vmeWrite32(uint32_t address, uint8_t aModifier, uint32_t data);
    int vmeRead32(uint32_t address, uint8_t aModifier, uint32_t* data);

    int vmeWrite16(uint32_t address, uint8_t aModifier, uint16_t data);
    int vmeRead16(uint32_t address, uint8_t aModifier, uint16_t* data);

    int vmeWrite8(uint32_t address, uint8_t aModifier, uint8_t data);
    int vmeRead8(uint32_t address, uint8_t aModifier, uint8_t* data);


    int vmeBlockWrite(uint32_t baseAddress, uint8_t aModifier, 
		      void* data, size_t transferCount, size_t* countTransferred);
    int vmeBlockRead(uint32_t baseAddress, uint8_t aModifier,
		     void* data,  size_t transferCount, size_t* countTransferred);
    int vmeFifoRead(uint32_t address, int8_t aModifier,
		    void* data, size_t transferCount, size_t* countTransferred);

    // List operations.

public:
    int executeList(CVMEUSBReadoutList& list,
		    void*               pReadBuffer,
		    size_t              readBufferSize,
		    size_t*             bytesRead);
    
    int loadList(uint8_t                listNumber,
		 CVMEUSBReadoutList&    list,
		 off_t                  listOffset = 0);

    // Once the interface is in DAQ auntonomous mode, the application
    // should call the following function to read acquired data.

    int usbRead(void* data, size_t bufferSize, size_t* transferCount,
		int timeout = 2000);

    // Register bit definintions.

public: 
    class ActionRegister {   // e.g. CVMUSB::ActionRegister::startDAQ is a bit.
    public:
	static const uint16_t startDAQ   = 1;
	static const uint16_t usbTrigger = 2;
	static const uint16_t clear      = 4;
	static const uint16_t sysReset   = 8;
	static const uint16_t scalerDump = 0x10;
    };
    
    class FirmwareRegister {
    public:
	static const uint32_t minorRevMask     = 0x000000ff;
	static const uint32_t minorRevShift    = 0;
	
	static const uint32_t majorRevMask     = 0x0000ff00;
	static const uint32_t minorRevShift    = 8;


	// These are my best guesses.

	static const uint32_t betaVersionMask  = 0x00ff0000;
	static const uint32_t betaVersionShift = 16;

	static const uint32_t yearMask         = 0x0f000000;
	static const uint32_t yearShift        = 24;
       
	static const uint32_t monthMask        = 0xf0000000;
	static const uint32_t monthshfit       = 27;
    };

    class GlobalModeRegister {
    public:
	static const uint16_t bufferLenMask    = 0xf;
	static const uint16_t bufferLenShift   = 0;
	static const uint16_t bufferLen13K     = 0;
	static const uint16_t bufferLen8K      = 1;
	static const uint16_t bufferLen4K      = 2;
	static const uint16_t bufferLen2K      = 3;
	static const uint16_t bufferLen1K      = 4;
	static const uint16_t bufferLen512     = 5;
	static const uint16_t bufferLen256     = 6;
	static const uint16_t bufferLen128     = 7;
	static const uint16_t bufferLen64      = 8;
	static const uint16_t bufferLenSingle  = 9;

	static const uint16_t mixedBuffers     = 0x20;
	static const uint16_t doubleSeparater  = 0x40;
	static const uint16_t align32          = 0x80;
	
	static const uint16_t doubleHeader     = 0x100;
	static const uint16_t busReqLevelMask  = 0x7000;
	static const uint16_t busReqLevelShift = 12;
    };
    class DAQSettingsRegister {
	static const uint32_t readoutTriggerDelayMask     = 0xff;
	static const uint32_t readoutTriggerDelayShift    = 0;
	
	static const uint32_t scalerReadoutPeriodMask     = 0x0xff00;
	static const uint32_t scalerReadoutPeriodShfit    = 8;

	static const uint32_t scalerReadoutFrequenyMask   = 0xffff0000;
	static const uint32_t scalerReadoutFrequencyShift = 16;
    };
    class LedSourceRegister {
	// Top yellow led:

	static const uint32_t topYellowOutFifoNotEmpty    = 0;
	static const uint32_t topYellowInFifoNotEmpty     = 1;
	static const uint32_t topYellowScalerEvent        = 2;
	static const uint32_t topYellowInFifoFull         = 3;
	static const uint32_t topYellowDTACK              = 4;
	static const uint32_t topYellowBERR               = 5;
	static const uint32_t topYellowBusRequest         = 6;
	static const uint32_t topYellowBusGranted         = 7;
	static const uint32_t topYellowInvert             = 0x8;
	static const uint32_t topYellowLatch              = 0x10;

	// Red LED:

	static const uint32_t redEventTrigger           = (0 << 8);
	static const uint32_t redNimInput1              = (1 << 8);
	static const uint32_t redNimInput2              = (2 << 8);
	static const uint32_t redBusy                   = (3 << 8);
	static const uint32_t redDTACK                  = (4 << 8);
	static const uint32_t redBERR                   = (5 << 8);
	static const uint32_t redBusRequest             = (6 << 8);
	static const uint32_t redBusGranted             = (7 << 8);
	static const uint32_t redInvert                 = (8 << 8);
	static const uint32_t redLatch                  = (0x10 << 8);

	// Green led:

	static const uint32_t greenAcquire              = (0 << 16);
	static const uint32_t greenStackNotEmpty        = (1 << 16);
	static const uint32_t greenEventReady           = (2 << 16);
	static const uint32_t greenEventTrigger         = (3 << 16);
	static const uint32_t greenDTACK                = (4 << 16);
	static const uint32_t greenBERR                 = (5 << 16);
	static const uint32_t greenBusRequest           = (6 << 16);
	static const uint32_t greenBusGranted           = (7 << 16);
	static const uint32_t greenInvert               = (8 << 16);
	static const uint32_t greenLatch                = (0x10 << 16);

	// Bottom yellow LED>

	static const uint32_t bottomYellowNotArbiter    = (0 << 24);
	static const uint32_t bottomYellowUsbTrigger    = (1 << 24);
	static const uint32_t bottomYellowUSBReset      = (2 << 24);
	static const uint32_t bottomYellowBERR1         = (3 << 24);
	static const uint32_t bottomYellowDTACK         = (4 << 24);
	static const uint32_t bottomYellowBERR          = (5 << 24);
	static const uint32_t bottomYellowBusRequest    = (6 << 24);
	static const uint32_t bottomYellowBusGranted    = (7 << 24);
	static const uint32_t bottomYellowInvert        = (8 << 24);
	static const uint32_t bottomYellowLatch         = (0x10 << 24);
    };

};



#endif
