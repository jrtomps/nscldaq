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
    void     writeActionRegister(uint32_t value);
    uint32_t readActionRegister();

    uint32_t readFirmwareID();

    void     writeGlobalMode(uint32_t value);
    uint32_t readGlobalMode();

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


    

    // VME I/O operations.

    // List operations.

public:
    int executeList(CVMEUSBReadoutList& list,
		    void*               pReadBuffer,
		    size_t              readBufferSize,
		    size_t*             bytesRead);
    
    int loadList(uint8_t                listNumber,
		 CVMEUSBReadoutList&    list,
		 off_t                  listOffset = 0);

    // Register bit definintions (implemented as nested classes).
};



#endif
