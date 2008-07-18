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

#ifndef __CMADC32_H
#define __CMADC32_h

#ifndef __CVMUSBCONFIGURABLEOBJECT_H
#include "CVMUSBConfigurableObject.h"
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif


// Forward class definitions:

class CVMUSB;
class CVMUSBReadoutList;


/*!
   The MADC32 is a 32 channel ADC module produced by Mesytec.
   This module will be used in single event mode.
   The following configuration parameters can be sued to tailor
   the module:

\verbatim
   Name                 Value type          Description
   -base                integer             Base address of the module in VME space.
   -id                  integer [0-255]     Module id (part of the module header).
   -ipl                 integer [0-7]       Interrupt priority level 0 means disabled.
   -vector              integer [0-255]     Interrupt vector.
   -timestamp           bool  (false)       If true enables the module timestamp.
   -gatemode            enum (separate,common)  Determines if the bank gates are
                                            independent or common.
   -holddelays          int[2]              Delay between trigger and gate for each bank.
   -holdwidths          int[2]              Lengths of generated gates.
   -gategenerator       bool                Enable gate generator (hold stuff)
   -inputrange          enum (4v,8v,10v)    ADC input range.
   -ecltermination      bool                Enable termination of the ECL inputs.
   -ecltming            bool                Enables ECL timestamp inputs
                                            (oscillator and reset).
   -nimtiming           bool                Enables NIM input for timestamp inputs
                                            (oscillator & rset).
   -timingsource        enum (vme,external)  Determines where timestamp source is.
   -timingdivisor       int [0-15]          Divisor (2^n) of timestamp clock
   -thresholds          int[32] [0-4095]    Threshold settings (0 means unused).
   -nimbusy             enum (busy, gate0, gate1 cbus) Sets what comes out the nim busy.
                                            default is ..well.. busy.
\endverbatim
*/
class CMADC32 : public CVMUSBConfigurableObject
{
private:
  int operator==(CMADC32& rhs) const;
  int operator!=(CMADC32& rhs) const;

  // The interface for CReadoutHardware:

public:
  virtual void onAttach();
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual CVMUSBConfigurableObject* clone() const;


};



#endif
