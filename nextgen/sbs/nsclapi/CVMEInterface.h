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

};


#endif
