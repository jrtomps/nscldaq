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

#ifndef __CCBDCES8210SCALERTRIGGER_H
#define __CCBDCES8210SCALERTRIGGER_H

#ifndef __CSCALERTRIGGER_H
#include "CScalerTrigger.h"
#endif

// forward class definitions:

class CBD8210;

/*!
   Provides a scaler trigger for the CES 8210
   branch highway driver.  The INT 4 input is used
   so that this module can also be used simlutaneously
   to provide event triggers.

   The 8210 is characterized by a
   branch number that defaults to 0.

*/
class CCBDCES8210ScalerTrigger : public CScalerTrigger
{
private:
  CBD8210*  m_pController;
public:
  CCBDCES8210ScalerTrigger(int b = 0);
  ~CCBDCES8210ScalerTrigger();
private:
  CCBDCES8210ScalerTrigger(const CCBDCES8210ScalerTrigger& rhs);
  int operator==(const CCBDCES8210ScalerTrigger& rhs);
  int operator!=(const CCBDCES8210ScalerTrigger& rhs);
public:

  // Overrides to produce a trigger:

  virtual bool operator()();
  virtual void Initialize();
};

#endif
