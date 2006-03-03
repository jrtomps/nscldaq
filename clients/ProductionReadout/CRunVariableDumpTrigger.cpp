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


//////////////////////////CRunVariableDumpTrigger.cpp file////////////////////////////////////
#include <config.h>
#include "CRunVariableDumpTrigger.h"
#include "CExperiment.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
/*!
  Construct a trigger.
  \param rExperiment - A reference to the experiment object which will be triggered
                       when this event is scheduled.

 */
CRunVariableDumpTrigger::CRunVariableDumpTrigger (CExperiment& rExp) :
  m_rExperiment(rExp)
 
{

} 
/*!
   Copy construction. This is invoked when e.g. an object is passed by value
   to a function.  The copy constructor makes a clone of the rhs object.
*/
CRunVariableDumpTrigger::CRunVariableDumpTrigger(const CRunVariableDumpTrigger& rhs)  :
  m_rExperiment(rhs.m_rExperiment)
{

}
// Functions for class CRunVariableDumpTrigger

/*!
    Called when it's time to trigger a run variable
    trigger. 

*/
void 
CRunVariableDumpTrigger::operator()()  
{
  m_rExperiment.TriggerStateVariableBuffer();
  m_rExperiment.TriggerRunVariableBuffer();
}
