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

// This is the easy-vme interface to the Wiener USB/VME device.
// The 'standard' usage is not very fast, however a downloadable
// microprogram (called the stack) allows the system to get very good
// performance.   See device specific interfaces.
// 
// We assume that the usb devices allow us to rw them (see wienerusbd).

// Headers

#include <config.h>
#include <CVMEInterface.h>	// which we are implementing.
#include "WienerUSBVMEInterface.h" // Wiener USB/VME extensions.
#include <usb.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <string>

using namespace std;

// Constant definitions

static const short USB_WIENER_VENDOR_ID(0x16dc);
static const short USB_VMUSB_PRODUCT_ID(0xb);

static const int   ENDPOINT_OUT(2); // Outbound endpoint.
static const int   ENDPOINT_IN(0x86); // Inbound endpoint.

// Constants to help construct the stack:

static const unsigned int   STACK_MAXIMMEDIATE(7); // Maximum transfers safe in an

   
                                               // immediate stack.
static const unsigned int   STACK_MAXSIZE(0xfff);
static const unsigned short STACK_SIZEPERREAD(4); // Words per read operation.
static const unsigned short STACK_SIZEPERWRITE(6); // Words per write operation.

// Primary addresses:

static const unsigned short  TA_REGISTERBLOCK(1);
static const unsigned short  TA_MCS(2);
static const unsigned short  TA_ACS(3);
static const unsigned short  TA_VCG(8);
static const unsigned short  REQ_DATA(4); // Or with TA_xxxx to req data.

// First line of the stack is MODE:

static const unsigned long MODE_NODS0(0x40);
static const unsigned long MODE_NODS1(0x80);
static const unsigned long MODE_READ(0x100);
static const unsigned long MODE_BIGENDIAN(0x1000);
static const unsigned long MODE_HITDATA(0x2000);
static const unsigned long MODE_HITMODE(0x8000);
static const unsigned long MODE_PRODUCTSHIFT(22); // left shift product terms here.
static const unsigned long MODE_BLTSHIFT(24); // left shift BLT count here.

// Second line is the adddress, but has lword too:

static const unsigned long ADDRESS_LWORD(1); // This is really *lword.

// Finally, we need to do reads even on writes

static const unsigned long DUMMYSIZE(9192); // From Jan's libxxusb.cpp.


// USB Timeouts (taken from Jan's libxxusb.cpp).

static const unsigned long WRITE_TIMEOUT(20000);	//  in ms.
static const unsigned long READ_TIMEOUT(30000); // in ms.

// Claim retry time in usec.

static const unsigned long    USB_CLAIM_RETRYDELAY(1000); // microseconds.

// Data Structures:

// Unit defines an 'open' vme device.  In this case the open model
// is that of opening an address space (modifier) in a VME crate.
// Since each usb unit is capable of carrying any address modifier,
// The AM is computed and carried with the Unit structure.

typedef struct _Unit {
  usb_dev_handle*   s_pHandle;	// Handle open on the unit.
  unsigned char     s_AM;	// VME Address modifier.
} Unit, *pUnit;


// Modifier map defines a mapping between an AddressMode and a
// vme bus address modifier:
//
typedef struct _ModifierMap {
  enum CVMEInterface::AddressMode s_Mode;
  unsigned char                   s_Modifier;  
} ModifierMap, *pModifierMap;



// The usb bus is a dynamic entity.  On the first open we
// will probe that bus enumerating the usb interfaces that are present.
// the assumption is that the vme crates will not by 'heavily' dynamic
// but will only slowly vary, and will remain the same for the life of a program.
//

static bool                 initialized(false);
typedef struct usb_device *usb_device_ptr;
static std::vector<usb_device_ptr>  vmeCrates;	// Array of crates we found.

// Map CVMEInterface::AddressMode -> VME AM's.

static ModifierMap ModifierMappings[6] = {
  {CVMEInterface::A16, 0x2d},
  {CVMEInterface::A24, 0x3d},
  {CVMEInterface::A32, 0x0d},

  {CVMEInterface::GEO, 0x2f},
  {CVMEInterface::MCST,0x0d},
  {CVMEInterface::CBLT,0x0b}
};
static const int numModifiers = sizeof(ModifierMappings)/sizeof(ModifierMap);


// Local function to initialize our static data structures:
//
//   - Initialize the VME bus.
//   - Probe the VME bus looking for VC_USB devices.
//   - The usb_device* for each found crate is stored in the vmeCrates vector.
//     the index in the vector defines the crate number.
//   For what it's worth, we do a deep bus search (that is each bus is completely
//   enumerated before going on to the next bus.
//
static void Initialize() {
  usb_init();
  if(usb_find_busses() < 0) {
    throw string("Failed to find usb busses");
  }
  if(usb_find_devices() < 0) {
    throw string("Failed in usb_find_devices");
  }
  struct usb_bus* busses = usb_get_busses();
  while(busses) {
    struct usb_device* aDevice = busses->devices;
    while(aDevice) {
      // We found a device if the vendor is Wiener and the product id is
      // correct:
      
      if( (aDevice->descriptor.idVendor  == USB_WIENER_VENDOR_ID) &&
	  (aDevice->descriptor.idProduct == USB_VMUSB_PRODUCT_ID)) {
	vmeCrates.push_back(aDevice);
      }

      aDevice = aDevice->next;
    }
    busses = busses->next;
  }
  initialized = true;

}

/*!
   Local function to translate an AddressMode to an address modifier.
   \param mode 
       The CVMEInterface::Address mode.
   \return unsigned char
   \retval  The address modifier corresponding to this address mode.

   \throw string
     IF the addresss mode is not found in the lookup table.

 */
unsigned char
AddressModifier(CVMEInterface::AddressMode mode) 
{
  for (int i =0; i < numModifiers; i++) {
    if (mode  == ModifierMappings[i].s_Mode) {
      return ModifierMappings[i].s_Modifier;
    }
  }
  throw string("Address modifier is invalid and could not be translated");
}

/*!
   Public interface to 'open' a vme crate.   What we do is 
   do a usb_open on the crate requested, create and fill in a Unit 
   structure passing a pointer to that structure as an opaque type.

   \param nMode
      The CVMEInterface::AdddressMode to open the device on.
   \param crate
      The number of the crate to open.

   \return void*
   \retval An opaque pointer to use in other calls to this package.

   \throw string  - If the open fails.
   \throw string  - If the address modifier is nonsense.
   \throw string  - If the crate number is out of range.
*/
void*
CVMEInterface::Open(CVMEInterface::AddressMode nMode,
		    unsigned short crate)
{
  // If necessary probe the bus:

  if(!initialized) {
    Initialize();
  }

  // Ensure the crate and mode are valid:

  if(crate >= vmeCrates.size()) {
    throw string("Invalid crate number in CVMEInterface::Open");
  }
  unsigned char AM = AddressModifier(nMode); // Throws on error.

  // Now try to open the device:

  usb_dev_handle* handle = usb_open(vmeCrates[crate]);
  if(!handle) {
    throw string("Unable to open the cdrate in CVMEInterface::Open");
  }

  // Create the unit, fill it in and hand it to the caller.
  // There's a tacit assumption that multiple opens can be done on the
  // same 'device'.

  pUnit unit       = new Unit;
  unit->s_pHandle = handle;
  unit->s_AM      = AM;

  usleep(150);
  return (void*) unit;
}
/*!
   Close a VME unit.
   - usb_close() is called for the open device.
   - The Unit structure allocated for the device is destroyed.

   \param handle
       The opaque device handle gotten from the Open call.
   \throw string if the usb_close failed.
*/
void
CVMEInterface::Close(void* handle)
{
  pUnit p = static_cast<pUnit>(handle);
  if (usb_close(p->s_pHandle) < 0) {
    throw string("usb_close failed, most likely reason is bad handle");
  }
  delete p;

}
/*!
   Map is unsupported and will throw an exception
*/
void* 
CVMEInterface::Map(void* handle, unsigned long base, unsigned long nBytes)
{
  throw string("Wiener VC_USB does not support memory mapped I/O");
}
/*!
   Unmap is unsupported and will throw an exception.
*/
void
CVMEInterface::Unmap(void* handle, void* p, unsigned long bytes)
{
  throw string("Wiener VC_USB does not support memory mapped I/O");
}

// Interface specific functions.

/*!
  Read a block of longwords from the VME.  At present there are some limiting
  assumptions:
  - The base address is longword aligned.
  - Even BLTs are done as individual cycles (that is a stack is created
    consisting of count transfers.
  - At most 7 transactions are safe.

   \param handle - Handle received from the Open call.
   \param base   - The VME Base address for the transfer.  It is a requirement
                   in this version that (base & 0xfffffffc) == base
                   [base is longword aligned].
   \param pBuffer - Buffer into which the data will be read.
   \param count   - number of longwords to transfer.

   \return int
   \retval  Number of bytes transferred.

   \throw   string - If the base address is not longword aligned.
   \throw   string - If the stack length is too long.
   \throw   string - on an I/O error.

 */
int 
WienerUSBVMEInterface::atomicReadLongs(void* handle, unsigned long base, void* pBuffer, 
				 unsigned long count)
{
  pUnit pHandle = (pUnit)handle;
  usb_dev_handle* pDevice    = pHandle->s_pHandle;
  unsigned char   aModifier  = pHandle->s_AM;

  // Error checking: The base address must be longword aligned:
  // and count*4 must be within 0xffc  (a safety fudge here).

  if ((base & 0xfffffffc) != base) {
    throw string("WienerUSBVMEInterface::atomicReadLongs - address not long aligned");
  }
  if ( count > STACK_MAXIMMEDIATE) {
    throw string("WienerUSBVMEInterface::atomicReadLongs - transfer count too large");
  }
  // Now build the stack:

  unsigned long stack[STACK_MAXSIZE]; // this is overly large.
  stack[0]   = count*STACK_SIZEPERREAD+1; // transfer words+ header.
  unsigned long* pStack(&(stack[1]));

  for (int i =0; i<=count; i++) {
    *pStack++ = aModifier | MODE_READ;
    *pStack++ = base;
    base     += 4;		// Next address.
  }
  usbImmediateStackTransaction(pHandle, stack, pBuffer, count*sizeof(long));
  return count*sizeof(long);

}
/*!
   Read a block of words from the VME.  At present, there are some limiting
   assumptions that are enforced via exception tossing:
   - The base address is word aligned.
   - Even BLT's are now done as individual cycles.
   - The stack length must be at most 0xfff words long.

   \param handle - Handle received from the Open call.
   \param base   - The VME Base address for the transfer.  It is a requirement
                   in this version that (base & 0xfffffffe) == base
                   [base is word aligned].
   \param pBuffer - Buffer into which the data will be read.
   \param count   - number of words to transfer.

   \return int
   \retval  Number of bytes transferred.

   \throw   string - If the base address is not word aligned.
   \throw   string - If the stack length is too long.
   \throw   string - on an I/O error.
*/
int 
WienerUSBVMEInterface::atomicReadWords(void* handle, unsigned long base, void* pBuffer,
				 unsigned long count)
{
  pUnit pHandle = (pUnit)handle;
  usb_dev_handle* pDevice    = pHandle->s_pHandle;
  unsigned char   aModifier  = pHandle->s_AM;

  // Error checking: The base address must be longword aligned:
  // and count*STACK_SIZEPERREAD must be within 0xffc  (a safety fudge here).

  if ((base & 0xfffffffe) != base) {
    throw string("WienerUSBVMEInterface::atomicReadWords - address not word aligned");
  }
  if (count > STACK_MAXIMMEDIATE) {
    throw string("WienerUSBVMEInterface::atomicReadWords - transfer count too large");
  }
  // Now build the stack:

  unsigned long stack[STACK_MAXSIZE]; // this is overly large.
  stack[0]   = count*STACK_SIZEPERREAD+1; // Each xfer words+ header.
  unsigned long* pStack(&(stack[1]));

  for (int i =0; i<=count; i++) {
    *pStack++ = aModifier | MODE_READ;
    *pStack++ = base | ADDRESS_LWORD;
    base     += 2;		// Next address.
  }

  usbImmediateStackTransaction(pHandle, stack, pBuffer, count*sizeof(short));
  return count*sizeof(short);

}
/*!
   Read a block of words from the VME.  At present, there are some limiting
   assumptions that are enforced via exception tossing:
   - Even BLT's are now done as individual cycles.
   - The stack length must be at most 0xfff words long.

   \param handle - Handle received from the Open call.
   \param base   - The VME Base address for the transfer.
   \param pBuffer - Buffer into which the data will be read.
   \param count   - number of bytes to transfer.

   \return int
   \retval  Number of bytes transferred.

   \throw   string - If the stack length is too long.
   \throw   string - on an I/O error.
*/
int 
WienerUSBVMEInterface::atomicReadBytes(void* handle, unsigned long base, void* pBuffer,
		     unsigned long count)
{
  pUnit pHandle = (pUnit)handle;
  usb_dev_handle* pDevice    = pHandle->s_pHandle;
  unsigned char   aModifier  = pHandle->s_AM;
  unsigned short  inBuffer[STACK_MAXIMMEDIATE];

  if (count > STACK_MAXIMMEDIATE) {
    throw string("WienerUSBVMEInterface::atomicReadBytes - transfer count too large");
  }
  // Now build the stack:

  unsigned long stack[STACK_MAXSIZE]; // this is overly large.
  stack[0]   = count*STACK_SIZEPERREAD+1; // xfer words+ header.
  unsigned long* pStack(&(stack[1]));



  for (int i =0; i<=count; i++) {

    // The bottom  bit of the address determine the data strobes.

    unsigned short dstrobes;
    if ((base & 1) == 0) {
      dstrobes = MODE_NODS1;	// Low byte in the word.
    } else {
      dstrobes = MODE_NODS0;	// High byte in the word.
    }

    *pStack++ = aModifier | MODE_READ | dstrobes;
    *pStack++ = (base & 0xfffffffe) | ADDRESS_LWORD;
    base     += 1;		// Next address.
  }

  // The input buffer from the usb transaction will have one byte/word.

  usbImmediateStackTransaction(pHandle, stack, inBuffer, count*sizeof(short));
  unsigned char* p = (unsigned char*)pBuffer;
  for (int i =0; i < count; i ++) {
    *p++ = inBuffer[i];
  }
  return count*sizeof(char);

}
/*!
   Write a bunch of longwords to the VME bus.  At present, we operate
   under some limiting assumptions:
   -  The base address must be longword aligned.
   -  BLT AM's will still be done as individual transfers.
   -  The stack length must fit in the 0xfff stack length for
      direct operations.

  \param handle   - Handle returned from Open
  \param base     - Base address of the transfer.  This must be longword
                    aligned.
  \param pBuffer  - Buffer from which the longwords will be written...
                    note that this data must be copied into the stack.
  \param count    - Number of longwords to transfer

  \return int
  \retval Number of bytes transferred.

  \throw string - If the transfer address is not longword aligned.
  \throw string - If the transfer count is too large to do the transfer in one
                  hop.
  \throw ???    - Whatever exceptions usbImmediateStackTransaction may throw.
*/
int 
WienerUSBVMEInterface::atomicWriteLongs(void* handle, unsigned long base, void* pBuffer,
		      unsigned long count)
{
  // Extract the AM and usb_dev_handle from the handle:

  pUnit pHandle = (pUnit)handle;
  usb_dev_handle* pDevice    = pHandle->s_pHandle;
  unsigned char   aModifier  = pHandle->s_AM;

  unsigned long* pData = (unsigned long*) pBuffer;

  // Error checking (see the function header comments: 

  if ((base & 0xfffffffc) != base) {
    throw string("atomicWriteLongs - Base address is not longword aligned");
  }
  if (count > STACK_MAXIMMEDIATE) {
    throw string("atomicWriteLongs - Transfer count exceeds maximum");
  }
  // Now construct the stack.  Note that the data must be moved into the stack.

  unsigned long stack[STACK_MAXSIZE];
  unsigned char inputBuffer[DUMMYSIZE];	// for usbImmediateStackTransaction
  stack[0] = count*STACK_SIZEPERWRITE+1; // transfersize + header.
  unsigned long* pStack(&(stack[1]));
  
  for (int i =0; i < count; i++) {
    *pStack++ = aModifier;
    *pStack++ = base;
    *pStack++ = *pData++;
    
    base += 4;
  }

  usbImmediateStackTransaction(pHandle, stack, inputBuffer, sizeof(inputBuffer));
  return count*sizeof(long);
}
/*
   Write a bunch of words to the VME bus.  At present, we operate
   under some limiting assumptions:
   -  The base address must be word aligned.
   -  BLT AM's will still be done as individual transfers.
   -  The stack length must fit in the 0xfff stack length for
      direct operations.

  \param handle   - Handle returned from Open
  \param base     - Base address of the transfer.  This must be word
                    aligned.
  \param pBuffer  - Buffer from which the words will be written...
                    note that this data must be copied into the stack.
  \param count    - Number of words to transfer

  \return int
  \retval Number of bytes transferred.

  \throw string - If the transfer address is not word aligned.
  \throw string - If the transfer count is too large to do the transfer in one
                  hop.
  \throw ???    - Whatever exceptions usbImmediateStackTransaction may throw.
 */
int 
WienerUSBVMEInterface::atomicWriteWords(void* handle, unsigned long base, void* pBuffer,
		      unsigned long count)
{
  // Extract the AM and usb_dev_handle from the handle:

  pUnit pHandle = (pUnit)handle;
  usb_dev_handle* pDevice    = pHandle->s_pHandle;
  unsigned char   aModifier  = pHandle->s_AM;

  unsigned short* pData = (unsigned short*) pBuffer;

  // Error checking (see the function header comments: 

  if ((base & 0xfffffffe) != base) {
    throw string("atomicWriteWords - Base address is not word aligned");
  }
  if (count > STACK_MAXIMMEDIATE) {
    throw string("atomicWriteWords - Transfer count exceeds maximum");
  }
  // Now construct the stack.  Note that the data must be moved into the stack.

  unsigned long stack[STACK_MAXSIZE];
  unsigned char inputBuffer[DUMMYSIZE];	// for usbImmediateStackTransaction
  stack[0] = count*STACK_SIZEPERWRITE+1; // transfersize + header.
  unsigned long* pStack(&(stack[1]));
  
  for (int i =0; i < count; i++) {
    *pStack++ = aModifier; 
    *pStack++ = base | ADDRESS_LWORD;
    *pStack++ = *pData++;
    
    base += 2;
  }
  usbImmediateStackTransaction(pHandle, stack, inputBuffer, sizeof(inputBuffer));
  return count*sizeof(short);  
}
/*
   Write a bunch of bytes to the VME bus.  At present, we operate
   under some limiting assumptions:
   -  BLT AM's will still be done as individual transfers.
   -  The stack length must fit in the 0xfff stack length for
      direct operations.

  \param handle   - Handle returned from Open
  \param base     - Base address of the transfer.

  \param pBuffer  - Buffer from which the words will be written...
                    note that this data must be copied into the stack.
  \param count    - Number of words to transfer

  \return int
  \retval Number of bytes transferred.

  \throw string - If the transfer count is too large to do the transfer in one
                  hop.
  \throw ???    - Whatever exceptions usbImmediateStackTransaction may throw.
 */
int 
WienerUSBVMEInterface::atomicWriteBytes(void* handle, unsigned long base, void* pBuffer,
			       unsigned long count)
{
  // Extract the usb device handle and address modifier from the opaque handle.

  pUnit pHandle = (pUnit)handle;
  usb_dev_handle* pDevice    = pHandle->s_pHandle;
  unsigned char   aModifier  = pHandle->s_AM;

  char* pData = (char*)pBuffer; // Data must be copied to stack.

  // Error checking: The transfer must fit in the stack:

  if (count > STACK_MAXIMMEDIATE) {
    throw string("atomicWriteBytes: Transfer count exceeds maximum allowed");
  }
  // Now build the stack... Byte I/O is a bit different than what we've done
  // so far:  We need to worry about positioning the data in the correct
  // "bus lane" and we also have to make the data strobes do the right thing
  // depending on whether or not this is an odd or even transfer.

  unsigned long stack[STACK_MAXSIZE];
  unsigned char inputBuffer[DUMMYSIZE];
  stack[0] = count*STACK_SIZEPERWRITE+1;
  unsigned long* pStack(&(stack[1]));
  
  for (int i =0; i < count; i++) {
    unsigned short dstrobes;	// The DS0/DS1 mask.

    if ((base & 1) == 0) {	// Even bytes
      dstrobes  = MODE_NODS1;
    } 
    else {			// Odd bytes.
      dstrobes  = MODE_NODS0;
    }
    unsigned long datum = *pData++;
    *pStack++ = aModifier | dstrobes;
    *pStack++ = (base & 0xfffffffe) | ADDRESS_LWORD;
    *pStack++ = (datum);
    base++;
  }
  usbImmediateStackTransaction(pHandle, stack, inputBuffer, sizeof(inputBuffer));
  return count;
}
/////// Below this line are functions that are not atomic

/*!
   Read a bunch of longs.  The buffer must be longword aligned.
   \param handle - Handle received from the Open call.
   \param base   - The VME Base address for the transfer. 
   \param pBuffer - Buffer into which the data will be read.
   \param count   - number of longwords to transfer.

   \return int
   \retval  Number of bytes transferred.
*/
int 
WienerUSBVMEInterface::ReadLongs(void* handle, unsigned long base, void* pBuffer, 
		       unsigned long count)
{
  unsigned long* plBuffer = (unsigned long*)pBuffer;
  unsigned long  nRead(0);
  while (count) {
    if (count > STACK_MAXIMMEDIATE) {
      nRead    += atomicReadLongs(handle, base, plBuffer, STACK_MAXIMMEDIATE);
      base     += STACK_MAXIMMEDIATE*sizeof(unsigned long);
      plBuffer += STACK_MAXIMMEDIATE;
      count    -= STACK_MAXIMMEDIATE;
    } 
    else {			// Final read...
      nRead    += atomicReadLongs(handle, base, plBuffer, count);
      return nRead;
    }
  }
}
/*!
   Read a bunch of words.  The buffer must be word  aligned.
   \param handle - Handle received from the Open call.
   \param base   - The VME Base address for the transfer. 
   \param pBuffer - Buffer into which the data will be read.
   \param count   - number of longwords to transfer.

   \return int
   \retval  Number of bytes transferred.
*/
int 
WienerUSBVMEInterface::ReadWords(void* handle, unsigned long base, void* pBuffer,
				 unsigned long count)
{
  unsigned short* pwBuffer = (unsigned short*)pBuffer;
  unsigned long   nRead(0);

  while (count) {
    if (count > STACK_MAXIMMEDIATE) {
      nRead    += atomicReadWords(handle, base, pwBuffer, STACK_MAXIMMEDIATE);
      base     += STACK_MAXIMMEDIATE*sizeof(unsigned short);
      pwBuffer += STACK_MAXIMMEDIATE;
      count    -= STACK_MAXIMMEDIATE;
    } 
    else {			// Final read...
      nRead    += atomicReadWords(handle, base, pwBuffer, count);
      return nRead;
    }
  }

}
/*!   
    Read a bunch of bytes:
   \param handle - Handle received from the Open call.
   \param base   - The VME Base address for the transfer. 
   \param pBuffer - Buffer into which the data will be read.
   \param count   - number of longwords to transfer.

   \return int
   \retval  Number of bytes transferred.
*/
int 
WienerUSBVMEInterface::ReadBytes(void* handle, unsigned long base, void* pBuffer,
				 unsigned long count)
{
  unsigned char*   pbBuffer = (unsigned char*)pBuffer;
  unsigned long    nRead(0);

  while (count) {
    if (count > STACK_MAXIMMEDIATE) {
      nRead    += atomicReadBytes(handle, base, pbBuffer, STACK_MAXIMMEDIATE);
      base     += STACK_MAXIMMEDIATE*sizeof(unsigned char);
      pbBuffer += STACK_MAXIMMEDIATE;
      count    -= STACK_MAXIMMEDIATE;
    } 
    else {			// Final read...
      nRead    += atomicReadBytes(handle, base, pbBuffer, count);
      return nRead;
    }
  }
}
/*!
   Write a bunch of longwords to the VME bus.  The buffer must be longword
   aligned.

   \param handle   - Handle received from the Open call.
   \param base     - VME base address for the transfer.
   \param pBuffer  - Pointer to the buffer that holds the data to write.
   \param count    - Number of longs to write.
*/
int 
WienerUSBVMEInterface::WriteLongs(void* handle, unsigned long base, void* pBuffer,
				  unsigned long count)
{
  unsigned long* plBuffer = (unsigned long*)pBuffer;
  unsigned long  nWritten(0);

  while(count) {
    if (count > STACK_MAXIMMEDIATE) {
      nWritten   += atomicWriteLongs(handle, base, plBuffer,
				     STACK_MAXIMMEDIATE);
      base       += STACK_MAXIMMEDIATE*sizeof(long);
      plBuffer   += STACK_MAXIMMEDIATE;
      count      -= STACK_MAXIMMEDIATE;
    }
    else {
      return nWritten   += atomicWriteLongs(handle, base, plBuffer, count);
    }
  }
}
/*!
     Write a bunch of words to the VME.  The buffer must be word aligned.

   \param handle   - Handle received from the Open call.
   \param base     - VME base address for the transfer.
   \param pBuffer  - Pointer to the buffer that holds the data to write.
   \param count    - Number of longs to write.

 */
int 
WienerUSBVMEInterface::WriteWords(void* handle, unsigned long base, void* pBuffer,
				  unsigned long count)
{
  unsigned short* pwBuffer = (unsigned short*)pBuffer;
  unsigned long  nWritten(0);

  while(count) {
    if (count > STACK_MAXIMMEDIATE) {
      nWritten   += atomicWriteWords(handle, base, pwBuffer,
				     STACK_MAXIMMEDIATE);
      base       += STACK_MAXIMMEDIATE*sizeof(short);
      pwBuffer   += STACK_MAXIMMEDIATE;
      count      -= STACK_MAXIMMEDIATE;
    }
    else {
      return nWritten   += atomicWriteWords(handle, base, pwBuffer, count);
    }
  }
}
/*!
    Write a bunch of byts to the VME.

   \param handle   - Handle received from the Open call.
   \param base     - VME base address for the transfer.
   \param pBuffer  - Pointer to the buffer that holds the data to write.
   \param count    - Number of longs to write.

*/
int 
WienerUSBVMEInterface::WriteBytes(void* handle, unsigned long base, void* pBuffer,
				  unsigned long count)
{
  unsigned char* pwBuffer = (unsigned char*)pBuffer;
  unsigned long  nWritten(0);

  while(count) {
    if (count > STACK_MAXIMMEDIATE) {
      nWritten   += atomicWriteBytes(handle, base, pwBuffer,
				     STACK_MAXIMMEDIATE);
      base       += STACK_MAXIMMEDIATE*sizeof(char);
      pwBuffer   += STACK_MAXIMMEDIATE;
      count      -= STACK_MAXIMMEDIATE;
    }
    else {
      return nWritten   += atomicWriteBytes(handle, base, pwBuffer, count);
    }
  }
}

////////////////////  Utilities.

/*!
  Peforms a USB immediate stack transaction.  Immediate stack transactions
  are VME sequencer commands that are performed >now<.
  Note that due to the command packet structure, it is necessary to copy the
  stack prior to performing the transaction.

  Note that this function will do a claim and release on the usb interface
  in order to function politely with other programs that may use this device...
  although in the NSCLDAQ, the locking discipline (via CVMEInterface::Lock)
  should usually be sufficient to prevent contention.
  
  \param handle - Handle received from the Open call.
  \param stack  - The immediate stack of transactions to perform.
                  (note that the stack size in words is in the first long of
		  the stack).
  \param inputBuffer - The input buffer to receive any data returned from the
                  VME bus (for e.g. read transactions).
  \param readSize - The number of bytes the input buffer can hold.

  \return int
  \retval Number of bytes read into the input buffer.

  \throw string - For any usb I/O errors.

  \note If the usb interface is already claimed, we'll try to claim it
        periodically until we succeed.
*/
int
WienerUSBVMEInterface::usbImmediateStackTransaction(void* handle,
						    void* stack,
						    void* inputBuffer,
						    unsigned long readSize)
{
  // Extract the usb_dev_handle:

  pUnit pHandle = (pUnit)handle;
  usb_dev_handle* pDevice    = pHandle->s_pHandle;
  unsigned long*  pStack = (unsigned long*)stack;

  // Attempt to claim the interface, retry every USB_CLAIM_RETRYDELAY us if
  // we fail:
  //    Note that all examples I can find (few enough), are always
  //    claiming interface 0.... seems to me this may not be strictly speaking
  //    correct, but I'm not sure what is correct... so we'll do what the rest
  //    of the world does.
  //
#ifdef CLAIMUSB
  while(1) {
    int status = usb_claim_interface(pDevice, 0);
    if (status >= 0) break;	// Got it.
    if (status == -EBUSY) {
      usleep(USB_CLAIM_RETRYDELAY);
    } 
    else {
     string msg("usbImmediateStackTransaction: Failed to claim interface: ");
     msg += strerror(-status);
     throw msg;
    }
  }
#endif
  // Construct the outpacket and send it...

  int nOutWords = pStack[0] + 2; // Stack count not self inclusive.
  unsigned short* pOutPacket = new unsigned short[nOutWords];
  unsigned short* pOutData   = pOutPacket;
  *pOutData++   = TA_VCG | REQ_DATA;     // First word of outblock is target address.
  memcpy(pOutData, pStack, 
	 (pStack[0]+1)*sizeof(unsigned short));	// Assumes host is little endian

  int nWritten = usb_bulk_write(pDevice, ENDPOINT_OUT,
				(char*)pOutPacket, nOutWords*sizeof(unsigned short),
				WRITE_TIMEOUT);
  if (nWritten < 0) {
#ifdef CLAIMUSB
    usb_release_interface(pDevice, 0);
#endif
    throw string("usbImmediateStackTransaction - usb_bulk_write failed");
  }
  

  // Read the in packet...we don't read directly to the user buffer to ensure
  // the fifo gets emptied even if the user screws up the read count.

  char inPacket[DUMMYSIZE];
  int nRead = usb_bulk_read(pDevice, ENDPOINT_IN,
			    inPacket, sizeof(inPacket), READ_TIMEOUT);

  if (nRead < 0) {
#ifdef CLAIMUSB
    usb_release_interface(pDevice, 0);
#endif
    throw string("usbImmediateStackTransaction- usb_bulk_read failed");
  }
  // Copy any input data -> inputBuffer... at most readSize bytes.

  memcpy(inputBuffer, inPacket,
	 (nRead < readSize) ? nRead : readSize);

  // Release the interface
#ifdef CLAIMUSB
  usb_release_interface(pDevice, 0);
#endif

  // Return the result... always nRead so the user knows if they missed something.

  return nRead;
}
