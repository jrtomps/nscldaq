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

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <assert.h>

#include <iostream>

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

typedef struct usb_device *usb_device_ptr;

// In some cases we need to define the semun union:

#if defined(__GNU_LIBRARY__) && !(defined(_SEM_SEMUN_UNDEFINED))
#else
union semun {
  int                val;	// SETVAL value.
  struct semid_ds    *buf;	// IPC_STAT, IPC_SET buffer.
  unsigned short int *array;	// Array for GETALL/SETALL
  struct seminfo*    _buf;	// Buffer for IPC_INFO
};
#endif




// Unit defines an 'open' vme device.  In this case the open model
// is that of opening an address space (modifier) in a VME crate.
// Since each usb unit is capable of carrying any address modifier,
// The AM is computed and carried with the Unit structure.

typedef struct _Unit {
  unsigned short   s_Crate;    // Number of the crate.
  unsigned char     s_AM;	// VME Address modifier.
} Unit, *pUnit;


// Modifier map defines a mapping between an AddressMode and a
// vme bus address modifier:
//
typedef struct _ModifierMap {
  enum CVMEInterface::AddressMode s_Mode;
  unsigned char                   s_Modifier;  
  
} ModifierMap, *pModifierMap;


// Due to the funky way in which we have to handle usb_claim_interface,
// usb_release_interface and its interaction with usb_open/usb_close,
// We will actually do our opens in the Lock and closes in the
// Unlock functions.  Therefore we need to keep track of which
// devices the user is actually using (has 'open').
// This is done with the following structure:
// 
typedef struct _CrateInfo {
  usb_device_ptr   s_pDevice;	// Pointer to the device.
  usb_dev_handle*  s_pHandle;   // Pointer to the actual open handle.
  unsigned int     s_nOpens;	// Number of user opens on the crate.
} CrateInfo, *CrateInfoPtr;


// The usb bus is a dynamic entity.  On the first open we
// will probe that bus enumerating the usb interfaces that are present.
// the assumption is that the vme crates will not by 'heavily' dynamic
// but will only slowly vary, and will remain the same for the life of a program.
//

static bool                 initialized(false);
static std::vector<CrateInfoPtr>  vmeCrates;	// Array of crates we found.


/*!
   This file implements coarse grained VME locking.
   If applications use this, the entire VME subsystem will be controlled
   by a single lock.  
*/

static int semid = -1;		// This will be the id of the locking semaphore
static int semkey= 0x564d4520;  // "VME " :-).


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


// For debugging the API's initial libusb accesses you can modify this..
//  or call WienerUSBVMEInterface::setDebug prior to the first
// open call.
static   unsigned debug_level(0);



// Debugging hex dump of words:

static void hexdumpw(void* p, unsigned nWords) {
  unsigned short* pw = (unsigned short*)p;
  for(int i =0; i < nWords; i++) {
    if ((i % 8) == 0) {
      cerr << endl;
    }
    cerr << hex << " " << *pw << dec;
    pw++;
  }
  cerr << endl << endl;
  cerr.flush();
}


/*!
   This internal function is used to establish the semaphore:
   - If the semaphore exists, it's id is just stored in semid.
   - If the semaphore does not exist we try to create it O_EXCL
     this is an attempt to deal with any timing holes that may
     occur when two programs simultaneously attemp to create the semaphore.
   - If the O_EXCL creation succeeds (semaphore does not exist), it is
     given in initial value of 1 so that a single process can pass the lock
     gate.
   - If the O_EXCL creation fails, the process backs off for a while and
     then does a semget for an existing  semaphore again assuming that on the 
     second try, all initialization has been complete.

   \throw   String
      If an error occured on any of the system calls.

*/
static void
AttachSemaphore()
{
  // Retry loop in case anybody makes and then kills it:

  while(1) {
    // Try to get the id of an existing semaphore:

    semid = semget(semkey, 0, 0777); // Try to map:
    if(semid >= 0) break;	     // Previously existing!!
    if(errno != ENOENT) {
      throw 
	string("AttachSemaphore - semget error unexpected");
    }
    // Sempahore does not exist.  Try to be the only guy to 
    // create it:

    semid = semget(semkey, 1, 0777 | IPC_CREAT | IPC_EXCL);
    if(semid >= 0) {
      // We're the creator... initialize the sempahore, and return.

      union semun data;
      data.val = 1;

      int istat = semctl(semid, 0, SETVAL, data); // Allow 1 holder
      if(istat < 0) {
	throw string("AttachSemaphore - semctl error unexpected");
      }

      break;
    }
    if(errno != EEXIST) {
      throw
	string("AttachSemaphore - semget error unexpected");
    }
    //   The semaphore popped into being between the initial try
    //   to just attach it and our try to create it.
    //   The next semget should work, but we want to give
    //   the creator a chance to initialize the semaphore so
    //   we don't try to take out a lock on the semaphore before
    //   it is completely initialized:

    sleep(1);
  }
  return;
}

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
  usb_set_debug(debug_level);
  if(usb_find_busses() < 0) {
    throw string("Failed to find usb busses");
  }
  if(usb_find_devices() < 0) {
    std::cerr << "throwing usb_find_devices failure\n";
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
	CrateInfoPtr info = new CrateInfo;
	info->s_pDevice = aDevice;
	info->s_pHandle = (usb_dev_handle*)NULL;
	info->s_nOpens  = 0;

	vmeCrates.push_back(info);
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
    Lock the semaphore.  If the semid is -1, the
    semaphore is created first.  

    \throw string
       - Really from AttachSemaphore
       - From failures in semop.
*/
void 
CVMEInterface::Lock() 
{
  // If necessary, get the semaphore id..

  if (semid == -1) AttachSemaphore();
  assert(semid >= 0);		// Otherwise attach.. throws.

  struct sembuf buf;
  buf.sem_num = 0;		// Only one semaphore.
  buf.sem_op  = -1;		// Want to take the semaphore.
  buf.sem_flg = SEM_UNDO;	// For process exit.

  while (1) {			// In case of signal...
    int stat = semop(semid, &buf, 1);

    if(stat == 0) break;

    if(errno != EINTR) {	// Bad errno:
      throw string("CVMEInterface::Lock semop gave bad status");
    }
    // On EINTR try again.
  }
  // Now that I've locked the interface, I can open
  // and claim all the crates:

  unsigned int nCrates = vmeCrates.size();
  for (int i =0; i < nCrates; i++) {
    CrateInfoPtr p = vmeCrates[i];
    if(p->s_nOpens > 0) {
      p->s_pHandle = usb_open(p->s_pDevice);
      if(p->s_pHandle) {
	usb_claim_interface(p->s_pHandle, 0);
	usleep(150);		// Seemed to need this earlier.
      } else {
	cerr << "CVMEInterface::Lock: Failed to open crate " << i << endl;
	throw "Crate open failed in CVMEInterface::Lock()";
      }
    }
  }
  return;
}
/*!
  Unlock the semaphore. It is a crime to unlock the semaphore if it doesn
  not yet exist, since that would be unlocking a semaphore that is not yet
  locked.
  
  \throw  string
     If the semop operation produced an error.
  \throw string
     If the semaphore did not yet exist.
*/
void
CVMEInterface::Unlock()
{
  if(semid == -1) {
    throw string("Attempt to unlock the semaphore before it was created");
  }

  // Take all open interfaces and release/close them.
  // We put in a bit of delay since superstition currently says
  // they may be needed.
  //

  unsigned int nCrates = vmeCrates.size();
  for (int i=0; i < nCrates; i++) {
    CrateInfoPtr p = vmeCrates[i];
    if(p->s_pHandle) {
      usb_release_interface(p->s_pHandle, 0);
      usleep(150);
      usb_close(p->s_pHandle);
      usleep(150);
      p->s_pHandle = (usb_dev_handle*)NULL;
    }
  }
  struct sembuf buf;
  buf.sem_num = 0;
  buf.sem_op  = 1;
  buf.sem_flg= SEM_UNDO;	// Undoes the locking undo.

  while(1) {			// IN case of signal though not likely.
    int stat = semop(semid, &buf, 1);
    if(stat == 0) break;	// Got the job done!!
    if(errno != EINTR) {
      throw string("CVMEInterface::Unlock semop gave bad status");
    }
    // on EINTR try again.
  }
  return;
}

/*!
   Public interface to 'open' a vme crate.  
   Create a unit structure and mark the crate open.
   Due to the way that usb_claim_interface/usb_release_interface
   interact with usb_open/usb_close, it is not possible to 
   do the usb_open here.  We require lock which will open all
   crates and claim them.. The unlock will release and close them.
   The user will get a pointer to an opaque type that contains
   the address modifier and the crate number.

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

  // All we do is mark the user interested in the
  // crate.. the open/claim will get done in Lock
  // and the close in Unlock.
  //

  vmeCrates[crate]->s_nOpens++;

  // Create the unit, fill it in and hand it to the caller.
  // There's a tacit assumption that multiple opens can be done on the
  // same 'device'.

  pUnit unit       = new Unit;
  unit->s_Crate    = crate;
  unit->s_AM       = AM;

  return (void*) unit;
}
/*!
   Close a VME unit.
   - The Unit structure allocated for the device is destroyed.
   - The open count in the vme crate is decremented so that
     when the last instance of a Unit pointing to a crate is
     closed, Lock will no longer open/claim the unit.

   \param handle
       The opaque device handle gotten from the Open call.

*/
void
CVMEInterface::Close(void* handle)
{
  pUnit p = static_cast<pUnit>(handle);

  // Actual opens and closes are done by the
  // Lock/Unlock functions.  All we do
  // is destroy the user's handle and decrement
  // the count that keeps track of how many times
  // the user opened this vme crate.

  vmeCrates[p->s_Crate]->s_nOpens--;
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

/*!  
   Read is really readlongs... at present only a full number of long word
   transfers is supported.
*/
int
CVMEInterface::Read(void* pDeviceHandle,
		    unsigned long nOffset,
		    void*  pBuffer,
		    unsigned long nBytes)
{
  return WienerUSBVMEInterface::ReadLongs(pDeviceHandle,
					  nOffset,
					  pBuffer,
					  nBytes/sizeof(long));
}

/*!  
   Write is really writelongs... at present only a full number of long word
   transfers is supported.
*/
int
CVMEInterface::Write(void* pDeviceHandle,
		     unsigned long nOffset,
		     void* pBuffer,
		     unsigned long nBytes)
{
  return WienerUSBVMEInterface::WriteLongs(pDeviceHandle,
					   nOffset,
					   pBuffer,
					   nBytes/sizeof(long));
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
  unsigned char   aModifier  = pHandle->s_AM;
  unsigned short  inBuffer[STACK_MAXIMMEDIATE];

  if (count > STACK_MAXIMMEDIATE) {
    throw string("WienerUSBVMEInterface::atomicReadBytes - transfer count too large");
  }
  // Now build the stack:

  unsigned long stack[STACK_MAXSIZE]; // this is overly large.
  stack[0]   = count*STACK_SIZEPERREAD+1; // xfer words+ header.
  unsigned long* pStack(&(stack[1]));



  for (int i =0; i<count; i++) {

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


  pUnit pHandle = (pUnit)handle;
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
  // Extract the AM and 

  pUnit pHandle = (pUnit)handle;
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
  // Extract the  address modifier from the opaque handle.

  pUnit pHandle = (pUnit)handle;
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
    unsigned long datum = *pData++;

    if ((base & 1) == 0) {	// Even bytes
      dstrobes  = MODE_NODS1;
    } 
    else {			// Odd bytes.
      dstrobes  = MODE_NODS0;
    }
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
  usb_dev_handle* pDevice    = vmeCrates[pHandle->s_Crate]->s_pHandle;
  unsigned long*  pStack = (unsigned long*)stack;
  
  // It's an error to try to access the device when
  // we haven't locked the VME interface since the
  // USB is inherently un-shareable.

  if(!pDevice) {
    cerr << "Vme crate " << pHandle->s_Crate << " accessed without lock!\n";
    throw "usbImmediateStackTransaction - called with unlocked interface";
  }


  // Construct the outpacket and send it...

  int nOutWords = pStack[0] + 2; // Stack count not self inclusive.
  unsigned short* pOutPacket = new unsigned short[nOutWords];
  unsigned short* pOutData   = pOutPacket;
  *pOutData++   = TA_VCG | REQ_DATA;     // First word of outblock is target address.
  memcpy(pOutData, pStack, 
	 (pStack[0]+1)*sizeof(unsigned short));	// Assumes host is little endian

  if(debug_level) {
    hexdumpw(pOutPacket, nOutWords);
  }
  int nWritten = usb_bulk_write(pDevice, ENDPOINT_OUT,
				(char*)pOutPacket, nOutWords*sizeof(unsigned short),
				WRITE_TIMEOUT);
  if (nWritten < 0) {
    throw string("usbImmediateStackTransaction - usb_bulk_write failed");
  }
  

  // Read the in packet...we don't read directly to the user buffer to ensure
  // the fifo gets emptied even if the user screws up the read count.

  char inPacket[DUMMYSIZE];
  int nRead = usb_bulk_read(pDevice, ENDPOINT_IN,
			    inPacket, sizeof(inPacket), READ_TIMEOUT);

  if (nRead < 0) {
    throw string("usbImmediateStackTransaction- usb_bulk_read failed");
  }
  // Copy any input data -> inputBuffer... at most readSize bytes.
  if(debug_level) {
    cerr << "Read " << nRead << " bytes\n";
    hexdumpw(inPacket, nRead/sizeof(short));
    
  }
  
  memcpy(inputBuffer, inPacket,
	 (nRead < readSize) ? nRead : readSize);



  // Return the result... always nRead so the user knows if they missed something.

  return nRead;
}
/*!
   Set the debug level.
   If we have not yet been initialized, this just sets the
   debug_level variable.  Otherwise, usb_set_debug is called.

   \param level  - the new debug level for the usb library.

*/
void
WienerUSBVMEInterface::setDebug(int level) {
  debug_level = level;
  if(initialized) {
    usb_set_debug(level);
  }
}
