/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2013.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/*!
   Provides device/driver independent interfaces to the VME bus.
   This is implemented as a class with wholey static members for the following
   reasons:
   - Class provides a namespace context to prevent global namespace polution.
   - Class allows for future optimizations that support 'sharing' address space
   mappings.

    The basic functions we support are:
    - Map an address space.
    - Unmap an address space.
    - Do a block read
    - Do a block write.

    The address modes supported are:
    A16/D32        - Short I/O space.
    A24/D32        - Standard memory.
    A32/D32        - Extended memory.
    GEO/D32        - Geographical addressing.
    MCST/D32       - Multicast control register access.
    CBLT/D32       - Chained block transfer.

    \note Assumptions:
    - The goal is simplicity.
    - There is only one driver on each system.
    - All address spaces are open read/write.
    - The goal is simplicity!


*/
/*
  Author:
    Ron Fox
    NSCL
    Michigan State University
    East Lansing, MI 48824-1321
    fox@nscl.msu.edu

    $Header$

    Change history:

    $Log$
    Revision 8.3  2006/02/17 18:35:04  ron-fox
    Add in the CVMEInterface::{Read,Write} functions missing from the
    initial code pass.

    Revision 8.2  2005/06/24 11:32:39  ron-fox
    Bring the entire world onto the 8.2 line

    Revision 4.1  2004/11/08 17:37:47  ron-fox
    bring to mainline

    Revision 3.2  2003/10/31 17:50:04  ron-fox
    Added CVMEInterface::Lock and CVMEInterface::Unlock
    to support serialization between mmap and
    block transfers.

    Revision 3.1  2003/03/22 04:04:03  ron-fox
    Added SBS/Bit3 device driver.

    Revision 2.1  2003/02/11 16:46:44  ron-fox
    Retag to version 2.1 to remove the weird branch I accidently made.

    Revision 1.1.1.1  2003/02/05 14:04:29  ron-fox
    Initial import of the NSCL Daq clients version 7.0-000 to sourceforge.


    Revision 2.5  2002/11/20 16:06:59  fox
    Add Multiple crate support

    Revision 2.4  2002/11/13 18:56:26  fox
    Add support for multiple crates

    Revision 2.3  2002/10/09 11:28:17  fox
    Add copyright/license stamp.

 * Revision 2.2  2002/07/02  15:13:56  fox
 * Go to 2.xx based releases (recover from client crash too).
 *
 * Revision 2.1  2002/07/02  15:08:16  fox
 * Transition to 2.1 releases
 *
 * Revision 1.1  2002/06/28  20:18:55  fox
 * Add device independent VME interfaces and add them to the
 * VMETcl facility.
 *
*/

#ifndef __VMEINTERFACE_H
#define __VMEINTERFACE_H


class CVMEInterface
{
 public:
  // Type definitions:

  typedef enum {			// Addressing modes:
    A16,
    A24,
    A32,
    GEO,
    MCST,
    CBLT,
    LAST,
    ShortIO   = A16,
    Standard  = A24,
    Extended = A32,
    Geographical = GEO,
    Multicast = MCST,
    ChainedBlock = CBLT
#ifdef HAVE_WIENERUSBVME_INTERFACE
    ,
    A24SuperBLT,
    A24UserBLT,
    A32SuperBLT,
    A32UserBLT,
    
#endif
  } AddressMode;

  static const char* m_szDriverName; // Name of driver (e.g. NSCLBiT3).

  static void* Open(AddressMode nMode,
		    unsigned short crate = 0); //!< Open a logical device.
  static void  Close(void* pDeviceHandle); //!< Close the logical device.
  static void* Map(void* pDeviceHandle,
	    unsigned long nBase, 
	    unsigned long nBytes); //!< Map a chunk of the bus to pva.
  static void Unmap(void* pDeviceHandle,
	     void*  pBase,
	     unsigned long lBytes); //!< Unmap a previously mapped section.

  static int Read(void* pDeviceHandle,
		  unsigned long nOffset,
		  void*  pBuffer,
		  unsigned long nBytes); //!< Block read from VME.
  static int Write(void* pDeviceHandle,
		   unsigned long nOffset,
		   void* pBuffer,
		   unsigned long nBytes); //!< Block write to vme.
  static void Lock();
  static void Unlock();

  // These are used so that private locks can be supported.
  // Lock 0 is always the DMA/PIO lock.
  // Lock 1-7 are used for driver private functions.
  // 
  static void Lock(int semnum);
  static void Unlock(int semnum);
 private:
  static void AttachSemaphore();

};


#endif
