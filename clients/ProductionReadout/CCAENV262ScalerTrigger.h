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

#ifndef __CCAENV262SCALERTRIGGER_H
#define __CCAENV262SCALERTRIGGER_H

#ifndef __CSCALERTRIGGER_H
#include "CScalerTrigger.h"
#endif


// Forward class refs:

class CCaenIO;

/*!
   This class is a scaler trigger that uses a CAEN V262 I/O register.
   inputs and outputs are used identically to the event trigger.

   WARNING - be sure this module is at a different base address than
   the event trigeger.

*/
class CCAENV262ScalerTrigger : public CScalerTrigger
{
private:
  CCaenIO*  m_pIO;		// Pointer to the i/o Module.
public:
  CCAENV262ScalerTrigger(unsigned long base, unsigned long vmeCrate = 0);
  virtual ~CCAENV262ScalerTrigger();

  // Disallowed canonicals.

private:
  CCAENV262ScalerTrigger(const CCAENV262ScalerTrigger& rhs);
  CCAENV262ScalerTrigger& operator=(const CCAENV262ScalerTrigger& rhs);
  int operator==(const CCAENV262ScalerTrigger& rhs);
  int operator!=(const CCAENV262ScalerTrigger& rhs);
public:

  // overrides that implement our functionality:

  virtual  bool operator()();
  virtual  void Initialize();
};


#endif
