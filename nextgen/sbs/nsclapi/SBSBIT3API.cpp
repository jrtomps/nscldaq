static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";


#include <config.h>

#include "CVMEInterface.h"
#include "SBSBit3API.h"		// Device specific api.

#ifndef BT1003
#define BT1003
#endif
extern "C" {
#include <btapi.h>
#include <bt_trace.h>
}
#include <string>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

const char* CVMEInterface::m_szDriverName="SBSBIT3";

/*
 Local data structures.
 */

// Translating address spaces to logical device specifiers:

typedef struct _DeviceEntry {
  CVMEInterface::AddressMode   s_eModeId;
  bt_dev_t                     s_eLogicalDevice;
  int                          s_pioAmod; // If not UINT_MAX this is the the mmap amod.
  int                          s_dmaAmod; // If not UINT_MAX this is the dma amod.
} DeviceEntry, *pDeviceEntry;

static const DeviceEntry kaDeviceTable[] = {
  {CVMEInterface::A16,  BT_DEV_IO,  UINT_MAX,       UINT_MAX    },
  {CVMEInterface::A24,  BT_DEV_A24, UINT_MAX,       UINT_MAX   },
  {CVMEInterface::A32,  BT_DEV_A32, UINT_MAX,       UINT_MAX   },
  {CVMEInterface::GEO,  BT_DEV_A24, 0x2f,           UINT_MAX   }, // In general probably does not support DMA.
  {CVMEInterface::MCST, BT_DEV_A32, BT_AMOD_A32_ND, UINT_MAX},
  {CVMEInterface::CBLT, BT_DEV_A32, UINT_MAX,       BT_AMOD_A32_NB}
};

/*
  This opaque struct is what is handed back to the user as a device handle.
*/

typedef struct _DevHandle {
  bt_desc_t           s_handle;		        // SBS device handle.
  const DeviceEntry*  s_description;		// Corresponding entry in kaDeviceTable.
  size_t              s_fullBufferSize;         // Allocation size of transfer buffer.
  uint8_t*            s_rawBuffer;              // Pointer to full buffer.
  uint8_t*            s_alignedBuffer;          // page aligned buffer.

  _DevHandle() :
    s_fullBufferSize(0),
    s_rawBuffer(0),
    s_alignedBuffer(0) {}
  ~_DevHandle() {
    delete []s_rawBuffer;
  }

  int  pioAmod() {
    return s_description->s_pioAmod;
  }
  int dmaAmod() {
    return s_description->s_dmaAmod;
  }
  bt_dev_t device() {
    return s_description->s_eLogicalDevice;
  }
  uint8_t* getBuffer(size_t nBytes);

} DevHandle, *pDevHandle;

static const unsigned int knDeviceTableSize = sizeof(kaDeviceTable)/
                                              sizeof(DeviceEntry);

static const unsigned int BTERRORLENGTH(100); // Length of an error string.

static const int      AMOD_LOCK(1);           // Semaphore number for lock when diddling the amod.

// Local private 'static' functions.


/*!
  Return a pointer to a page aligned buffer for a handle that can 
  hold at least the requested number of bytes.  If necessary
  the buffer is allocated/reallocated to correct size.
  @param nBytes - Required bytes.
*/
uint8_t*
_DevHandle::getBuffer(size_t nBytes)
{
  int pageSize = getpagesize();
  int pageMask = pageSize - 1;	// Assumes pageSize is a power of 2.

  // Total size we need:

  size_t requiredBytes  = nBytes +  pageSize;		// To  ensure we can align.

  if (requiredBytes > s_fullBufferSize) {
    s_rawBuffer        = reinterpret_cast<uint8_t*>(realloc(s_rawBuffer, requiredBytes));
    s_fullBufferSize   = requiredBytes;
    // Realloc is not gauranteed to give the same pointer so
    // - recompute the page aligned buffer and size.
    // - visit each page and make it differ so that we avoid
    //   linux's memory compression which migh leave us with
    //   all our pointers pointing to the same page-frame.
    //
    s_alignedBuffer = reinterpret_cast<uint8_t*>(reinterpret_cast<uint32_t>(s_rawBuffer + pageMask) 
						 & ~pageMask); // Start of next page unless we're already aligned.
    uint8_t* p     = s_rawBuffer;
    uint8_t* e     = s_rawBuffer + s_fullBufferSize;
    uint8_t  i     = 0;
    while(p < e) {
      *p   = i;
      p   += pageSize;
    }
  }
  return s_alignedBuffer;
  
}


/*!
   Translate an address space selector into a logical device
   selector. What is returned is a dynamically allocated stuct that
   describes the device.

Parameters:

\param eAmode CVMEInterface::AddressMode [in] Addressing mode to
               translate.

Returns:
\return pDevHandle dynamically allocated.  This must be destroyed on close time.
       The s_description pointer is filled in, the s_handle is set to NULL which
       is likely to be invalid in case it's accidently used.

Exceptions:
\throw string describing why the lookup failed.

*/

static pDevHandle
AddressModeToDevice(CVMEInterface::AddressMode eMode)
{
  for(int i = 0; i < knDeviceTableSize; i++) {
    if(kaDeviceTable[i].s_eModeId == eMode) {
      pDevHandle result = new DevHandle;
      result->s_description = &(kaDeviceTable[i]);
      result->s_handle      = static_cast<bt_desc_t>(0);

      return result;
    }
  }
  throw string("[SBSBit3]AddressModeToDevice-no such address mode");
}


/*!
   If necessary throw an error from the return code generated by a
   Bt interface library function.
   \param pDevice     [bt_desc_t* in]: Pointer to device unit open on b3.
   \param nReturnCode [bt_error_t in]: A return code from the bit 3 interface
                                       library.
   \param pContextString [const char* in]: A string describing the context
                            of the potential error condition.

   \return - does if nReturnCode == BT_SUCCESS, otherwise, formats and
             throws a string exception.
   \throw  string if nReturnCode != BT_SUCCESS.

*/
static void 
CheckError(bt_desc_t* pDevice,
	   bt_error_t nReturnCode,
	   const char* pContextString)
{
  if(nReturnCode != BT_SUCCESS) {
    int nContextLen = strlen(pContextString);
    char ErrorMessage[nContextLen + BTERRORLENGTH];
    bt_strerror(*pDevice, nReturnCode, pContextString,
		ErrorMessage, sizeof(ErrorMessage));
    throw string(ErrorMessage);
  }
}

/*
 *  Overload of the function above, but the first parameter is device handle pointer
 */
static inline  void 
CheckError(pDevHandle p, bt_error_t nReturnCode, const char* pContextString)
{
  CheckError(&(p->s_handle), nReturnCode, pContextString);
}


/**
 * Function to set the appropriate address modifier.
 * @param pHandle   - Handle open on the device.
 * @param which     - One of BT_INFO_PIO_AMOD or BT_INFO_DMA_AMOD
 *
 * @return bt_devdata_t - the old address modifier or UINT_MAX if it not modified.

 */
static bt_devdata_t 
setAmod(pDevHandle pHandle, bt_info_t which)
{
  bt_devdata_t oldAmod(UINT_MAX);
  bt_devdata_t newAmod;
  if ((which == BT_INFO_MMAP_AMOD) || (which == BT_INFO_PIO_AMOD))  {
    newAmod = pHandle->pioAmod();
  }
  else {
    newAmod = pHandle->dmaAmod();
  }
  if (newAmod != UINT_MAX) {
    bt_error_t err =  bt_get_info(pHandle->s_handle, which, &oldAmod);
    assert(err == BT_SUCCESS);
    CVMEInterface::Lock(AMOD_LOCK);
    err = bt_set_info(pHandle->s_handle, which, newAmod);
    assert(err == BT_SUCCESS);
  }
  return oldAmod;

}
/*
 * Function to reverse the effect of setAmod.
 * @param pHandle Handle open on the device.
 * @param which   One of  BT_INFO_PIO_AMOD or BT_INFO_DMA_AMOD
 * @param prior   Value returned from setAmod.
 *
 */
static void
restoreAmod(pDevHandle pHandle, bt_info_t which , bt_devdata_t prior)
{
  if (prior != UINT_MAX) {
    bt_error_t err = bt_set_info(pHandle->s_handle, which, prior);
    assert(err == BT_SUCCESS);
    CVMEInterface::Unlock(AMOD_LOCK);
  }

}

/*!
   Open the a device and returns a handle to it:
   - The Address space selector is translated to a device name.
   - a bt_desc_t is allocated.
   - bt_open is called to fill in the handle.
   - A Pointer to the handle is returned to the user.

Parameters:
\param eMode CVMEInterface::AddressMode [in] Specifies the
             address mode desired for the device.
\param nCrate Number of the crate to open.  This can be a value from 0-15.

Throws:
 \throw string - Text string describing the error.

\note The assumption is that both read and write access are desired.
\note We assume there's only one VME crate attached to the system.
*/
void*
CVMEInterface::Open(AddressMode eMode,
		    unsigned short nCrate)
{

  // Validate the crate number.

  if(nCrate > BT_MAX_UNITS) {
    throw string("CVMEInterface::[SBS]Open - vme crate number > BT_MAX_UNITS");
  }
  pDevHandle pHandle(0);
    
  try {
    char Device[BT_MAX_DEV_NAME];


    pHandle   = AddressModeToDevice(eMode);

    if(!bt_gen_name(nCrate, pHandle->device(), Device, sizeof(Device))) {
      throw 
	string("CVMEInterface[SBSBit3]::Open - bt_gen_name failed");
    }

    bt_error_t err = bt_open(&(pHandle->s_handle), Device, BT_RDWR);
    CheckError(pHandle, err, "CVMEInterface[SBSBit3]::Open - Open failed ");

    // Set the appropriate byte swap mode for block transfers.

    CSBSBit3VmeInterface::SetSwapMode(pHandle, BT_SWAP_NONE);

  }
  catch(...) {
    delete pHandle;
    throw;
  }
 
  return (void*)pHandle;
}
/*!
  Closes a previously opened device.  Note that the device handle
  is a pointer to a bt_desc_t that will be deleted.

Parameters:

\param pDevice - void* [modified] Pointer to the device dependent
          handle.  In our case that represents a bt_desc_t.

Exceptions:
\throw string - Describing any errors.
*/
void  
CVMEInterface::Close(void* pDeviceHandle)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t *p = &(pHandle->s_handle);
  
  bt_error_t err = bt_close(*p);
  
  CheckError(p, err, "CVMEInterface[SBSBit3]::Close - bt_close failed:");
  delete pHandle;


}

/*!
  Map a region of VME address space into process virtual address
  space.  Once mapped, accesses to the process adress space will
  be modifying or reading the VME addreess space without any driver
  intervention.

Parameters:
\param pDeviceHandle - void* [in] Device dependent handle open on 
                  the VME interface device (bt_desc_t*).

\param lOffset - unsigned long [in] - base address of the first
      VME location to map.

\param lBytes - unsigned long [in] - Number of bytes of VME space
               desired.

Returns:
\return void* Pointer to the address space mapped.

\note Additional space may be mapped before and after the requested
     range in order to acocmodate the memory mapping granularity
     of the underlying operating system.  This additional mapping
     is transparent to the user software.

Exceptions:
\throw string - Explains any errors.

*/
void*
CVMEInterface::Map(void* pDeviceHandle,
		   unsigned long nBase, 
		   unsigned long nBytes)
{
  
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);
  void*      pSpace;


  // If the handle has an associated pioAMOD, we must set it and
  // then return it.. All of this is done with the AMOD lock held.
  //
  bt_devdata_t oldValue;
  oldValue = setAmod(pHandle, BT_INFO_MMAP_AMOD);

  bt_error_t err = bt_mmap(*p,
			   &pSpace, nBase, nBytes,
			   BT_RDWR, BT_SWAP_NONE);

  restoreAmod(pHandle, BT_INFO_MMAP_AMOD, oldValue);

  CheckError(p, err, "CVMEInterface[SBSBit3]::Map - bt_mmap failed : ");
  return pSpace;
}
/*! 
  Unmap an existing address map from process virtual addres space to
  VME address space.  Once the unmap is done, accesses to that 
  virtual address window will fail with  a SEGFLT or BUSERR
  (OS dependent).

Parameters:
\param pDeviceHandle - void* [in] Device handle pointer.  In our
                   case, this points to a bt_desc_t.

\param pBase    - void* [i] Points to the base of the map.
                 lBytes pointed to by this pointer will become
		 inaccessible to the calling process as a result
		 of this call.
\param lBytes - unsigned long [in] Number of bytes in the region.

Exceptions:
\throw - string - describing errors from bt_unmap.

*/
void
CVMEInterface::Unmap(void* pDeviceHandle,
		     void*  pBase,
		     unsigned long lBytes)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);

  bt_desc_t* p = &(pHandle->s_handle);
  
  bt_error_t err = bt_unmmap(*p, pBase, lBytes);

  CheckError(p, err, "CVMEInterface[SBSBit3]::Unmap - bt_mmap failed : ");

}
/*!
  Performs a block transfer of data from the VME bus to a local
  buffer.  This function does not require a map to operate.

Parameters:
\param pDeviceHandle - void* [in] Pointer to the device handle
             open on a PCI<-->VME bus interface. In our case
	     the handle is a bt_desc_t*

\param nOffset - unsigned long [in] Address in VME space from which
            the transfer will start.

\param pBuffer - void* [out] Pointer to the buffer into which data
           will be transferred.

\param nBytes  - unsigned long [in] Number of bytes to transfer.

Returns:
\return int  - Number of bytes transferred.

Exceptions:
\throw string - Descriptive error text if there was a problem.
*/

int
CVMEInterface::Read(void* pDeviceHandle,
		    unsigned long nOffset,
		    void*  pBuffer,
		    unsigned long nBytes)
{


  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);

  bt_desc_t* p = &(pHandle->s_handle);



  size_t    nTransferred;

  // If there is a dma address modifier associated with this 
  // device handle, we must lock the semaphore, save the old
  // value set the one we want, perform the I/O and
  // restore the value before unlocking the semaphore.

  bt_devdata_t oldAmod;
  oldAmod = setAmod(pHandle, BT_INFO_DMA_AMOD);

  CSBSBit3VmeInterface::SetTraceMask( pHandle,
				      BT_TRC_RD_WR |
				      BT_TRC_DMA   |
				      BT_TRC_PIO   |
				      BT_TRC_RD_WR |
				      BT_TRC_INFO |
				      BT_TRC_DETAIL |
				      BT_TRC_FUNC);

  // Buffer the read through a page aligned buffer:

  uint8_t*     pLocalBuffer = pHandle->getBuffer(nBytes);
  bt_error_t err = bt_read(*p, pLocalBuffer, nOffset, nBytes,
			   &nTransferred);

  restoreAmod(pHandle, BT_INFO_DMA_AMOD, oldAmod);


  CheckError(p, err, "CVMEInterface[SBSBit3]::Read - bt_read failed : ");
  memcpy(pBuffer, pLocalBuffer, nTransferred);
  return nTransferred;
     

}
/*!
  Write a block of data from a user buffer to addresses on the VME
bus.

Parameters:
\param pDeviceHandle void* [in] Pointer to the device specific
               handle. In our case this is realy a bt_desc_t*

\param lOffset unsigned long [in] Base address in the VME for the
              transfer.

\param pBuffer void * [in] Pointer to the buffer from which
    the data comes.

\param lBytes unsigned long [in] Number of bytes to write.

Returns:
\return int  - Number of bytes transferred.

Exceptions:
\throw string - Descriptive error text if there was a problem.
*/
int
CVMEInterface::Write(void* pDeviceHandle,
		     unsigned long nOffset,
		     void* pBuffer,
		     unsigned long nBytes)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);

  bt_desc_t* p = &(pHandle->s_handle);

  size_t    nTransferred;

  // If there is a dma address modifier associated with this 
  // device handle, we must lock the semaphore, save the old
  // value set the one we want, perform the I/O and
  // restore the value before unlocking the semaphore.

  bt_devdata_t oldAmod;
  oldAmod = setAmod(pHandle, BT_INFO_DMA_AMOD);

  // Buffer the write through a page aligned buffer:

  uint8_t* pLocalBuffer = pHandle->getBuffer(nBytes);
  memcpy(pLocalBuffer, pBuffer, nBytes);
  bt_error_t err = bt_write(*p, pLocalBuffer, nOffset, nBytes,
			   &nTransferred);

  restoreAmod(pHandle, BT_INFO_DMA_AMOD, oldAmod);


  CheckError(p, err, "CVMEInterface[SBSBit3]::Write - bt_read failed : ");

  return nTransferred;
     
 
}
//  The functions following this comment are all SBSBit3 API specific.
//  The header for them is in SBSBit3API.h  Users call these functions at
//  their peril as they bind them to a specific piece of hardware.
//

/*!
   Set the dma transfer mode to use VME block transfer.  VME Block transfer
   allows the device to do away with additional address cycles after a single
   initial address cycle establishes the transfer base address, and the DS's
   establish the transfer width.  Subsequent data transfers assume an
   appropriate increment in address.. This allows data transfer to be
   significantly faster for blocks of contiguous data.

   Older slave interfaces may not implement DMA transfer however.

   \param pHandle (void* [in]):  pointer to the device handle gotten via 
      CVMEInterface::Open.
   \param enable (bool [in]): true if block transfer mode is desired, 
          false otherwise.

*/
void
CSBSBit3VmeInterface::SetDMABlockTransfer(void* pDeviceHandle, bool enable)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  bt_error_t err = bt_set_info(*p, BT_INFO_BLOCK, (bt_devdata_t)enable);
  CheckError(p, err, "CSBSBit3VmeInterface::SetDMABlockTransfer failed");
}
/*!
   Set the DMA transfer mode to 'pause'.  In pause, mode, the device
   re-arbitrates the bus more often than required by the block mode
   transfer specification.  While this allows other bus masters a better
   bus access latency in the presence of DMA transfers, it also reduces
   the DMA throughput.

   \param pDevice (void* [in]): Pointer to device handle returned from
   VMEInterface::Open().
   \param enable (bool [in]): State desired:
   - true  - turns on pause mode.
   - false - turns off pause mode.
*/
void
CSBSBit3VmeInterface::SetDMAPauseMode(void* pDeviceHandle, bool enable)
{ pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  bt_error_t err = bt_set_info(*p, BT_INFO_PAUSE, (bt_devdata_t)enable);
  CheckError(p, err, "CSBSBit3VmeInterface::SetDMAPauseMode failed");
} 
/*!
  Set the maximum transfer width of the interface.  Valid values of the 
  bt_width_t parameter can be any of:
  - BT_WIDTH_D8   8 bit maximum transfer width
  - BT_WIDTH_D16  16 bit maximum transfer width
  - BT_WIDTH_D32  32 bit maximum transfer width
  - BT_WIDTH_D64  64 bit maximum transfer width
  - BT_WIDTH_ANY  Let the driver choose the best setting.
 
  By default, after power up, the driver is set at BT_WIDTH_D32 since
  most P's don't support D6 transfers (according to the Bit3 manual, while this
  has changed with PCI-64, the SBS/Bit3 -620 is a 32 bit wide PCI module.

  \param pHandle (void* [in]): Handle open on the device, gotten via 
    a successful call to CVMEInterface::Open
  \param width (bt_data_width_t [in]): Requested width.  See above for legal
     values.
*/
void
CSBSBit3VmeInterface::SetMaxTransferWidth(void* pDeviceHandle, bt_width_t eWidth)
{ 
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

    bt_error_t err = bt_set_info(*p, BT_INFO_DATAWIDTH, eWidth);
  CheckError(p, err, "CSBSBit3VmeInterface::SetMaxTransferWidth failed");
}
/*!
  Set the DMA address modifier code.  It is important that this be selected
  from one of the block transfer modes if DMA block mode is on else DMA 
  with slaves may not work
  correctly.  The default DMA transfer mode is supervisory d32 block transfer.
  
  \param pHandle (void* [in]):  Handle returned from CVMEInterface::Open.
  \param nModifier (int [in]):  Address modifier.  Symbolic definitions
  of these are as follows:
  -  BT_AMOD_A32_SB (0x0F) A32 Extended Supervisory Block transfer.
  -  BT_AMOD_A32_SP (0x0E) A32 Extended Supervisory Program.
  -  BT_AMOD_A32_SD (0x0D) A32 Extended Supervisory Data Access.
  -  BT_AMOD_A32_NB (0x0B) A32 Extended Non-Privileged Block transfer.
  -  BT_AMOD_A32_NP (0x0A) A32 Extended Non-Privileged Program.
  -  BT_AMOD_A32_ND (0x09) A32 Extended Non-Privileged Data.
  -  BT_AMOD_A32_SMBLT (0x0C)  A32 Multiplexed Block Transfer (D64) priv.
  -  BT_AMOD_A32_NMBLT   (0x08)  A32 Multiplexed Block Transfer (D64) nonpriv
  -  BT_AMOD_A32   see  BT_AMOD_A32_SD
  -  BT_AMOD_A32_D64 see BT_AMOD_A32_SMBLT
  -  BT_AMOD_A24_SB (0x3F)  A24 Standard Supervisory Block
  -  BT_AMOD_A24_SP (0x3E)  A24 Standard Supervisory Program   
  -  BT_AMOD_A24_SD (0x3D)  A24 Standard Supervisory Data Access 
  -  BT_AMOD_A24_NB (0x3B)  A24 Standard Non-Privileged Block  
  -  BT_AMOD_A24_NP (0x3A)  A24 Standard Non-Privileged Program 
  -  BT_AMOD_A24_ND (0x39)  A24 Standard Non-Privileged Data 
  -  BT_AMOD_A24_SMBLT (0x3C) A24 Multiplexed Block Transfer (D64) privileged
  -  BT_AMOD_A24_NMBLT (0x38) A24 Multiplexed Block Transfer (D64) nonpriv.
  -  BT_AMOD_A24  see BT_AMOD_A24_SD
  -  BT_AMOD_A24_D64 see BT_AMOD_A24_SMBLT
  -  BT_AMOD_A16_SD (0x2D)  A16 Short Supervisory Data Access
  -  BT_AMOD_A16_ND (0x29)  A16 Short Non-Privileged Data Access 
  -  BT_AMOD_A16  see   BT_AMOD_A16_SD
  -  BT_AMOD_GEO   (0x2F)  Geographical addressing
  -  BT_AMOD_MCCTL (0X09)  Multicast control register writes (CAEN ADCS).
  -  BT_AMOD_CBLT  (0x0B)  Chained block transfer (CAEN adcs).

*/
void
CSBSBit3VmeInterface::SetDMAAddressModifier(void* pDeviceHandle, int Modifier)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);
  bt_error_t  err = bt_set_info(*p, BT_INFO_DMA_AMOD, Modifier);
  CheckError(p, err, "CSBSBit3VmeInterface::SetDMAAddressModifier failed");
}
/*!
   SetMMAPAddress modifier sets the address modifier used for the mmap call.
   note that this is somewhat dangerous.  The devices themselves have a 
   default address modifier.  Use that whenever possible, as this setting
   does not revert to default on close !!!!

   \param pHandle (void* [in]): Device handle returned from CVMEInterface::Open
   \param Modifier (int [in]): Requested address modifier.  Legal values are:
  -  BT_AMOD_A32_SB (0x0F) A32 Extended Supervisory Block transfer.
  -  BT_AMOD_A32_SP (0x0E) A32 Extended Supervisory Program.
  -  BT_AMOD_A32_SD (0x0D) A32 Extended Supervisory Data Access.
  -  BT_AMOD_A32_NB (0x0B) A32 Extended Non-Privileged Block transfer.
  -  BT_AMOD_A32_NP (0x0A) A32 Extended Non-Privileged Program.
  -  BT_AMOD_A32_ND (0x09) A32 Extended Non-Privileged Data.
  -  BT_AMOD_A32_SMBLT (0x0C)  A32 Multiplexed Block Transfer (D64) priv.
  -  BT_AMOD_A32_NMBLT   (0x08)  A32 Multiplexed Block Transfer (D64) nonpriv
  -  BT_AMOD_A32   see  BT_AMOD_A32_SD
  -  BT_AMOD_A32_D64 see BT_AMOD_A32_SMBLT
  -  BT_AMOD_A24_SB (0x3F)  A24 Standard Supervisory Block
  -  BT_AMOD_A24_SP (0x3E)  A24 Standard Supervisory Program   
  -  BT_AMOD_A24_SD (0x3D)  A24 Standard Supervisory Data Access 
  -  BT_AMOD_A24_NB (0x3B)  A24 Standard Non-Privileged Block  
  -  BT_AMOD_A24_NP (0x3A)  A24 Standard Non-Privileged Program 
  -  BT_AMOD_A24_ND (0x39)  A24 Standard Non-Privileged Data 
  -  BT_AMOD_A24_SMBLT (0x3C) A24 Multiplexed Block Transfer (D64) privileged
  -  BT_AMOD_A24_NMBLT (0x38) A24 Multiplexed Block Transfer (D64) nonpriv.
  -  BT_AMOD_A24  see BT_AMOD_A24_SD
  -  BT_AMOD_A24_D64 see BT_AMOD_A24_SMBLT
  -  BT_AMOD_A16_SD (0x2D)  A16 Short Supervisory Data Access
  -  BT_AMOD_A16_ND (0x29)  A16 Short Non-Privileged Data Access 
  -  BT_AMOD_A16  see   BT_AMOD_A16_SD
  -  BT_AMOD_GEO   (0x2F)  Geographical addressing
  -  BT_AMOD_MCCTL (0X09)  Multicast control register writes (CAEN ADCS).
  -  BT_AMOD_CBLT  (0x0B)  Chained block transfer (CAEN adcs).
*/
void
CSBSBit3VmeInterface::SetMMAPAddressModifier(void* pDeviceHandle, int Modifier)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  bt_error_t  err = bt_set_info(*p, BT_INFO_MMAP_AMOD, Modifier);
  CheckError(p, err, "CSBSBit3VmeInterface::SetMMAPAddressModifier failed");
}
/*!
   Set the data swap mode.  The SBS/Bit3 interface has byte swapping hardware
   that can be set as needed to acommodate differences in byte order between
   the host (linux system ), and slave interface (VME board).
   \param pHandle (void*  [in]): Handle received from a successful call to 
      CVMEInterface::Open.
   \param SwapMode (bt_swap_t [in]) Swap mode, can be one of the follwoing:
   -  BT_SWAP_NONE           No swapping
   -  BT_SWAP_BSNBD          Byte swap, non byte data
   -  BT_SWAP_WS             Word swap
   -  BT_SWAP_WS_BSNBD       Word swap and byte swap  non btye data
   -  BT_SWAP_BSBD           Byte swap  btye data 
   -  BT_SWAP_BSBD_BSNBD     Btye swap  byte data and byte swap  non byte data
   -  BT_SWAP_BSBD_WS        Byte swap  byte data and word swap
   -  BT_SWAP_BSBD_WS_BSNBD  All possible swapping
   -  BT_SWAP_DEFAULT        Driver default swapping
 
   BT_SWAP_DEFAULT is set to transparently swap bytes in VME byte order (big 
   endian) to PC byte ordering (little endian).
   Note that this setting only affects DMA transfers.  At present, you have no
   control over the swap mode for memory mapping, and it is BT_SWAP_DEFAULT.

*/
void
CSBSBit3VmeInterface::SetSwapMode(void* pDeviceHandle, bt_swap_t SwapMode)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);
  bt_error_t  err = bt_set_info(*p, BT_INFO_SWAP, SwapMode);
  CheckError(p, err, "CSBSBit3VmeInterface::SetSwapMode failed");
}
/*!
  Sets the dma threshold.  The dma theshold defines the minimum length
  transfer in CVMEInterface::Read or CVMEInterface::Write that will turn on
  the bit3's dma engine.  If a Read/Write is issued for a transfer smaller than
  the threshold, the driver will perform PIO to satisfy it, otherwise it will
  perform the transfer via DMA.

  This is a trade off between the high speed transfer
  of the dma engine against the longer setup time requirement compared to
  PIO transfers.  If a Read/Write is issued for a transfer smaller than

  \param pHandle (void* [in]): Device handle returned from CVMEInterface::Open
  \param nTransfers (int [in]): The new threshold.

*/
void
CSBSBit3VmeInterface::SetDMAThreshold(void* pDeviceHandle, int nTransfers)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  bt_error_t  err = bt_set_info(*p, BT_INFO_DMA_THRESHOLD, nTransfers);
  CheckError(p, err, "CSBSBit3VmeInterface::SetDMAThreshold failed");
}
/*!
  Sets the DMA polling ceiling.  This determines how many transfers are 
  required before the driver decides to use interrupts as opposed to polling
  to determine that a DMA transfer has completed.  

  This value represents a trade-off between operation completion latency
  (better for polling), and overall linux system responsiveness (better
  for interrupts).  Since any polling will be done in the device driver,
  the linux system will be almost completely unresponsive for the duration
  of a DMA that polls for completion.
  
  \param pHandle (void* [in]): Handle on the device from CVMEInterface::Open.
  \param nTransfers (int [in]): The new ceiling value.
*/
void
CSBSBit3VmeInterface::SetDMAPollCeiling(void* pDeviceHandle, int nTransfers)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  bt_error_t  err = bt_set_info(*p, BT_INFO_DMA_POLL_CEILING, nTransfers);
  CheckError(p, err, "CSBSBit3VmeInterface::SetDMAPollCeiling failed");  
}
/*!
  Sets the trace mask.  The trace mask determines how much inforation the
  driver logs to the /var/log files.  The less information logged, the better
  the performance, however the more information logged, the easier
  it is to troubleshoot problems.
  \param pHandle (void * [in]): The handle returned from CVMEInterface::Open
  \param nMask  A bitwise or of any of the following bits:
  -  BT_TRC_ERROR     Fatal system errors, shouldn't happen
  -  BT_TRC_WARN      driver errors that may happen due to external conditions
  -  BT_TRC_INFO      Messages for errors returned to the user
  -  BT_TRC_CFG       driver configuration and initialization
  -  BT_TRC_RD_WR     generic read/write (outside of DMA & PIO)
  -  BT_TRC_MMAP      memory mapping and related code/
  -  BT_TRC_BIND      bt_bind() and related code 
  -  BT_TRC_CTRL      All ioctl() functions 
  -  BT_TRC_OPEN      Driver open and close
  -  BT_TRC_ISR       Interrupt Service Routine
  -  BT_TRC_IRQ_DISP  Interrupt Request Processing/Dispatch
  -  BT_TRC_ICBR      user ICBR and the vector returned by applicable kernel
                      interrupt handlers.
  -  BT_TRC_ISR_ERR   ISR errors only
  -  BT_TRC_DMA       DMA, each transfer
  -  BT_TRC_PIO       PIO, each transfer
  -  BT_TRC_FUNC      function entry and exit
  -  BT_TRC_ALLOC     Allocation/release of resources
  -  BT_TRC_LOCK      all locks (user & driver internal)
  -  BT_TRC_MAPREG    details of each map register load 
  -  BT_TRC_RAWIO     Raw I/O register access
  -  BT_TRC_KLIB      Misc. shared kernel routines
  -  BT_TRC_KDDI      All 3rd party driver rouitnes KDDI
  -  BT_TRC_LIO       Low level I/O register rd/wr 
  -  BT_TRC_PROFILE   enable H/W timing marks for profiling
  -  BT_TRC_DETAIL    Detailed trace of enabled function 
  -  BT_TRC_HW_DEBUG  enable HW trace messages instead of console 
  -  BT_TRC_ERROR     Fatal system errors, shouldn't happen
  -  BT_TRC_WARN      driver errors that may happen due to external conditions
  -  BT_TRC_INFO      Messages for errors returned to the user
  -  BT_TRC_CFG       driver configuration and initialization
  -  BT_TRC_RD_WR     generic read/write (outside of DMA & PIO)
  -  BT_TRC_MMAP      memory mapping and related code/
  -  BT_TRC_BIND      bt_bind() and related code 
  -  BT_TRC_CTRL      All ioctl() functions 
  -  BT_TRC_OPEN      Driver open and close
  -  BT_TRC_ISR       Interrupt Service Routine
  -  BT_TRC_IRQ_DISP  Interrupt Request Processing/Dispatch
  -  BT_TRC_ICBR      user ICBR and the vector returned by applicable kernel
                      interrupt handlers.
  -  BT_TRC_ISR_ERR   ISR errors only
  -  BT_TRC_DMA       DMA, each transfer
  -  BT_TRC_PIO       PIO, each transfer
  -  BT_TRC_FUNC      function entry and exit
  -  BT_TRC_ALLOC     Allocation/release of resources
  -  BT_TRC_LOCK      all locks (user & driver internal)
  -  BT_TRC_MAPREG    details of each map register load 
  -  BT_TRC_RAWIO     Raw I/O register access
  -  BT_TRC_KLIB      Misc. shared kernel routines
  -  BT_TRC_KDDI      All 3rd party driver rouitnes KDDI
  -  BT_TRC_LIO       Low level I/O register rd/wr 
  -  BT_TRC_PROFILE   enable H/W timing marks for profiling
  -  BT_TRC_DETAIL    Detailed trace of enabled function 
  -  BT_TRC_HW_DEBUG  enable HW trace messages instead of console 

     Note that the default, power up state is BT_TRC_ERROR | BT_TRC_WARN.
*/
void
CSBSBit3VmeInterface::SetTraceMask(void* pDeviceHandle, int nMask)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  bt_error_t  err = bt_set_info(*p, BT_INFO_TRACE, nMask);
  CheckError(p, err, "CSBSBit3VmeInterface::SetTraceMask failed");
}

// Information retrieval functions:
//    Retrieve read-only parameters of the driver.
//
/*!
    Retrieve the SBS part number of the PCI component of the bus interface.

    \param pHandle (void* [in]): The handle returned from CVMEInterface::Open.
    \return part number as an integer.
*/
int
CSBSBit3VmeInterface::GetLocalCardPartNumber(void* pDeviceHandle)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);
  bt_devdata_t  Result;
  bt_error_t      err = bt_get_info(*p, BT_INFO_LOC_PN, &Result);
  CheckError(p, err, "CSBSBit3VmeInterface::GetLocalPartNumber failed");
  return Result;
}
/*!
  Retrieve the SBS part number of the VME component of the bus bridge.
    \param pHandle (void* [in]): The handle returned from CVMEInterface::Open.
    \return part number as an integer.
*/
int
CSBSBit3VmeInterface::GetRemoteCardPartNumber(void* pDeviceHandle)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  bt_devdata_t  Result;
  bt_error_t      err = bt_get_info(*p, BT_INFO_REM_PN, &Result);
  CheckError(p, err, "CSBSBit3VmeInterface::GetRemotePartNumber failed");
  return Result;
}
/*!
  Returns the number of bytes of local memory on any daughter board in
  the VME interface. Presumably this will be 0 if no memory daughter board
  is installed.
    \param pHandle (void* [in]): The handle returned from CVMEInterface::Open.
    \return memory size as unsigned int.
*/
unsigned int
CSBSBit3VmeInterface::GetLocalMemorySize(void* pDeviceHandle)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  bt_devdata_t  Result;
  bt_error_t      err = bt_get_info(*p, BT_INFO_LM_SIZE, &Result);
  CheckError(p, err, "CSBSBit3VmeInterface::GetLocalMemorySize failed");
  return Result;
}
/*!

  Returns a bool indicating whether or not the DMA block mode transfer
  is enabled.  For more information about this see, 
  CSBSBit3VmeInterface::SetDMABlockTransfer.

  \param pHandle (void* [in]): handle returned from CVMEInterface::Open.
  \return true if dma on this unit will be done in block transfer mode.

*/
bool
CSBSBit3VmeInterface::IsDMABlockTransfer(void* pDeviceHandle)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  bt_devdata_t  Result;
  bt_error_t      err = bt_get_info(*p, BT_INFO_BLOCK, &Result);
  CheckError(p, err, "CSBSBit3VmeInterface::IsDMABlockTransfer failed");
  return Result;
}
/*!
  Returns a bool that indicates whether or not DMA block transfers are done
  in pause mode.  For more information about pause mode, see:
  CSBSBit3VmeInterface::SetDMAPauseMode.
  \param pHandle (void* [in]) Handle returned from CVMEInterface::Open
  \return true if dma will be done in pause mode.
*/
bool
CSBSBit3VmeInterface::IsDMAPauseMode(void* pDeviceHandle)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  bt_devdata_t  Result;
  bt_error_t      err = bt_get_info(*p, BT_INFO_PAUSE, &Result);
  CheckError(p, err, "CSBSBit3VmeInterface::IsDMAPauseMode failed");
  return Result;
}
/*!
   Returns the Maximum transfer width.  
   \param pHandle (void* [in]) Device handle gotten from CVMEInterface::Open.
   \return The return value will be one of:
   - BT_WIDTH_D8   8 bits.
   - BT_WIDTH_D16  16 bits.
   - BT_WIDTH_D32  32 bits.
   - BT_WIDTH_D64  64 bits.
*/
bt_width_t
CSBSBit3VmeInterface::GetMaxTransferWidth(void* pDeviceHandle)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  bt_devdata_t Result;
  bt_error_t      err    = bt_get_info(*p, BT_INFO_DATAWIDTH, &Result);
  CheckError(p, err, "CSBSBit3VmeInterface::GetMaxTransferWidth failed");
  return Result;
}
/*!
   Return the current dma address modifier.  
   \param pHandle (void* [in])  Device handle gotten from CVMEInterface::Open.
   \return The return value will be one of
  -  BT_AMOD_A32_SB (0x0F) A32 Extended Supervisory Block transfer.
  -  BT_AMOD_A32_SP (0x0E) A32 Extended Supervisory Program.
  -  BT_AMOD_A32_SD (0x0D) A32 Extended Supervisory Data Access.
  -  BT_AMOD_A32_NB (0x0B) A32 Extended Non-Privileged Block transfer.
  -  BT_AMOD_A32_NP (0x0A) A32 Extended Non-Privileged Program.
  -  BT_AMOD_A32_ND (0x09) A32 Extended Non-Privileged Data.
  -  BT_AMOD_A32_SMBLT (0x0C)  A32 Multiplexed Block Transfer (D64) priv.
  -  BT_AMOD_A32_NMBLT   (0x08)  A32 Multiplexed Block Transfer (D64) nonpriv
  -  BT_AMOD_A32   see  BT_AMOD_A32_SD
  -  BT_AMOD_A32_D64 see BT_AMOD_A32_SMBLT
  -  BT_AMOD_A24_SB (0x3F)  A24 Standard Supervisory Block
  -  BT_AMOD_A24_SP (0x3E)  A24 Standard Supervisory Program   
  -  BT_AMOD_A24_SD (0x3D)  A24 Standard Supervisory Data Access 
  -  BT_AMOD_A24_NB (0x3B)  A24 Standard Non-Privileged Block  
  -  BT_AMOD_A24_NP (0x3A)  A24 Standard Non-Privileged Program 
  -  BT_AMOD_A24_ND (0x39)  A24 Standard Non-Privileged Data 
  -  BT_AMOD_A24_SMBLT (0x3C) A24 Multiplexed Block Transfer (D64) privileged
  -  BT_AMOD_A24_NMBLT (0x38) A24 Multiplexed Block Transfer (D64) nonpriv.
  -  BT_AMOD_A24  see BT_AMOD_A24_SD
  -  BT_AMOD_A24_D64 see BT_AMOD_A24_SMBLT
  -  BT_AMOD_A16_SD (0x2D)  A16 Short Supervisory Data Access
  -  BT_AMOD_A16_ND (0x29)  A16 Short Non-Privileged Data Access 
  -  BT_AMOD_A16  see   BT_AMOD_A16_SD
  -  BT_AMOD_GEO   (0x2F)  Geographical addressing
  -  BT_AMOD_MCCTL (0X09)  Multicast control register writes (CAEN ADCS).
  -  BT_AMOD_CBLT  (0x0B)  Chained block transfer (CAEN adcs).
*/
int
CSBSBit3VmeInterface::GetDMAAddressModifier(void* pDeviceHandle)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  bt_devdata_t Result;
  bt_error_t     err   = bt_get_info(*p, BT_INFO_DMA_AMOD, &Result);
  CheckError(p, err, "CSBSBit3VmeInterface::GetDMAAddressModifier failed");
  return    Result;
}
/*!
   Get the address modifier that will be applied to the mmap function.
   \param pHandle (void* [in])  Device handle gotten from CVMEInterface::Open.
   \return The return value will be one of
  -  BT_AMOD_A32_SB (0x0F) A32 Extended Supervisory Block transfer.
  -  BT_AMOD_A32_SP (0x0E) A32 Extended Supervisory Program.
  -  BT_AMOD_A32_SD (0x0D) A32 Extended Supervisory Data Access.
  -  BT_AMOD_A32_NB (0x0B) A32 Extended Non-Privileged Block transfer.
  -  BT_AMOD_A32_NP (0x0A) A32 Extended Non-Privileged Program.
  -  BT_AMOD_A32_ND (0x09) A32 Extended Non-Privileged Data.
  -  BT_AMOD_A32_SMBLT (0x0C)  A32 Multiplexed Block Transfer (D64) priv.
  -  BT_AMOD_A32_NMBLT   (0x08)  A32 Multiplexed Block Transfer (D64) nonpriv
  -  BT_AMOD_A32   see  BT_AMOD_A32_SD
  -  BT_AMOD_A32_D64 see BT_AMOD_A32_SMBLT
  -  BT_AMOD_A24_SB (0x3F)  A24 Standard Supervisory Block
  -  BT_AMOD_A24_SP (0x3E)  A24 Standard Supervisory Program   
  -  BT_AMOD_A24_SD (0x3D)  A24 Standard Supervisory Data Access 
  -  BT_AMOD_A24_NB (0x3B)  A24 Standard Non-Privileged Block  
  -  BT_AMOD_A24_NP (0x3A)  A24 Standard Non-Privileged Program 
  -  BT_AMOD_A24_ND (0x39)  A24 Standard Non-Privileged Data 
  -  BT_AMOD_A24_SMBLT (0x3C) A24 Multiplexed Block Transfer (D64) privileged
  -  BT_AMOD_A24_NMBLT (0x38) A24 Multiplexed Block Transfer (D64) nonpriv.
  -  BT_AMOD_A24  see BT_AMOD_A24_SD
  -  BT_AMOD_A24_D64 see BT_AMOD_A24_SMBLT
  -  BT_AMOD_A16_SD (0x2D)  A16 Short Supervisory Data Access
  -  BT_AMOD_A16_ND (0x29)  A16 Short Non-Privileged Data Access 
  -  BT_AMOD_A16  see   BT_AMOD_A16_SD
  -  BT_AMOD_GEO   (0x2F)  Geographical addressing
  -  BT_AMOD_MCCTL (0X09)  Multicast control register writes (CAEN ADCS).
  -  BT_AMOD_CBLT  (0x0B)  Chained block transfer (CAEN adcs).

*/
int
CSBSBit3VmeInterface::GetMMAPAddressModifier(void* pDeviceHandle)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  bt_devdata_t     Result;
  bt_error_t         err     = bt_get_info(*p, BT_INFO_PIO_AMOD, &Result);
  CheckError(p, err, "CSBSBit3VmeInterface::GetMMAPAddressModifier failed");
  return Result;
}
/*!
   The SBS/Bit3 interface has hardware support for byte swapping.  Ordinarily,
   the interface is set up to transparently swap between the VME standard
   byte ordering (big endian), and PC (little endian) byte ordering.  This
   function returns the current byte swapping selection.
   \param pHandle (void*  [in]): Handle received from a successful call to 
      CVMEInterface::Open.
   \return SwapMode (bt_swap_t [in]) Swap mode, can be one of the follwoing:
   -  BT_SWAP_NONE           No swapping
   -  BT_SWAP_BSNBD          Byte swap, non byte data
   -  BT_SWAP_WS             Word swap
   -  BT_SWAP_WS_BSNBD       Word swap and byte swap  non btye data
   -  BT_SWAP_BSBD           Byte swap  btye data 
   -  BT_SWAP_BSBD_BSNBD     Btye swap  byte data and byte swap  non byte data
   -  BT_SWAP_BSBD_WS        Byte swap  byte data and word swap
   -  BT_SWAP_BSBD_WS_BSNBD  All possible swapping
   -  BT_SWAP_DEFAULT        Driver default swapping
 
   BT_SWAP_DEFAULT is set to transparently swap bytes in VME byte order (big 
   endian) to PC byte ordering (little endian).
   Note that this setting only affects DMA transfers.  At present, you have no
   control over the swap mode for memory mapping, and it is BT_SWAP_DEFAULT.

*/
int
CSBSBit3VmeInterface::GetSwapMode(void* pDeviceHandle)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  bt_devdata_t  Result;
  bt_error_t      err      = bt_get_info(*p, BT_INFO_SWAP, &Result);
  CheckError(p, err, "CSBSBit3VmeInterface::GetSwapMode failed");
  return Result;
}
/*!
  Return the current dma threshold.  CVMEInterface::Read or 
  CVMEInterface::Write calls that cause more than this threshold number of 
  bus transfers will engage the DMA engine on the SBS/Bit3 interface.  Fewer
  transactions will result in a programmed I/O transfer in the driver.
  
  Control over this setting allows control over the trade-off between the
  higher transfer rates of the DMA engine vs. programmed contrrol against
  the higher overhead required to setup a DMA transfer.

  \param pHandle (void* [in]): Handle returned from CVMEInterface::Open
  \return The threshold value.
*/
int
CSBSBit3VmeInterface::GetDMAThreshold(void* pDeviceHandle)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  bt_devdata_t  Result;
  bt_error_t      err    = bt_get_info(*p, BT_INFO_DMA_THRESHOLD, &Result);
  CheckError(p, err, "CSBSBit3VmeInterface::GetDMAThreshold failed");
  return Result;
}
/*!
  Return the DMA Poll ceiling.  This value determines the number of 
  transfers performed by a CVMEInterface::Read or CVMEInterface::Write 
  operation before using interrupts to synchronize completion.  This
  value represents a trade-off between the improved latency of polling for
  completion and the worsened system response of doing so.  

  \param pHandle (void* [in]): Handle returned from CVMEInterface::Open
  \return The threshold value.
*/
int
CSBSBit3VmeInterface::GetDMAPollCeiling(void* pDeviceHandle)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  bt_devdata_t  Result;
  bt_error_t      err    = bt_get_info(*p, BT_INFO_DMA_POLL_CEILING, &Result);
  CheckError(p, err, "CSBSBit3VmeInterface::GetDMAPollCeiling failed");
  return Result;
}
/*!
    Gets the current debugging trace mask for the device driver.
    The driver is extensively instrumented with debugging trace code.
    The amount of information traced to the system log files is controled
    by a trace mask.  This function returns the current value of that mask.
    Bits in this mask represent a trade off between the extent of debug
    support and the driver overhead required to provide it.
    \param pHandle (void* [in]): Handle returned from CVMEInterface::Open
    \return The value of the trace mask. This is a bit mask make up of the
    logical or of the following bits:
  -  BT_TRC_ERROR     Fatal system errors, shouldn't happen
  -  BT_TRC_WARN      driver errors that may happen due to external conditions
  -  BT_TRC_INFO      Messages for errors returned to the user
  -  BT_TRC_CFG       driver configuration and initialization
  -  BT_TRC_RD_WR     generic read/write (outside of DMA & PIO)
  -  BT_TRC_MMAP      memory mapping and related code/
  -  BT_TRC_BIND      bt_bind() and related code 
  -  BT_TRC_CTRL      All ioctl() functions 
  -  BT_TRC_OPEN      Driver open and close
  -  BT_TRC_ISR       Interrupt Service Routine
  -  BT_TRC_IRQ_DISP  Interrupt Request Processing/Dispatch
  -  BT_TRC_ICBR      user ICBR and the vector returned by applicable kernel
                      interrupt handlers.
  -  BT_TRC_ISR_ERR   ISR errors only
  -  BT_TRC_DMA       DMA, each transfer
  -  BT_TRC_PIO       PIO, each transfer
  -  BT_TRC_FUNC      function entry and exit
  -  BT_TRC_ALLOC     Allocation/release of resources
  -  BT_TRC_LOCK      all locks (user & driver internal)
  -  BT_TRC_MAPREG    details of each map register load 
  -  BT_TRC_RAWIO     Raw I/O register access
  -  BT_TRC_KLIB      Misc. shared kernel routines
  -  BT_TRC_KDDI      All 3rd party driver rouitnes KDDI
  -  BT_TRC_LIO       Low level I/O register rd/wr 
  -  BT_TRC_PROFILE   enable H/W timing marks for profiling
  -  BT_TRC_DETAIL    Detailed trace of enabled function 
  -  BT_TRC_HW_DEBUG  enable HW trace messages instead of console 
*/
int
CSBSBit3VmeInterface::GetTraceMask(void* pDeviceHandle)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);


  bt_devdata_t Result;
  bt_error_t     err      = bt_get_info(*p, BT_INFO_TRACE, &Result);
  CheckError(p, err, "CSBSBit3VmeInterface::GetTraceMask Failed");
  return Result;
}
/*! 

    Does a sysreset on the VME bus.  This can be done to return devices
    on the VME bus to a known state.  Note that this function has \em very 
    global effect and should be used with caution.
    \param pHandle (void* [in]): Handle returned from CVMEInterface::Open.

*/
void
CSBSBit3VmeInterface::ResetVme(void* pDeviceHandle)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  bt_error_t   err    = bt_reset(*p);
  CheckError(p, err ,"CSBSBit3VmeInterface::ResetVme failed");
}

/*!
   Check for errors on the Bit3 device.  Errors must be reset with a call
   to CheckErrors.

   \return bt_error_t
   \retval BT_SUCCESS - no errors.
   \retval BT_ESTATUS - There are status errors.
   \retval BT_ENOPWR  - VME crate is off.
   \retval BT_EPWRCYC - Power just turned on.
   \retval BT_IO      - driver could not be queried.

*/
bt_error_t
CSBSBit3VmeInterface::CheckErrors(void* pDeviceHandle)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  return  bt_chkerr(*p);
}
/*!
    Reset errors on the interface.
    \return bt_error_t
    \retval BT_SUCCESS - success.
    \retval BT_ESTATUS - Status errors  persist.
    \retval BT_ENOPWR  - Power is still off.
    \retval BT_IO      - I/O failure.
 */
bt_error_t 
CSBSBit3VmeInterface::ClearErrors(void* pDeviceHandle)
{
  pDevHandle pHandle = reinterpret_cast<pDevHandle>(pDeviceHandle);
  bt_desc_t* p = &(pHandle->s_handle);

  return bt_clrerr(*p);
}
