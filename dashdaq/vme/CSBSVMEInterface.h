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

#ifndef __CSBSVMEINTERFACE_H
#define __CSBSVMEINTERFACE_H

#ifndef __CVMEINTERFACE_H
#include "CVMEInterface.h"
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
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


// Forward class definitions:

class CVMEPio;
class CVMEList;
class CVmeDMATransfer;

/*!
   This is the object factory that represents
   a PCI/VME interface based on the SBS PCI/VME bus bridge
   family.
*/
class CSBSVMEInterface : public CVMEInterface
{
private:
  bt_desc_t   m_handle;

public:
  // Constructors and canonicals...

  CSBSVMEInterface(unsigned short crate);
  virtual ~CSBSVMEInterface();

  // We cannot allow copy like stuff.
private:
  CSBSVMEInterface(const CSBSVMEInterface& rhs);
  CSBSVMEInterface& operator=(const CSBSVMEInterface& rhs);
  int operator==(const CSBSVMEInterface& rhs) const;
  int operator!=(const CSBSVMEInterface& rhs) const;
public:

  // Implementing the CVEMInterface pure virtuals.

public:
  // Capability inquiry.

  virtual bool canMap() const;
  virtual bool hasListProcessor() const;
  virtual bool hasDMABlockTransfer() const;

  // Overridable callbacks:

  virtual void onLock();
  virtual void onUnlock();
  
  // pure virtual: 

  virtual STD(string) deviceType()      const;
  virtual void*       getDeviceHandle() const;
  virtual CVMEAddressRange* createAddressRange(unsigned short addressModifier,
					       unsigned long  baseAddress,
					       size_t         bytes);
  virtual CVMEPio*          createPioDevice();
  virtual CVMEList*         createList();
  virtual CVmeDMATransfer*  createDMATransfer(unsigned short addressModifier,
					      TransferWidth  width,
					      unsigned long  base,
					      size_t         units);  

  // CSBSVMEInterface specific functions.

public:
  bt_devdata_t getLocalPartNumber();
  bt_devdata_t getRemotePartNumber();
  bt_devdata_t getMemorySize();
  void         setDriverTraces(bt_devdata_t traceFlags);
  bt_devdata_t getDriverTraces();
  void         reset();
  bt_error_t   checkError();  
  void         clearError();
  STD(string)  errorText(bt_error_t status);

private:
  void checkStatus(bt_error_t status,
		   const char* failureMessage);
};

#endif
