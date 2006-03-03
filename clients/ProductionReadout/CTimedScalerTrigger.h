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


#ifndef __CTIMEDSCALERTRIGGER_H
#define __CTiMEDSCALERTRIGGER_H
#include <CScalerTrigger.h>

//  Forward classes::

class CExperiment;

/*!
    This class is a scaler trigger that can be used to do a timed periodic
    trigger of scaler readouts..  This trigger will in general be registered
    by default and must be overidden by the user if they want something different.
    
    We depend on the following:
    - The application has a ge5tScalerPeriod() member
    - The experiment has a GetElapsedTime function.

*/
class CTimedScalerTrigger : public CScalerTrigger
{
private:
  CExperiment*    m_pExperiment;
  unsigned int    m_nLastTrigger;
  unsigned int    m_nNextTrigger;
  unsigned int    m_nTriggerInterval;

public:
  // Constructors and other canonicals.

  CTimedScalerTrigger(CExperiment* pExperiment);
  virtual ~CTimedScalerTrigger();
private:
  CTimedScalerTrigger(const CTimedScalerTrigger& rhs);
  CTimedScalerTrigger& operator=(const CTimedScalerTrigger& rhs);
  int operator==(const CTimedScalerTrigger& rhs) const;
  int operator!=(const CTimedScalerTrigger& rhs) const;
public:

  // Overrides:
 
  virtual void Initialize();
  virtual bool operator()();
};

#endif
