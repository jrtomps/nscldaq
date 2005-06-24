/*!
   Implements the VME interface for a Wiener PCI/VME bus bridge.
   note that this device driver is not capable of the mmap(2) call
   Therefore the Map and Unmap calls wil throw an exception.
   The read and write calls will be augmented by Wiener specific
   calls that support Reads and writes of specific widths (e.g.
   read a set of longs etc.).

*/

/*
   Change log:
   $Log$
   Revision 8.2  2005/06/24 11:32:40  ron-fox
   Bring the entire world onto the 8.2 line

   Revision 4.1  2004/11/08 19:38:13  ron-fox
   Add the Wiener api to the build.

*/

#include <config.h>
#include "CVMEInterface.h"
#include "WienerAPI.h"
#include <pcivme_ni.h>
#include <errno.h>
#include <string>
#include <stdio.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

const char* CVMEInterface:: m_szDriverName="WienerPCIVME";

// The struct below defines a mapping between an address mode
// enum and the corresponding VME address modifier:
//
typedef struct _ModifierMap {
  enum CVMEInterface::AddressMode m_Mode;
  unsigned char    m_Modifier;
} ModifierMap;

static ModifierMap ModifierMappings[6] = 
{
  {CVMEInterface::A16, 0x2d},
  {CVMEInterface::A24, 0x3d},
  {CVMEInterface::A32, 0x0d},

  {CVMEInterface::GEO, 0x2f},
  {CVMEInterface::MCST,0x0d},
  {CVMEInterface::CBLT,0x0b}
};
static int numModifiers = sizeof(ModifierMappings)/sizeof(ModifierMap);

/*!
   converts a crate number into a device name suitable
   for VMEOpen.  These devicenames are of the form
   vmemm_n where n is the crate number
   \param nCrate (unsigned short [in]):
      Number of the vme crate we want a device for.
   \return string
       Name of the device to open.

*/
static string
CrateToDevice(unsigned short nCrate) 
{
   char device[100];               // Should be big enough.
   sprintf(device, "/dev/vmemm_%d", nCrate+1);
   return string(device);
}
/*!
   Map an AddressMode enum into the corresponding VME 
   address Modfier.
   \param mode [CVMEInterface::AddressMode [in]):
      Enumerated mode velu.
   \return unsigned char 
       The corresponding address modifier.
   \throw string - reason for failure.
*/
static unsigned char
ConvertModifier(CVMEInterface::AddressMode mode)
{
  if((mode < 0) || (mode >= CVMEInterface::LAST)) {
    throw string("Invalid address modifier parameter ConvertModifier");
  }
  ModifierMap* pEntry(ModifierMappings);
  for(int i =0; i < numModifiers; i++) {
    if(mode == pEntry->m_Mode) {
         return pEntry->m_Modifier;
    }
     pEntry++;
  }
  throw string("Invalid address modifier parameter ConvertModifier");
}

/*! 

   Opens the crate.  Note that at present, only a single vme crate
   is supported.  

   \param nMode (AddressMode [in]): 
     Selects the address modifier used to access the crate via this
     fd.
    \param crate (Unsigned short [in]):
       Number of the crate to open
       
     \throw string
        A descriptive string on error.
*/

void*
CVMEInterface::Open(CVMEInterface::AddressMode nMode,
		    unsigned short crate)
{
  //  Convert the modifier selector into a VME bus modifier.

  unsigned char ubAddressModifier = ConvertModifier(nMode);

  // Open and return the result as a pointer to a new'd int.


  string Devicename = CrateToDevice(crate);
  int fd;
  int err = VMEopen(Devicename.c_str(),  
                    ubAddressModifier,  &fd);

  // err is errno if there's a problem.

  if(err) {
    throw string(strerror(err));
  }

  // Reset sysfail (on by default after power up:):

  VMEsysfailSet(fd, FALSE);
  int* pFd = new int;
  *pFd = fd;
  return (void*)pFd;

}
/*!
    Closes the crate.  The pointer to the file descriptor
   is deleted after the close call.
   \param pDevice (void* [in]):
      Pointer to a device handle returned from 
      CVMEInterface::Open.
   \throw string
       string descriptive of the reason an exception needed
      to be thrown.
*/
void
CVMEInterface::Close(void* pDeviceHandle)
{
   int* pHandle((int*)pDeviceHandle);
   int fd = *pHandle;
   VMEclose(fd);
   delete pHandle;
}
/*!
    Map - attempts to map a VME bus segment into the
   caller's process virtual address space.
   Note that at the time of writing this, this is not supported
   for the Wiener driver and therefore results in an exception.
   \param pDeviceHandle (void* [in]):
      Handle returned from a call to CVMEInterface::Open.
   \param nBase (unsigned long [in]):
      Base address of the map in VME address space.
   \param nBytes (unsigned long [in]):
      Number of bytes in the region mapped.
   \return void*
     Pointer to the new process VA list.
   \throw string
      A string indicating this operation is not supported.

*/
void*
CVMEInterface::Map(void* pDeviceHandle, unsigned long nBase,
                  unsigned long nBytes)
{
   throw string("The Wiener interface does not support Map");

}
/*!
   Unmap the a virtual map to the vme crate.  Note that at present,
   the driver does not support mmap.  Therefore we will throw
   a string exception explaining this.
   \param pDeviceHandle (void* [in]): 
      Device handle returned from CVMEInterface::Open.
   \param pBase         (void* [in]):
      Pointer to the base of the map in process virtual address
      space (e.g. the return value from a successful 
      CVMEInterface::Map call.
   \param nBytes        (unsigned long [in]):
      Number of bytes in the extent of the map.

   \throw string
      Indicating that maping is not supported by the Wiener
      device driver.
*/
void
CVMEInterface::Unmap(void* pDeviceHandle,
                     void* pBase,
                     unsigned long nBytes)
{
   throw string("Map/Unmap not supported by WIENER interface");
}
/*!
   Read a block of memory from the VME bus.  This maps to a
   call to VMERead with byte width.  Note that there are 
   interface specific functions that force the width
   to be wider than a byte.
   \param pDeviceHandle (void* [in]):
      Device handle returned from a call to 
      CVMEInterface::Open
   \param nOffset (unsigned long [in]):
      Base address in VME space at which the read will start.
   \param pBuffer (void* [out]):
      Pointer to the buffer into which the read is done.
   \param nBytes (unsigned long [in]):
      Number of bytes to read.
   \return int
      Number of bytes actually read.
   \throw string
      In case of failure a descriptive string is thrown.
*/
int
CVMEInterface::Read(void* pDeviceHandle,
                    unsigned long nOffset,
                    void* pBuffer, unsigned long nBytes)
{
   int* pFd((int*)pDeviceHandle);
   return VMEread(*pFd, nOffset, sizeof(char), 
                  nBytes, pBuffer);
}
/*!
   Writes byte wide to a section of VME address space.
   For other width writes, see the device specific functions.
   \param pDeviceHandle (void* [in]):
      Device handle from CVMEInterface::Open
   \param nOffset (unsigned long [in]):
      VME address of block of VME space to write to.
   \param pBuffer (void* [in]):
      Source of data to write.
   \param nBytes (unsigned long nBytes [in]):
      Number of bytes we want to write.
   \return int
      Number of bytes written.
*/
int
CVMEInterface::Write(void* pDeviceHandle,
                     unsigned long nOffset,
                     void* pBuffer, unsigned long nBytes)
{
   int* pFd((int*)pDeviceHandle);
   return VMEwrite(*pFd, nOffset, sizeof(char), 
                   nBytes, pBuffer);
}
/*!
	Performs a VME bus reset.
   \param pHandle (void* [in]):
      Handle open on the device (returned from 
      VMEInterface::Open
*/
void
WienerVMEInterface::ResetVme(void* pHandle)
{
   int* pFd((int*)pHandle);
   VMEreset(*pFd);
}

// Data Transfer operations:

/*
   Read word length entities from the VME bus.
   \param pHandle (void* [in]):
      Handle openo n the file.  This is a return value
      from CVMEInterface::Open
   \param nBase (unsigned long [in]):
      Base of region in VME from which the data will be read.   
   \param pBuffer (void* [out]):
      Pointer to the place where data will be stored.
   \param nWords (unsigned long [in]):
      Number of {\em words} of data to read.
   \return int
      Number of {\em words} of data actually read.

*/
int  
WienerVMEInterface::ReadWords(void* pHandle,
                               unsigned long nBase,
                               void* pBuffer,
                               unsigned long nWords)
{
   int* pFd((int*)pHandle);
   return VMEread(*pFd, nBase, sizeof(short), nWords,   
		  pBuffer);
}
/*!
    Write word length entities to the VME bus.
   \param pHandle (void* [in]):
      Handle open on the file.  This is a return value
      from CVMEInterface::Open
   \param nBase (unsigned long [in]):
      Base of region in VME to which the data will be read.   
   \param pBuffer (void* [out]):
      Pointer to the place where data will transferred from.
   \param nWords (unsigned long [in]):
      Number of {\em words} of data to write
   \return int
      Number of {\em words} of data actually written.

*/
int 
WienerVMEInterface::WriteWords(void* pHandle,
                               unsigned long nBase,
                               void* pBuffer,
                               unsigned long nWords)
{
   int* pFd((int*) pHandle);
   return VMEwrite(*pFd, nBase, sizeof(short), nWords,
           pBuffer);
}

   // Long sized transfer operations:
/*!
    Read long word entities to a contiguous block of
    vme address space:
  \param pHandle (void* [in]):
      Handle open on the file.  This is a return value
      from CVMEInterface::Open
   \param nBase (unsigned long [in]):
      Base of region in VME from which the data will be read.   
   \param pBuffer (void* [out]):
      Pointer to the place where data will be stored.
   \param nLongs (unsigned long [in]):
      Number of {\em longs} of data to read.
   \return int
      Number of {\em longs} of data actually read.
*/
int 
WienerVMEInterface::ReadLongs(void* pHandle,
                              unsigned long nBase,
                              void* pBuffer,
                              unsigned long nLongs)
{
   int* pFd((int*)pHandle);
   
   return VMEread(*pFd, nBase, sizeof(long), nLongs,
		  pBuffer);
}
/*!
   Write longword items to a contiguous block of VME memory.
   \param pHandle (void* [in]):
      Handle open on the file.  This is a return value
      from CVMEInterface::Open
   \param nBase (unsigned long [in]):
      Base of region in VME to which the data will be read.   
   \param pBuffer (void* [out]):
      Pointer to the place where data will transferred from.
   \param nLongs (unsigned long [in]):
      Number of {\em longs} of data to write
   \return int
      Number of {\em longs} of data actually written.

*/
int 
WienerVMEInterface::WriteLongs(void* pHandle,
                               unsigned long nBase,
                               void* pBuffer,
                               unsigned long nLongs)
{
   int* pFd((int*)pHandle);
   return VMEwrite(*pFd, nBase, sizeof(long), nLongs,
		   pBuffer);
}
