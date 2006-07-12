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

#ifndef __CWIENERVC32_H
#define __CWIENERVC32_H

#ifndef CCAMACINTERFACE_H
#include <CCAMACInterface.h>
#endif

#ifndef __CRT_UNISTD_H
#include <unistd.h>
#ifndef __CRT_UNISTD_H
#define __CRT_UNISTD_H
#endif
#endif


#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif


// Forward class definitions:

class CVMEAddressRange;
class CVC32CC32;
class CCAMACCrate;


/*!
   This class provides controller level support for the VC32 VME module.
   This VME module is always paired (at the NSCL) with a CC32 CAMAC crate controller.
   The module memory maps 1/2 the FNA space, the f16 bit is not mapped, writes
   are assumed to have f16 set, while reads are assumed to have f16 cleared.
*/
class CWienerVC32 : public CCAMACInterface
{
private:
  CVMEAddressRange&     m_Interface;
  CVC32CC32*            m_pCrate;
public:
  CWienerVC32(size_t vmecrate, uint32_t base);
  virtual ~CWienerVC32();
private:
  CWienerVC32(const CWienerVC32& rhs);
  CWienerVC32& operator=(const CWienerVC32& rhs);
  int operator==(const CWienerVC32& rhs) const;
  int operator!=(const CWienerVC32& rhs) const;
public:

  // operations:

public:
  virtual size_t maxCrates() const;
  virtual bool  haveCrate(size_t crate);
  virtual void addCrate(CCAMACCrate& crate, size_t number);
  virtual CCAMACCrate* removeCrate(size_t crate);
  virtual CCAMACCrate& operator[](size_t crate);
  virtual bool online(unsigned int crate);

  CVMEAddressRange& getAddressRange();
  off_t offset(size_t n, unsigned int f, unsigned int a);
 

  // Register access:
public:
  uint16_t readStatus();
  void     writeStatus(uint16_t value);

  uint16_t readCC32Status();
  void     C();
  void     Z();
  void     Inhibit();
  void     unInhibit();
  void     resetLamFlipFlop();
  void setBroadcastMask(uint32_t mask);
  void broadcast32(unsigned int f, unsigned int a, uint32_t data);
  void broadcast16(unsigned int f, unsigned int a, uint16_t data);
  uint32_t readLams();

  void     writeCycleTuneA(uint16_t value);
  void     writeCycleTuneB(uint16_t value);
  void     writeCycleTuneC(uint16_t value);

  uint16_t readCycleTuneA();
  uint16_t readCycleTuneB();
  uint16_t readCycleTuneC();
 
  void reset();
  

  // Register bit definition classes:

public:
  class VC32Status {
  public:
    static const uint16_t lamfifo = 0x8000;
    static const uint16_t online  = 0x4000;
    static const uint16_t size32k = 0x2000;
    static const uint16_t autoread= 0x1000;
    static const uint16_t roak    = 0x0800;
    
    // Remainder of the bits are for interrupts which we don't support.

  };

  class CC32Status {
  public:
    static const uint16_t typeMask = 0xf000; // Mask the type field
    static const uint16_t typeCC32 = 0x8000; // Module is a CC32.
    static const uint16_t typeVMEMM= 0x1000; // Module is a VME interface.
    static const uint16_t revMask  = 0x0f00;
    static const uint16_t revShift = 8;
    static const uint16_t moduleIdMask = 0x00f0;
    static const uint16_t moduleIdShift= 4;
    static const uint16_t Q        = 0x0008;
    static const uint16_t X        = 0x0004;
    static const uint16_t Inhibit  = 0x0002;
    static const uint16_t LamFlipFlop = 0x0001;
  };



};


#endif
