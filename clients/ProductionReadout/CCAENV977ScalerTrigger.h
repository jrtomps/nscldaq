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

#ifndef __CCAENV977SCALERTRIGGER_H
#define __CCAENV977SCALERTRIGGER_H

#ifndef __CSCALERTRIGGER_H
#include <CScalerTrigger.h>
#endif


// forward classes:

class CCAENV977;

/*!
   Provides a scaler trigger via the last bit input of a CAEN V977 module.
*/
class CCAENV977ScalerTrigger : public CScalerTrigger
{
private:
  CCAENV977*  m_pModule;
public:
  CCAENV977ScalerTrigger(unsigned long base, unsigned int vmeCrate = 0);
  ~CCAENV977ScalerTrigger();
private:
  CCAENV977ScalerTrigger(const CCAENV977ScalerTrigger& rhs);
  CCAENV977ScalerTrigger& operator=(const CCAENV977ScalerTrigger& rhs);
  int operator==(const CCAENV977ScalerTrigger& rhs) const;
  int operator!=(const CCAENV977ScalerTrigger& rhs) const;
public:

  virtual void Initialize();
  virtual bool operator()();

};

#endif
