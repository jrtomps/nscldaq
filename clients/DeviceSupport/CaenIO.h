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
  \class CCaenIO
  \file CaenIO.h

  Encapsulates a CAEN mod. v262 multipurpose i/o register card. Caen262's
  are mapped using a CVME<UShort_t> object. The module operates in the
  a24d16 vme space at base address 0x4444. The card consists of 4 NIM
  level inputs, 4 NIM level outputs, 4 140ns NIM pulsed outputs, and 16
  ECL levels. The read/write functions receive the input/output value to
  read from or write to, and those pararmeters must be between 0 and 3 for
  the NIM inputs/outputs. The ECL outputs are set by simply placing a bit
  pattern in its register.

  Author:
     Jason Venema
     NSCL
     Michigan State University
     East Lansing, MI 48824-1321
     mailto: venemaja@msu.edu
*/




#ifndef __CCAENIO_H
#define __CCAENIO_H

#include <config.h>


#ifndef __VMEMODULE_H
#include <VmeModule.h>
#endif

class CCaenIO : public CVmeModule
{
  UInt_t m_nOutputMask;  /*! each bit in the mask corresponds to a level
			   output. A 1 indicates the output is set. */
  enum { LENGTH = 256 };
  
 public:

  // Default constructor
  CCaenIO(UInt_t base, int nCrate = 0);
#if defined(HAVE_WIENERVME_INTERFACE) || defined(HAVE_WIENERUSBVME_INTERFACE)
#else
  CCaenIO(CVME<UShort_t>& am_CVME);
#endif
  // Copy constructor
  CCaenIO(const CCaenIO& aCCaenIO);

  // Destructor
  virtual ~CCaenIO() { }

  // Operator= Assignment operator
  CCaenIO& operator=(const CCaenIO& aCCaenIO);
  
  // Operator== Equality operator
  Int_t operator== (const CCaenIO& aCCaenIO);

  // Public member functions
 public:
 
  // Read from inputs
  UShort_t ReadInput(UInt_t input);
  UShort_t ReadInputs();
  
  // Writing to NIM pulse and level outputs
  void PulseOutput(UInt_t output);
  void SetLevel(UInt_t output);
  void ClearLevel(UInt_t output);
  void ClearAll();
  
  // Writing to ECL outputs
  void SetECL(UShort_t value);
  void ClearECL();
#ifdef HAVE_VME_MAPPING
  short* getInputPointer() const;
  short* getPulsedOutputPointer() const;
  short* getLatchedOutputPointer() const;
  short* getECLOutputPointer() const;
#endif
};

#endif
