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

/*
  \class CNimout
  \file Nimout.h

  Encapsulates a Nimout object. Nimout's are mapped to NIM modules
  using a CVME<UShort_t> object. These objects are specific to NIM
  modules which map to 16 bit address/16 bit data bus cards. Nimouts
  can be read from, and written to.

  Author:
     Jason Venema
     NSCL
     Michigan State University
     East Lansing, MI 48824-1321
     mailto: venemaja@msu.edu
*/

#ifndef __NIMOUT_H
#define __NIMOUT_H

#ifndef __VMEMODULE_H
#include <VmeModule.h>
#endif

class CNimout : public CVmeModule
{
  enum { LENGTH = 64 };
  UShort_t* m_pBase;
  UShort_t* m_pStrobeRegister;
  UShort_t* m_pControlRegister;

  // These are the registers in the bira nimout board. Their place
  // in the enumeration also happens to correspond their offset in
  // the address space.
 public:
  enum Register {
    DATA,
    STROBE,
    TIMING,
    CONTROL
  };

  // Default constructor
  CNimout(UInt_t base);
  CNimout(CVME<UShort_t>& am_CVME);
  
  // Copy Constructor
  CNimout(const CNimout& aCNimout);

  // Destructor
  virtual ~CNimout() { }

  // Operator= assignment operator
  CNimout& operator= (const CNimout& aCNimout);

  // Operator== Equality operator
  Int_t operator== (const CNimout& aCNimout);

  // Public member functions: Modify/read registers
 public:

  // Modify register bit patterns
  void ClearAll();
  void WriteRegister(Register reg, UShort_t pattern);
  Bool_t SetStrobeLength(DFloat_t time_in_ns);
  void StrobeAll();
  void OrRegister(Register reg, UShort_t or_pattern);
  void AndRegister(Register reg, UShort_t and_pattern);

  // Modify individual bits
  Bool_t SetBit(Register reg, UInt_t bit_num);
  Bool_t ClearBit(Register reg, UInt_t bit_num);
  void TransferData();
  void SetTBit();
  void ClearCBit();
  void ClearTBit();

  // Read registers
  UShort_t ReadRegister(Register reg);
  Bool_t ModuleReady();
  Bool_t TBitSet();
  void PrintAll();
};

#endif
