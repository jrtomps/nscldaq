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
#include <config.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


#include <CTimedScalerTrigger.h>
#include <CExperiment.h>
#include <CReadoutMain.h>



/*!
   Construct a timed scaler trigger.  At construction time
   we just need to save the experiment object pointer.

   \param pExperiment CExperiment* 
       Pointer to the experiment
*/
CTimedScalerTrigger::CTimedScalerTrigger(CExperiment* pExperiment) :
  m_pExperiment(pExperiment),
  m_nLastTrigger(0),
  m_nNextTrigger(0),
  m_nTriggerInterval(0)
{
  
}
/*!
   Destructor is basically a no op.
*/
CTimedScalerTrigger::~CTimedScalerTrigger()
{
}

/*!
    Initialization requires that we figure out when 'now' is, the
    scaler periodicity and when we read the next scaler.
*/
void
CTimedScalerTrigger::Initialize()
{
  m_nLastTrigger     = m_pExperiment->GetElapsedTime(); // start timing from 'now'.
  m_nTriggerInterval = CReadoutMain::getInstance()->getScalerPeriod()*10; // 10'ths
  m_nNextTrigger     = m_nLastTrigger + m_nTriggerInterval;

}

/*!
   Check for the trigger.  We have a trigger if the current elaspsed time
   is >= the next trigger time.  In that case we'll need to compute the
   next trigger time as well as return true.
*/
bool
CTimedScalerTrigger::operator()()
{
  unsigned int now = m_pExperiment->GetElapsedTime(); 
  bool     triggered= false;
  if (now >= m_nNextTrigger) {
    triggered = true;
    m_nLastTrigger = now;
    m_nNextTrigger = now + m_nTriggerInterval;
  }
  return triggered;
}
