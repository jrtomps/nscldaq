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
#include "CCBDCES8210ScalerTrigger.h"
#include <CBD8210.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

/*!
   Construct a trigger.  We just need to 
   create our m_pController member.
*/
CCBDCES8210ScalerTrigger::CCBDCES8210ScalerTrigger(int b)  :
  m_pController(new CBD8210(b))
{
}

/*!
   Destroy a trigger.  We need to destroy our m_pController memmber
   to prevent memory leaks.
*/
CCBDCES8210ScalerTrigger::~CCBDCES8210ScalerTrigger()
{
  delete m_pController;
}
/*!
    Test for the trigger.  If one is present, it is also reset:
*/
bool
CCBDCES8210ScalerTrigger::operator()()
{
  // Unfortunately, no matter how we slice it there's a timing hole
  // that may cause us to accidently clear IT2.  This is because
  // an IT2 may come along while we are busy futzing around 
  // figuring out what to write to the IFR to reset IT4.
  //

  unsigned short mask = m_pController->ReadCsr();
  if (mask & CBD8210::IT4BIT) {
    m_pController->WriteIFR(mask & CBD8210::IT2BIT);
    return true;
  }
  return false;
}
/*!
   Initialize by clearing the IT4 interrupt.
   As much as possible, we attempt to leave the
   IT2 interrupt alone,but again we're plagued by
   the race conditions that may occur.
*/
void
CCBDCES8210ScalerTrigger::Initialize()
{
  // Copy the IT2 bit to the ifr..ensuring that the IT4
  // bit is 0.  This is the best we can do to avoid the race.

  m_pController->WriteIFR(m_pController->ReadCsr() & CBD8210::IT2BIT);

}
