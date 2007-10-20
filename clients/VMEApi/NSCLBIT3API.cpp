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


#include "CVMEInterface.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <sys/ioctl.h>
#include <vmehb_ioctl.h>

const char* CVMEInterface::m_szDriverName="NSCLBIT3";

/*  Local data structures:
 */

// Translating address spaces to devices:

struct DeviceEntry {
  CVMEInterface::AddressMode s_eModeId;
  const char*                s_pszDeviceName;

};

static const DeviceEntry kaDeviceTable[] = {
  {CVMEInterface::A16, "/dev/vme16d32"},
  {CVMEInterface::A24, "/dev/vme24d32"},
  {CVMEInterface::A32, "/dev/vme32d32"},
  {CVMEInterface::GEO, "/dev/vmegeo24"},
  {CVMEInterface::MCST,"/dev/vmemca32"},
  {CVMEInterface::CBLT,"/dev/vmecba32"}
};
static const unsigned int knDeviceTableSize = sizeof(kaDeviceTable)/
                                              sizeof(DeviceEntry);

// Rounding regions:

struct AlignedRegion {
  unsigned long s_lAlignedBase;	// Base after alignment.
  unsigned long s_lAlignedSize;	// Size after alignment.
  unsigned long s_lUserOffset;	// User offset into aligned region.
};

/*!
   Utility function to translate a device selector to a
   device name:
   \param eAmode - AddressMode [in] The address mode to convert.
   
   \return The device name string corresponding to that mode.

   \throw message string if no match.
*/
static const char* Copyright=
ModeToDevice(CVMEInterface::AddressMode eAmode) 
{
  for(int i =0; i < knDeviceTableSize; i++) {
    if(kaDeviceTable[i].s_eModeId == eAmode) {
      return kaDeviceTable[i].s_pszDeviceName;
    }
  }
  throw string("CVMEInterface[NSCLBit3] No such address mode");
}
/*!
   Utility function to align a mapping specification to page
   boundaries.

   Parameters:
   \param pAligned - AlignedRegion* [out] - Returns the
                          specifications of the aligned region.
   \param lBase    - unsigned long [in] - User requested region
                           base.
   \param lSize    - unsigned long [in] - User requested region
                           size.
*/
void
AlignRegion(AlignedRegion* pAligned, 
	    unsigned long lBase, unsigned long lSize)
{
  // An address can be thought of as page selector bits and
  // offset bits.  This section of code creates masks for both of 
  // them.

  size_t nPagesize = getpagesize();
  size_t nOffsetMask = nPagesize-1;  // Mask of page offset bits.
  size_t nPageMask   = ~nOffsetMask; // Mask of page number bits.
 
  // Figure out the top and bottom addresses of the region:

  pAligned->s_lAlignedBase = lBase & nPageMask;
  unsigned long lTop       = (lBase + lSize - 1) | nOffsetMask;
  
  // Now we have all we need to fill in the blanks:

  pAligned->s_lAlignedSize = lTop - pAligned->s_lAlignedBase + 1;
  pAligned->s_lUserOffset  = lBase - pAligned->s_lAlignedBase;

}
/*!
  Open a handle on the device.  The handle is returned as a void
  pointer to device specifc data.
  # Convert the address address mode into a devicename.
  # Open the device.
  # Put the VME bus online.

  \param nMode - AddressMode [in] Selects the addressing mode
                 to be used accessing the VME bus.  The VME bus
		 is a multispaced bus. Address modifiers select an
		 address space and then the address selects 
		 locations within the address space.  See the
		 CVMEInterface::AddressMode enum for the supported
		 addressing modes.
  \param nCrate - Number of the crate to open. For the NSCL driver it is
                  an error to open a crate other than crate 0.

   \return void*  - A pointer to device specific data. In this case
                 a pointer to a file descriptor.

   \throw string description of any error.
*/
void*
CVMEInterface::Open(CVMEInterface::AddressMode nMode,
		    unsigned short nCrate) 
{

  if(nCrate != 0) {
    throw string("CVMEInterface[NSCL] -attempting to open crate other than 0");
  }

  int* pFd = new int;
  try {				// All errors get thrown.
    const char* pDev = ModeToDevice(nMode); // Translate the device.
    *pFd       = open(pDev, O_RDWR); // Open the device.
    if(*pFd < 0) {
      string err("CVMEInterface::Open Open failed for device ");
      err += pDev;
      err += ": ";
      err += strerror(errno);

      throw err;
    }
    struct vme_dev Vme;
    int stat = ioctl(*pFd, VME_CRATE_ON, &Vme);
    if(stat !=0) {
      string error("CVMEInterface::Open ");
      error  += "Unable to put VME crate online";
      close(*pFd);
      throw error;
    }
    
  }
  catch(...) {			// Need to delete resources.
    delete pFd;
    throw;
  }
  return (void*)pFd;
}
/*!
  Close a handle open on the VME device.  Once closed, the handle
  becomes unusable.  Note that memory maps don't go away unless
  the process exits or explicitly removes them before closing the
  device.

  Parameters:
  \param pDeviceHandle - void* [modified] - A pointer to the device
                             handle.  In this case the handle is a 
			     file descriptor.
  \throw string description of any error.
*/
void
CVMEInterface::Close(void* pDeviceHandle)
{
  
  
  int nFd  = *(int*)pDeviceHandle;
  if(close(nFd)) {
    int ern = errno;
    string error("CVMEInterface::Close close failed: ");
    error += strerror(ern);
    throw error;
  }
  

  delete (int*)pDeviceHandle;

}
/*!
   Map a chunk of vme memory into process virtual address space.
   In our case this is done via a call to mmap(2).  There are a few
   things we do to make life easier:
   - If necessary a larger region than requested is allocated to
     ensure page alignment. 
   - The region is first enlarged downward if necesary 
     by masking off page offset bits.
   - The region is then enlarged upwards if necessary
     by rounding the end point up to the next nearest page.

    Parameters:
    \param pDeviceHandle - void* [in] - Pointer to the
                         device handle (fd).
    \param nBase - unsigned long [in] Base address in VME space
                   of the desired map.
    \param nBytes- unsigned long [in] Number of bytes in the map.

   Returns:
   \return void* Pointer to the address in process virtual address
       space that corresponds to nBase on the VME bus.
   
   Exceptions:
   \throw string - Explanatory string.
*/
void*
CVMEInterface::Map(void* pDeviceHandle,
		   unsigned long nBase, unsigned long nBytes)
{
  // Compute the aligned version of the region:

  AlignedRegion Aligned;
  AlignRegion(&Aligned, nBase, nBytes);

  // Do the mmap... on error we throw an error string.

  void  *pMap =  mmap(0,  Aligned.s_lAlignedSize, 
		      PROT_READ | PROT_WRITE , MAP_SHARED,
		      *(int*)pDeviceHandle, Aligned.s_lAlignedBase);
  if((long)pMap == -1L) {
    int err = errno;
    string error("CVMEInterface[NSCLBIT3]::Map failed : ");
    error += strerror(errno);
    throw error;
  }
  unsigned long lMap = (unsigned long)pMap;
  lMap += Aligned.s_lUserOffset;
  return (void*)lMap;

}
/*!
   Unmap a memory region that has been mapped by Map.

Parameters:
   \param pDeviceHandle - void* [in] pointer to device handle (fd).
   \param pBase - void* [in] Pointer to the region returned from
                  a call to Map.
   \param lBytes - Number of bytes in the shared memory region
                   (passed in to Map).

  Exceptions:
   \throw - string - explanatory error string.
*/
void
CVMEInterface::Unmap(void* pDeviceHandle,
		      void*  pBase,
		      unsigned long lBytes)
{
  // Convert the region specification to an aligned region:

  unsigned long lBase = (unsigned long)pBase;
  AlignedRegion Aligned;
  AlignRegion(&Aligned, lBase, lBytes);

  // Unmap the actual mapped region:

  if(munmap((void*)Aligned.s_lAlignedBase, 
	    Aligned.s_lAlignedSize)) {
    int err = errno;
    string error("CVMEInterface[NSCLBit3]::Unmap failed : ");
    error += strerror(err);
    throw error;
  }

}
/*!
   Performs a block transfer from the VME bus to a user buffer.
   This is done by first performing an lseek and then doing a Read.

   Parameters:
   \param pDeviceHandle -void* [in] Pointer to the device handle (fd)
   \param lOffset - unsigned long [in] Offset from the bus at which
                        the read will be done.
   \param pBuffer - void* [out] Pointer to the buffer to receive the
                    transferred data.
   \param lBytes  - unsigned long [in] Number of bytes to read.

  Returns:
    \return int - Number of bytes to read.
*/
int
CVMEInterface::Read(void* pDeviceHandle,
		    unsigned long nOffset,
		    void*  pBuffer,
		    unsigned long nBytes)
{
  int fd = *(int*)pDeviceHandle;
  if(lseek(fd, nOffset, SEEK_SET) == -1) {
    int err = errno;
    string error("CVMEInterface[NSCLBit3]::Read lseek failed : ");
    error += strerror(err);
    throw error;
  }

  int nRead = read(fd, pBuffer, nBytes);
  if(nRead < 0) {
    int err = errno;
    string error("CVMEInterface[NSCLBit3]::Read read failed : ");
    error += strerror(err);
    throw error;
  }

  return nRead;
}
/*!
    Write a buffer as a block transfer to the VME bus.
    - Perform an lseek
    - Do a write.

Parameters:
\param pDeviceHandle - void* [in] Pointer to the device specific
                    handle (fd).
\param lOffset       - unsigned long [in] Offset into the VME address space
                     represented by pDeviceHandle for the first
		     write.
\param pBuffer       - void* [in] Pointer to the data to transfer.
\param lBytes        - unsigned long [in] Number of bytes to transfer.

Exceptions:
\throw string - Contains a descriptive error message.
*/
int
CVMEInterface::Write(void* pDeviceHandle,
		     unsigned long nOffset,
		     void* pBuffer,
		     unsigned long nBytes)
{
  int nFd = *(int*)pDeviceHandle;

  if(lseek(nFd, nOffset, SEEK_SET) == -1) {
    int err = errno;
    string error("CVMEInterface[NSCLBit3]::Write lseek failed : ");
    error += strerror(err);
    throw error;
  }

  int nWrite = write(nFd, pBuffer, nBytes);
  if(nWrite < 0) {
    int err = errno;
    string error("CVMEInterface[NSCLBit3]::Write wrte failed : ");
    error += strerror(err);
    throw error;
  }

  return nWrite;
}



