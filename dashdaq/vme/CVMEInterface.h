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

#ifndef __CVMEINTERFACE_H
#define __CVMEINTERFACE_H


#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

// forward classes:

class CVMEAddressRange;
class CVMEPio;
class CVMEList;
class CVmeDMATransfer;

/*!
    The CVMEInterface class is an abstract base class for
    classes that provide support for a Linux->VME interface
    subsystem.  VME interface objects are organized
    as an abstract factory.  Each concrete factory produces
    objects for several types of VME accesses:
    - Address Ranges - are intended to function as windows into a chunk
      of the VME address space for a specific address modifier.
      These are most efficiently implemented as mmap mappings.  Not
      all devices are capable of that sort of mapping, in that case it is
      recommended that some simulation be done based on e.g.
      a PIO subsystem.  The function canMap() returns true if the
      underlying device is capable of implementing this access mode as a
      map.
   - Programmed I/O - These are objects that are capable of single
     shot transfers from/to an arbitrary address in an arbitrary address
     modifier.  Typically these are implemented such that each operation
     requires interaction with a kernel level device driver.
   - Lists - These are stored lists of VME operations that can be 
     performed or, if possible, downloaded into the hardware for 
     execution that can be triggered either programmatically or on an
     external hardware event.  If the underlying device does not support
     lists it will simulate a 'direct list' using PIO objects typically.
     You can use the hasListProcessor() member to determine if the actual
     device supports a hardware list processor.
   - DMA block transfers - These represent ranges of addresses in a single
     address modifier which can only be completely read or completely written.
     These can be simulated using lists or PIO if the underlying device is not
     capable of DMA transfers.  Use the hasDMABlockTransfer() to determine if the
     underlying hardware can do block transfers.
*/
class CVMEInterface 
{
  // Data types 
public:
  typedef enum _TransferWidth {
    TW_8,
    TW_16,
    TW_32,
    TW_64
  } TransferWidth;
public:

  // Canonicals:
  
  virtual ~CVMEInterface();	// Required for base classes.

  // Capability inquiry.

  virtual bool canMap() const;
  virtual bool hasListProcessor() const;
  virtual bool hasDMABlockTransfer() const;

  // Overridable callbacks:

  virtual void onLock();
  virtual void onUnlock();
  
  // pure virtual: 

  virtual std::string deviceType()      const = 0;
  virtual void*       getDeviceHandle() const = 0;
  virtual CVMEAddressRange* createAddressRange(unsigned short addressModifier,
					       unsigned long  baseAddress,
					       size_t         bytes) = 0;
  virtual CVMEPio*          createPioDevice() = 0;
  virtual CVMEList*         createList() = 0;
  virtual CVmeDMATransfer*  createDMATransfer(unsigned short addressModifier,
					      TransferWidth  width,
					      unsigned long  base,
					      size_t         units) = 0;
};

#endif
