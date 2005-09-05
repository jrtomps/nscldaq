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


// This header defines VC-USB specific interfaces to the VME bus.
//

#ifndef __WIENERUSBVMEINTERFACE_H
#define __WIENERUSBVMEINTERFACE_H

/*!
   The WienerUSBVMEInterface class contains static functions
   that provide interface specific functionality.

    \bug  The Writexxx and Readxxx functions should be able to 
          run multiple stack operations if the count value of
	  their parameter is too large to run a single.
    \bug  The Writexxx and Readxxx functinos should be able to
          handle transactions that don't start longword aligned
          via a special start transaction followed by the 
          repeated set and a special stop transaction.
    \bug  The Writexx and Readxxx functions should be able to
          recognize a block transfer address modifier and
          generate a BLT transfer count or set of transactions
          with BLT transfer counts, rather than individual 
          transfer operations.
    \bug  The usbImmediateStackTransaction function should at some point
          have a timeout on the retries for usb_claim_inteface.
*/


class WienerUSBVMEInterface {
public:
  // These do the I/O in, if necessary multiple transactions and
  // don't have limits.

  static int ReadLongs(void* handle, unsigned long base, void* pBuffer, 
		       unsigned long count);
  static int ReadWords(void* handle, unsigned long base, void* pBuffer,
		       unsigned long count);
  static int ReadBytes(void* handle, unsigned long base, void* pBuffer,
		       unsigned long count);

  static int WriteLongs(void* handle, unsigned long base, void* pBuffer,
			unsigned long count);
  static int WriteWords(void* handle, unsigned long base, void* pBuffer,
			unsigned long count);
  static int WriteBytes(void* handle, unsigned long base, void* pBuffer,
			unsigned long count);

  // These do the I/O in a single transaction and have limits.
  static int atomicReadLongs(void* handle, unsigned long base, void* pBuffer, 
		       unsigned long count);
  static int atomicReadWords(void* handle, unsigned long base, void* pBuffer,
		       unsigned long count);
  static int atomicReadBytes(void* handle, unsigned long base, void* pBuffer,
		       unsigned long count);

  static int atomicWriteLongs(void* handle, unsigned long base, void* pBuffer,
			unsigned long count);
  static int atomicWriteWords(void* handle, unsigned long base, void* pBuffer,
			unsigned long count);
  static int atomicWriteBytes(void* handle, unsigned long base, void* pBuffer,
			unsigned long count);

  // I was considering making this private, but why  not make it available
  // to users with 'special' applications.

  static int usbImmediateStackTransaction(void* handle,
					  void* stack,
					  void* inputBuffer,
					  unsigned long readSize);
};

#endif
