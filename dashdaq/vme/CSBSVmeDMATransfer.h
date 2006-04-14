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

#ifndef __CSBSVMEDMATRANSFER_H
#define __CSBSVMEDMATRANSFER_H

#ifndef __CVMEDMATRANSFER_H
#include <CVmeDMATransfer.h>	/*  base class */
#endif

#ifndef BT1003
#define BT1003
#endif

#ifndef __SBS_BTAPI_H
extern "C" {
#include <btapi.h>
}
#ifndef __SBS_BTAPI_H
#define __SBS_BTAPI_H
#endif
#endif



/*!
   Performs DMA transfers for SBS/Bit3 devices.
*/
class CSBSVmeDMATransfer : public CVmeDMATransfer
{
private:
  bt_desc_t     m_handle;
  bt_devdata_t  m_blockMode;

  bt_devdata_t  m_oldAm;
  bt_devdata_t  m_oldWidth;
  bt_devdata_t  m_oldBlockmode;

public:
  CSBSVmeDMATransfer(bt_desc_t handle,
		     unsigned short               addressModifier,
		     CVMEInterface::TransferWidth width,
		     unsigned long                base, 
		     size_t                       length);
  virtual ~CSBSVmeDMATransfer();

  // Disallow copy etc.

private:
  CSBSVmeDMATransfer(const CSBSVmeDMATransfer& rhs);
  CSBSVmeDMATransfer& operator=(const CSBSVmeDMATransfer& rhs);
  int operator==(const CSBSVmeDMATransfer& rhs) const;
  int operator!=(const CSBSVmeDMATransfer& rhs) const;

  // SBS Specific:

  void setBlockmode(bool enable);
public:


  // Override the read /write members:

  virtual size_t Read(void* buffer);
  virtual size_t Write(void* buffer);


  // utilities:

private:
  void setDriverParameters();
  void restoreDriverParameters();
};



#endif
