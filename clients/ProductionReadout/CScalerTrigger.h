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


#ifndef __CSCALERTRIGGER_H
#define __CSCALERTRIGGER_H

/*!
    CScalerTrigger is an abstract base class for all scaler trigger classes.
    It defines the interface of all objects that may provide triggers
    to read out scaler data.
*/
class CScalerTrigger
{
public:
  CScalerTrigger();
  virtual ~CScalerTrigger();
private:
  CScalerTrigger(const CScalerTrigger& rhs);
  CScalerTrigger& operator=(const CScalerTrigger& rhs);
  int operator==(const CScalerTrigger& rhs) const;
  int operator!=(const CScalerTrigger& rhs) const;
public:

  //  The members below are usually overridden by concreate subclasses.
  //  only operator() must be overridden, all others have a default no-op
  //  implementation:

  virtual bool operator()()  = 0;
  virtual void Initialize();
  virtual void Cleanup();
};

#endif
