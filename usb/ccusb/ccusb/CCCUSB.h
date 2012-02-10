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


#ifndef __CCCUSB_H
#define __CCCUSB_H

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

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

//  The structures below are defined in <usb.h> which is included
//  by the implementation and can be treated as opaque by any of our
//  clients (they are in fact opaque in usb.h if memory servers.

struct usb_device;
struct usb_dev_handle;


// Forward Class definitions:

class CCCUSBReadoutList;

/*!
  This class is the low level support for the Wiener/JTEC CCUSB module.
the CCUSB is a USB CAMAC controller.  To use this class you must first locate
a module by invoking enumerate.  enumerate  returns a vector of
usb_device*s.   One of those can be used to instantiate the CCCUSB 
object which can the ben operated on.

*/
class CCCUSB 
{

    // Class member data.
private:
    struct usb_dev_handle*  m_handle;	// Handle open on the device.
    struct usb_device*      m_device;   // Device we are open on.
    int                     m_timeout; // Timeout used when user doesn't give one.

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

    CCCUSB(struct usb_device* vmUsbDevice);
    virtual ~CCCUSB();		// Although this is probably a final class.

    // Disallowed functions as described above.
private:
    CCCUSB(const CCCUSB& rhs);
    CCCUSB& operator=(const CCCUSB& rhs);
    int operator==(const CCCUSB& rhs) const;
    int operator!=(const CCCUSB& rhs) const;
public:

    // Register I/O operations.
public:
    void     writeActionRegister(uint16_t value);


    // The following execute single CAMAC operations.
    // note that the CC-USB defines all registers but the action register to live in
    // CAMAC space.

    int simpleWrite16(int n, int a, int f, uint16_t data, uint16_t& qx);
    int simpleWrite24(int n, int a, int f, uint32_t data, uint16_t& qx);
    int simpleRead16( int n, int a, int f, uint16_t& data, uint16_t& qx);
    int simpleRead24( int n, int a, int f, uint32_t& data, uint16_t& qx);
    int simpleControl(int n, int a, int f, uint16_t& qx);

    // Convenience function that access the CC-USB registers.

    int readFirmware(uint32_t& value);

    int readGlobalMode(uint16_t& value);
    int writeGlobalMode(uint16_t value);

    int readDelays(uint16_t& value);
    int writeDelays(uint16_t value);

    int readScalerControl(uint32_t& value);
    int writeScalerControl(uint32_t value);

    int readLedSelector(uint32_t& value);
    int writeLedSelector(uint32_t value);
    int readOutputSelector(uint32_t& value);
    int writeOutputSelector(uint32_t value);

    int readDeviceSourceSelectors(uint32_t& value);
    int writeDeviceSourceSelectors(uint32_t value);

    int readDGGA(uint32_t& value);
    int readDGGB(uint32_t& value);
    int readDGGExt(uint32_t& value);
    int writeDGGA(uint32_t value);
    int writeDGGB(uint32_t value);
    int writeDGGExt(uint32_t value);

    int readScalerA(uint32_t& value);
    int readScalerB(uint32_t& value);

    int readLamTriggers(uint32_t& value);
    int writeLamTriggers(uint32_t value);

    int readUSBBulkTransferSetup(uint32_t& value);
    int writeUSBBulkTransferSetup(uint32_t value);
    
    int c();
    int z();
    int inhibit();
    int uninhibit();
    

    // List operations.

public:
    int executeList(CCCUSBReadoutList& list,
		    void*               pReadBuffer,
		    size_t              readBufferSize,
		    size_t*             bytesRead);

    int loadList(uint8_t                listNumber,
		 CCCUSBReadoutList&    list);

    // Once the interface is in DAQ auntonomous mode, the application
    // should call the following function to read acquired data.

    int usbRead(void* data, size_t bufferSize, size_t* transferCount,
		int timeout = 2000);

    // Other administrative functions:

    void setDefaultTimeout(int ms); // Can alter internally used timeouts.

    // Register bit definintions.

    // Local functions:
private:
    int transaction(void* writePacket, size_t writeSize,
		    void* readPacket,  size_t readSize);

    void* addToPacket16(void* packet,   uint16_t datum);
    void* addToPacket32(void* packet,   uint32_t datum);
    void* getFromPacket16(void* packet, uint16_t* datum);
    void* getFromPacket32(void* packet, uint32_t* datum);

    uint16_t* listToOutPacket(uint16_t ta, CCCUSBReadoutList& list, size_t* outSize);


    int read32(int n, int a, int f, uint32_t& data);
    int read16(int n, int a, int f, uint16_t& data); /* Really just for register reads */

    int write32(int n, int a, int f, uint32_t data, uint16_t& qx);
    int write16(int n, int a, int f, uint16_t data, uint16_t& qx); /*  just for register writes */


  // The following are classes that define bits/fields in the registers of the CC-USB.
  // Each class is one register:

public:
  //!  Action register - all data members are individual bits.
  class ActionRegister {
  public:
    static const uint16_t startDAQ   = 1;
    static const uint16_t usbTrigger = 2;
    static const uint16_t clear      = 4;
    static const uint16_t scalerDump = 0x10;
    
  };

  //! Firmware register *Mask are in place masks, *Shift shift the field to low order justify it
  class FirmwareRegister {
  public:
    static const uint32_t  revisionMask  = 0xff;
    static const uint32_t  revisionShift = 0;

    static const uint32_t  yearMask      = 0xf00;
    static const uint32_t  yearShift     = 8;

    static const uint32_t  monthMask     = 0xf000;
    static const uint32_t  monthShift    = 12;
  };

  //! Fields and value for the Global mode register.
  class GlobalModeRegister {
  public:
    static const uint16_t bufferLenMask    = 0xf;
    static const uint16_t bufferLenShift   = 0;
    static const uint16_t bufferLen4K      = 0;
    static const uint16_t bufferLen2K      = 1;
    static const uint16_t bufferLen1K      = 2;
    static const uint16_t bufferLen512     = 3;
    static const uint16_t bufferLen256     = 4;
    static const uint16_t bufferLen128     = 5;
    static const uint16_t bufferLen64      = 6;
    static const uint16_t bufferLenSingle  = 7;
    
    static const uint16_t mixedBuffers     = 0x20;
    
    static const uint16_t doubleHeader     = 0x100;
    
    static const uint16_t enableSecondary  = 0x1000;
    };

  //!  The Delay register sets the post trigger delay prior to starting a list, and lam timeout.
  class DelayRegister {
  public:
    static const uint16_t triggerDelayMask = 0xff;
    static const uint16_t triggerDelayShift= 0;
    static const uint16_t lamTimeoutMask   = 0xff00;
    static const uint16_t lamTimeoutShift  = 8;
  };

  //!  The Scaler Control register determines when/how often scaler events are read:

  class ScalerControlRegister {
  public:
    static const uint32_t eventsCountMask   = 0xffff; // Events between readouts.
    static const uint32_t eventsCountShift  = 0;

    static const uint32_t timeIntervalMask  = 0xff0000;	// .5 second units between readouts.
    static const uint32_t timeIntervalShift = 16;
  };

  //! The LED source selector register determines which LEDs mean what.

  class LedSourceRegister {
  public:
    // Red LED:

    static const uint32_t redEventTrigger       = 0;
    static const uint32_t redBusy               = 1;
    static const uint32_t redUSBTrigger         = 2;
    static const uint32_t redUSBOutFifoNotEmpty = 3;
    static const uint32_t redUSBInFifoNotFull  = 4;
    static const uint32_t redAcquire            = 6; // Yes 5 is skipped.
    static const uint32_t redCAMACF2            = 7;
    static const uint32_t redInvert             = 0x10;
    static const uint32_t redLatch              = 0x20;

    // Greeen LED:

    static const uint32_t greenAcquire          = 0x000;
    static const uint32_t greenCAMACF1          = 0x100;
    static const uint32_t greenEventTrigger     = 0x300;	// Yes there's a skip.
    static const uint32_t greenCAMACN           = 0x400;
    static const uint32_t greenI1               = 0x600;        // another skip.
    static const uint32_t greenUSBInFifoNotEmpty= 0x700;
    static const uint32_t greenInvert           = 0x1000;
    static const uint32_t greenLatch            = 0x2000;

    // Yellow LED:

    static const uint32_t yellowI3                = 0x00000;
    static const uint32_t yellowBusy              = 0x10000;
    static const uint32_t yellowI2                = 0x20000;
    static const uint32_t yellowCAMACS1           = 0x30000;
    static const uint32_t yellowCAMACS2           = 0x40000;
    static const uint32_t yellowUSBInFifoNotEmpty = 0x50000;
    static const uint32_t yellowScalerReadout     = 0x60000;
    static const uint32_t yellowUSBTrigger        = 0x70000;
    static const uint32_t yellowInvert            = 0x100000;
    static const uint32_t yellowLatch             = 0x200000;
    
  };
  //! The Output selector register determines the meaning of the NIM Outputs.

  class OutputSourceRegister {
  public:
    // NIM O1
    
    static const uint32_t nimO1Busy              = 0x000000;
    static const uint32_t nimO1EventTrigger      = 0x000001;
    static const uint32_t nimO1DGGA              = 0x000002;
    static const uint32_t nimO1DGGB              = 0x000003;
    static const uint32_t nimO1Latch             = 0x000010;
    static const uint32_t nimO1Invert            = 0x000020;


    // NIM O2


    static const uint32_t nimO2Acquire           = 0x000000;
    static const uint32_t nimO2Event             = 0x000200;
    static const uint32_t nimO2DGGA              = 0x000400;
    static const uint32_t nimO2DGGB              = 0x000600;
    static const uint32_t nimO2Latch             = 0x001000;
    static const uint32_t nimO2Invert            = 0x002000;


    // NIM O3

    static const uint32_t nimO3EndOfBusy         = 0x000000;
    static const uint32_t nimO3Busy              = 0x020000;
    static const uint32_t nimO3DGGA              = 0x040000;
    static const uint32_t nim03DGGB              = 0x060000;
    
    static const uint32_t nimO3Latch             = 0x100000;
    static const uint32_t nimO3Invert            = 0x200000;

  };
  //! Device source selector sets up the inputs to the internal devices:

  class DeviceSourceSelectorsRegister {
  public:
    // Scaler A source/control

    static const uint32_t scalerADisabled        = 0x00000000;
    static const uint32_t scalerAI1              = 0x00000001; // count on NIM I1 etc...
    static const uint32_t scalerAI2              = 0x00000002;
    static const uint32_t scalerAI3              = 0x00000003;
    static const uint32_t scalerAEvent           = 0x00000004;
    static const uint32_t scalerACarryB          = 0x00000005;
    static const uint32_t scalerADGGA            = 0x00000006;
    static const uint32_t scalerADGGB            = 0x00000007;

    static const uint32_t scalerAEnable          = 0x00000010;
    static const uint32_t scalerAReset           = 0x00000020;
    static const uint32_t scalerAFreezeReg       = 0x00000040;
    
    // Scaler B source/control

    static const uint32_t scalerBDisabled       = 0x00000000;
    static const uint32_t scalerBI1             = 0x00000100;
    static const uint32_t scalerBI2             = 0x00000200;
    static const uint32_t scalerBI3             = 0x00000300;
    static const uint32_t scalerBEvent          = 0x00000400;
    static const uint32_t scalerBCarryA         = 0x00000500;
    static const uint32_t scalerBDGGA           = 0x00000600;
    static const uint32_t scalerBDGGB           = 0x00000700;

    static const uint32_t scalerBEnable         = 0x00001000;
    static const uint32_t scalerBReset          = 0x00002000;
    static const uint32_t scalerBFreezeReg      = 0x00004000;

    // Gate and delay generator A input source:

    static const uint32_t dggADisabled          = 0x00000000;
    static const uint32_t dggAI1                = 0x00010000;
    static const uint32_t dggAI2                = 0x00020000;
    static const uint32_t dggAI3                = 0x00030000;
    static const uint32_t dggAEvent             = 0x00040000;
    static const uint32_t dggAEndOfEvent        = 0x00050000;
    static const uint32_t dggAUSBTrigger        = 0x00060000;
    static const uint32_t dggAPulser            = 0x00070000;

    // Gate and delay generator B input source:

    static const uint32_t dggBDisabled          = 0x00000000;
    static const uint32_t dggBI1                = 0x01000000;
    static const uint32_t dggBI2                = 0x02000000;
    static const uint32_t dggBI3                = 0x03000000;
    static const uint32_t dggBEvent             = 0x04000000;
    static const uint32_t dggBEndOfTrigger      = 0x05000000;
    static const uint32_t dggBUSBTrigger        = 0x06000000;
    static const uint32_t dggBPulser            = 0x07000000;
  };
  //! Bit fields for the two DGG gate width/delay registers.
  class DGGAndPulserRegister {
  public:
    static const uint32_t dggFineDelayMask        = 0xffff;
    static const uint32_t dggFineDelayShift       = 0;
    
    static const uint32_t dggGateWidthMask        = 0xffff0000;
    static const uint32_t dggGateWidthShift       = 16;
    
  };
  //! Bit fields for the extended course delay register.
  class DGGCoarseRegister {
  public:
    static const uint32_t ACoarseMask             = 0xffff;
    static const uint32_t ACoarseShift            = 0;
    
    static const uint32_t BCoarseMask             = 0xffff0000;
    static const uint32_t BCoarseShift            = 16;
  };
  //! Multibuffer/timeout setup is in the TransferSetup Register.
  class TransferSetupRegister {
  public:
    static const uint32_t multiBufferCountMask   = 0xff;
    static const uint32_t multiBufferCountShift  = 0;
    
    static const uint32_t timeoutMask            = 0xf00;
    static const uint32_t timeoutShift           = 8;
    
  };
  //! Bits in the Q/X response word for e.g. simple ops.
  
  static const uint16_t Q = 1;
  static const uint16_t X = 2;
};


#endif
