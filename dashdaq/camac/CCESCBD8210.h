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

#ifndef __CCESCBD8210_H
#define __CCESCBD8210_H
#include "CCAMACInterface.h"

class CCAMACCrate;
class CVMEAddressRange;


/*!
   CCESCBD8210 is a camac interface class for the CES CBD 8210
   VME CAMAC parallel highway driver.  This is a single
   slot VME board.  Each board can control up to 7 crates.
   8 of these boards can live in a single VME crate.
   The board itself has some register space.  Crates
   are memory mapped regions of A24 space.  Note that the
   module itself is D16 so D24 transfers must be done in 
   two 16 bit transfers.. high part first since the VME
   bus is intrinsically big endian.
*/
class CCESCBD8210 : public CCAMACInterface
{
  // Member data:
private:
  CVMEAddressRange*   m_pRegisters;
  CCAMACCrate*        m_Crates[8]; // Crates 1-7 are used.
  unsigned int        m_VMECrate;  // VME crate number.
  unsigned int        m_Branch;	   //  Branch number (interface in vme).

  // constructors and canonicals

public:
  CCESCBD8210(unsigned int vmeCrate, unsigned int branchNumber);
  virtual ~CCESCBD8210();


  // These are not implemented.
private:
  CCESCBD8210(const CCESCBD8210& rhs);
  CCESCBD8210& operator=(const CCESCBD8210& rhs);
  int operator==(const CCESCBD8210& rhs) const;
  int operator!=(const CCESCBD8210& rhs) const;
public:

  // final functions

public:
  unsigned long offset(size_t slot, unsigned int function,
		       unsigned int subaddress, bool shortTransfer=true);

  unsigned long base(unsigned crate);
  size_t        mapsize();
  void Z();
  unsigned short readCSR();
  void           writeCSR(unsigned short data);

  bool lastX(); 
  bool lastQ();


  void writeITF(unsigned short datum);

  unsigned short readBTB();

  // overridable functions


public:
  virtual bool         haveCrate(size_t crate);
  virtual void         addCrate(CCAMACCrate& crate, size_t number);
  virtual CCAMACCrate* removeCrate(size_t number);
  virtual CCAMACCrate& operator[](size_t index);
  virtual bool         online(size_t index);


  // Bits in the CSR:

public:
  static const unsigned short csr_Q      = 0x8000;
  static const unsigned short csr_X      = 0x4000;
  static const unsigned short csr_TO     = 0x2000;
  static const unsigned short csr_BD     = 0x1000;
  static const unsigned short csr_MNOX   = 0x0800;
  static const unsigned short csr_symask = 0x07c0;
  static const unsigned short csr_syshift= 6;
  static const unsigned short csr_MTO    = 0x0020;
  static const unsigned short csr_MLAM   = 0x0010;
  static const unsigned short csr_MIT2   = 0x0008;
  static const unsigned short csr_MIT4   = 0x0004;
  static const unsigned short csr_IT2    = 0x0002;
  static const unsigned short csr_IT4    = 0x0001;

  // Bits in the ITF register:

public:
  static const unsigned short itf_IT4    = 1;
  static const unsigned short itf_IT2    = 2;

};

#endif
