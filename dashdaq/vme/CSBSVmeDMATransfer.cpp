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

#include <config.h>
#include "CSBSVmeDMATransfer.h"
#include "CSBSVmeException.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

/*!
   Construct a dma transfer block.  We only fill in the
   handle the other data members are used during the transfer.
   \param handle   : bt_desc_t [in]
      Handle open on the device.  This can be gotten from an
      CSBSVMEInterface object, or from a direct call to bt_open()
  \param addressModifier : unsigned short [in]
      Address modifier that selects an address space for the transfer.
   \param width : CVMEInterface::TransferWidth [in]
      Width at which transfers should take place.
   \param base : long [in]
      Address to/from which transfers will take place.
   \param length : size_t [in]
      Number of bytes in the transfer.
*/
CSBSVmeDMATransfer::CSBSVmeDMATransfer(bt_desc_t                    handle,
				       unsigned short               addressModifier,
				       CVMEInterface::TransferWidth width,
				       unsigned long                base, 
				       size_t                       length) :
  CVmeDMATransfer(addressModifier, width, base, length),
  m_handle(handle),
  m_blockMode(false)
{}

/*!
   The destructor is required for destructor chaining back to the base class.
*/
CSBSVmeDMATransfer::~CSBSVmeDMATransfer()
{
}

/*!
    Read a block of data from the VME
    To do this we set the Driver parameters as per our transfer,
    do the transfer and then restore them.
    Errors are reported via CSBSVmeException

    \param buffer : void * [out]
        Pointer to the data area into which the read data are put.
    \return size_t  - The number of bytes transferred.
*/
size_t
CSBSVmeDMATransfer::Read(void* buffer)
{
  setDriverParameters();

  size_t readLength;
  bt_error_t status = bt_read(m_handle, buffer, base(), length(), &readLength);

  restoreDriverParameters();

  if (status != BT_SUCCESS) {
    throw CSBSVmeException(status,
			   "CSBSVmeDMATransfer::Read - bt_read failed");
  }
  return readLength;
}
/*!
   Write a block of data to the VME
   To do this we set the driver parameters as per our transfer,
   do the transfer, and then restore the driver parameters.

   \param buffer : void* [in]
      Pointer to the location from which the data comes.
   \return size_t  - Number of bytes actually written.

*/
size_t
CSBSVmeDMATransfer::Write(void* buffer)
{
  setDriverParameters();

  size_t readLength;
  bt_error_t status = bt_write(m_handle, buffer, base(), length(), &readLength);

  restoreDriverParameters();

  if (status != BT_SUCCESS) {
    throw CSBSVmeException(status,
			   "CSBSVmeDMATransfer::Read - bt_read failed");
  }
  return readLength;
}


/*!
   Enable/disable block transfer modes.
*/
void
CSBSVmeDMATransfer::setBlockmode(bool enable)
{
  m_blockMode = enable;
}
////////////////////////////////////////////////////////////////

// Set the driver parameters according to what is required of our
// transfer.  Old parameters are stored in the m_old* vars for
// restoration with restoreDriverParameters.
//

void
CSBSVmeDMATransfer::setDriverParameters()
{
  bt_devdata_t amod = (bt_devdata_t)modifier();
  bt_width_t   transferWidth = BT_WIDTH_ANY;
  switch (width()) {
  case CVMEInterface::TW_8:
    transferWidth = BT_WIDTH_D8;
    break;
  case CVMEInterface::TW_16:
    transferWidth = BT_WIDTH_D16;
    break;
  case CVMEInterface::TW_32:
    transferWidth = BT_WIDTH_D32;
    break;
  }

  // Get old params: 

  bt_get_info(m_handle, BT_INFO_DATAWIDTH, &m_oldWidth);
  bt_get_info(m_handle, BT_INFO_DMA_AMOD,  &m_oldAm);
  bt_get_info(m_handle, BT_INFO_BLOCK,     &m_oldBlockmode);

  // Set new params:

  bt_set_info(m_handle, BT_INFO_DATAWIDTH, (bt_devdata_t)transferWidth);
  bt_set_info(m_handle, BT_INFO_DMA_AMOD,  amod);
  bt_set_info(m_handle, BT_INFO_BLOCK,      m_blockMode);


}
// Restore driver parameters from the m_old* data.

void
CSBSVmeDMATransfer::restoreDriverParameters()
{
  bt_set_info(m_handle, BT_INFO_DATAWIDTH, m_oldWidth);
  bt_set_info(m_handle, BT_INFO_DMA_AMOD,  m_oldAm);
  bt_set_info(m_handle, BT_INFO_BLOCK,     m_oldBlockmode);
}
