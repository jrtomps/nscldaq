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

#ifndef __NULLVMEINTERFACE_H
#define __NULLVMEINTERFACE_H

#ifndef __CVMEINTERFACE_H
#include "CVMEInterface.h"
#endif

// This class is intended for testing only... it is a minimal
// VME Interface.

class nullVMEInterface : public CVMEInterface
{
  bool m_fLocked;
public:

  nullVMEInterface();

  // overrides:

  virtual std::string deviceType()      const;
  virtual void*       getDeviceHandle() const;
  virtual CVMEAddressRange*  createAddressRange(unsigned short addressModifier,
						unsigned long  baseAddress,
						size_t         bytes);
  virtual CVMEPio*           createPioDevice();
  virtual CVMEList*          createList();
  virtual CVMEDMATransfer*  createDMATransfer(unsigned short addressModifier,
					      CVMEInterface::TransferWidth  width,
					      unsigned long  base,
					      size_t         units);

  virtual void onLock();
  virtual void onUnlock();
  bool locked();
};

#endif
