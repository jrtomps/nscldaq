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
static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";/*
  \class CNimout
  \file Nimout.cpp

  Implements a Nimout object. Nimout's are mapped to NIM modules
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
#include <config.h>
#include "Nimout.h"
#include <iostream>


using namespace std;


/*!
  \fn CNimout(UInt_t base)

  Purpose: 
     Basic constructor for type CNimout. Defines a
     VmeModule with CVME mapped at location 'base'
     and with length 'length'.

  \param UInt_t base - offset into Vme module at which to begin mapping
*/
CNimout::CNimout(UInt_t base) :
  CVmeModule(CVmeModule::a16d16, base, LENGTH),
  m_pStrobeRegister(0)
{
#ifdef HAVE_VME_MAPPING
  CVME<UShort_t> space = getCVME();    // Get address space
  m_pBase            = (UShort_t*)space.getgenptr(0);
  m_pStrobeRegister  = &(m_pBase[STROBE]);
  m_pControlRegister =  &(m_pBase[CONTROL]);
#endif
}


/*!
  \fn CNimout(const CNimout& aCNimout)

  Purpose: 
     Copy constructor for CNimout

  \param const CNimout& aCNimout - the object to copy to this
*/
CNimout::CNimout(const CNimout& aCNimout) :
  CVmeModule(aCNimout)
{
}

/*!
  \fn CNimout& operator= (const CNimout& aCNimout)

  Purpose: 
     operator= Assignment operator

  \param const CNimout& aCNimout - the object to assign to this
*/
CNimout&
CNimout::operator= (const CNimout& aCNimout)
{
  if(this == &aCNimout) return *this;
  CVmeModule::operator= (aCNimout);
  return *this;
} 

/*!
  \fn Int_t CNimout::operator== (const CNimout& aCNimout)

  Purpose: 
     operator== Equality operator.

  \param const CNimout& aCNimout - the object to check for equality with

  \return 1 if they are equal
          0 if they are not equal
*/
Int_t
CNimout::operator== (const CNimout& aCNimout)
{
  return (CVmeModule::operator== (aCNimout));
}

/*!
  \fn void CNimout::ClearAll()

  Purpose:
     Clears all contents of all registers
*/
void
CNimout::ClearAll()
{
  pokew(0, DATA);
  pokew(0, STROBE);
  pokew(0, TIMING);
  pokew(0, CONTROL);
}


/*!
  \fn void CNimout::WriteRegister(Register reg, UInt_t pattern)

  Purpose: 
     Writes the pattern to the register specified by the
     'reg' parameter. Register is an enumeration.

  \param Register reg - enumeration value indicating the register to write to
         UInt_t pattern - the bit pattern to write to the data register
*/
void
CNimout::WriteRegister(Register reg, UShort_t pattern)
{

  m_pBase[reg] = pattern;

}

/*!
  \fn Bool_t CNimout::SetStrobeLength(DFloat_t time_in_ns)

  Purpose: 
     Sets the strobe timing register to a value which corresponds
     to the parameter in nanoseconds. This may require a write to
     the T bit of the control status register.

     Note: The smallest possible value for time_in_ns is 62.5nS,
           and all possible values are multiples of 62.5nS (if the
	   T bit is deasserted) or 4096uS (if the T bit is high). If
	   the parameter is NOT an even multiple of 62.5 (or 4096), 
	   then the time will be ROUNDED to the nearest multiple of
	   62.5 (or 4096). If the nearest multiple is 0 or is greater
	   than 268.43136sec, then function returns the value 0.

  \param DFloat_t time_in_ns - the time (in nanoseconds) to set
                               the strobe timing register to.

  \return 1 if the function was successful
          0 if the function was unsuccessful (see above note)
*/
Bool_t
CNimout::SetStrobeLength(DFloat_t time_in_ns)
{
  // Timing is incremented in 62.5ns increments
  UInt_t csr = peekw(CONTROL);  // current value of the c/s reg.
  UInt_t timing_pat;
  
  // If the time is less than 4096uS, we do not 
  // have to set the T bit. Otherwise we set it.
  if(time_in_ns <= 4096000) 
    {
      pokew((csr & 0xfffffffd), CONTROL);

      // Note that this division will cause ROUNDING 
      // if time_in_ns is not a multiple of 62.5! 
      timing_pat = (UInt_t)(time_in_ns / 62.5);
      if(!timing_pat) return 0;
    }
  else 
    {
      pokew((csr | 0x2), CONTROL);

      // Note that this division will cause ROUNDING if
      // time_in_ns is not a multiple of 4096000 (4096us)! 
      timing_pat = (UInt_t)(time_in_ns / 4096000);
      if((timing_pat > 65536) ||
	 (timing_pat < 0)) 
	return 0;
    }

  // Now we set the global timing of the strobe
  pokew(timing_pat, TIMING);
  return 1;
}

/*
  \fn void CNimout::StrobeAll()

  Purpose:
     Sets the strobe register to all 1's
*/
void
CNimout::StrobeAll()
{
  *m_pStrobeRegister = 0xffff;
}

/*!
  \fn void CNimout::OrRegister(Register reg, UInt_t or_pattern)

  Purpose: 
     Logically or's the bit pattern held by register 'reg'
     with the parameter or_pattern. Throws a CRangeError exception
     if there is a range error.

  \param Register reg - enumeration indicating which register to 'or'
         UInt_t or_pattern - the bit pattern to or with the pattern
                             contained in the register 'reg'
*/
void
CNimout::OrRegister(Register reg, UShort_t or_pattern)
{
  try {
    UShort_t curr_pattern = peekw(reg);
    pokew((curr_pattern | or_pattern), reg);
  }
  catch(CRangeError& re) {
    throw re;
  }
}

/*!
  \fn void CNimout::AndRegister(Register reg, UShort_t and_pattern)

  Purpose: 
     Logically and's the bit pattern held by register 'reg'
     with the parameter and_pattern. Throws a CRangeError exception
     if there is a range error.

  \param Register reg - enumeration indicating which register to 'and'
         UShort_t and_pattern - the bit pattern to and with the pattern
	                        contained in the register 'reg'.
*/
void
CNimout::AndRegister(Register reg, UShort_t and_pattern)
{
  try {
    UShort_t curr_pattern = peekw(reg);
    pokew((curr_pattern & and_pattern), reg);
  }
  catch(CRangeError& re) {
    throw re;
  }
}

/*!
  \fn Bool_t CNimout::SetBit(Register reg, UShort_t bit_num)

  Purpose: 
     Asserts the bit bit_num of register reg. Returns 0 if
     bit_num is larger than 15 (the highest bit in a register).
     Throws a CRangeError exception if there is a range error.

  \param Register reg - enumeration indicating which register to write to
         UShort_t bit_num - bit number to assert

  \return 1 if the operation was successful
          0 if the operation was unsuccessful
*/
Bool_t
CNimout::SetBit(Register reg, UInt_t bit_num)
{
  if(bit_num > 15) return 0;
  
  try {
    UShort_t curr_reg = peekw(reg);
    curr_reg = (curr_reg | (1 << bit_num));
    pokew(curr_reg, reg);
  }
  catch(CRangeError& re) {
    throw re;
  }

  return 1;
}

/*!
  \fn Bool_t CNimout::ClearBit(Register reg, UShort_t bit_num)

  Purpose: 
     Deasserts the bit bit_num of register reg. Returns 0 if
     bit_num is larger than 15 (the highest bit in a register).
     Throws a CRangeError exception if there is a range error.

  \param Register reg - enumeration indicating which register to write to.
         UShort_t bit_num - bit number to deassert.
*/
Bool_t
CNimout::ClearBit(Register reg, UInt_t bit_num)
{
  if(bit_num > 15) return 0;

  try {
    UShort_t curr_reg = peekw(reg);
    UShort_t mask     = ~(1 << bit_num);
    curr_reg        = (curr_reg & mask);
    pokew(curr_reg, reg);
  }
  catch(CRangeError& re) {
    throw re;
  }

  return 1;
}

/*!
  \fn void CNimout::TransferData()

  Purpose: 
     Asserts the C/S bit of the Control Status Register.
     Data is transferred when the C/S bit is set. The module
     is ready for new data when the status bit reads zero.
*/
void
CNimout::TransferData()
{

  *m_pControlRegister |= 0x01;

}

/*!
  \fn void CNimout::SetTBit()

  Purpose: 
     Asserts the T bit of the Control Status Register.
     When the T bit is low, the bit weight of the register is
     62.5ns, and full scale is 4096us. When the T bit is high,
     the bit weight of the register is 4096us, and full scale
     is 268.43146 sec.
*/
void
CNimout::SetTBit()
{
  UShort_t curr_reg = peekw(CONTROL);
  pokew((curr_reg | 0x2), CONTROL);
}

/*!
  \fn void CNimout::ClearCBit()

  Purpose:
     Deasserts the C bit of the control/status register.
*/
void
CNimout::ClearCBit()
{
  UShort_t curr_reg = peekw(CONTROL);
  pokew((curr_reg & 0xfffe), CONTROL);
}

/*!
  \fn void CNimout::ClearTBit()

  Purpose:
     Deasserts the timing bit (T) of the control/status register.
*/
void
CNimout::ClearTBit()
{
  UShort_t curr_reg = peekw(CONTROL);
  pokew((curr_reg & 0xfffd), CONTROL);
}

/*!
  \fn UShort_t CNimout::ReadRegister(Register reg)

  Purpose:
     Reads the contents of register 'reg' from the Bira Nimout
     and returns the result. Throws a CRangeError exception if there
     is a range error.

  \param Register reg - the register enumeration indicating the register
                        to read from.

  \return The contents of the register
*/
UShort_t
CNimout::ReadRegister(Register reg)
{
  try {
    return peekw(reg);
  }
  catch(CRangeError& re) {
    throw re;
  }
}

/*!
  \fn Bool_t CNimout::ModuleReady()

  Purpose:
     Reads the contents of register CONTROL and determines
     whether or not the control/status bit is set. Returns
     1 if it is not set.

  \return The value of the control/status bit (1 or 0)
*/
Bool_t
CNimout::ModuleReady()
{
  return !(peekw(CONTROL) & 0x1);
}

/*!
  \fn Bool_t CNimout::TBitSet()

  Purpose:
     Reads the contents of register CONTROL and determines
     whether or not the timing bit is set. Returns 1 if the
     timing bit is set.

  \return The value of the timing bit (1 or 0)
*/
Bool_t
CNimout::TBitSet()
{
  return (peekw(CONTROL) & 0x2);
}

/*
  \fn void CNimout::PrintAll()

  Purpose:
     Displays contents of all registers on the screen.
*/
void
CNimout::PrintAll()
{
  cout << "Current contents of registers:" << hex << endl;
  cout << "\tDATA:    0x" << ReadRegister(DATA) << endl;
  cout << "\tSTROBE:  0x" << ReadRegister(STROBE) << endl;
  cout << "\tTIMING:  0x" << ReadRegister(TIMING) << endl;
  cout << "\tCONTROL: 0x" << ReadRegister(CONTROL) << endl;
}
