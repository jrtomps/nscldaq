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

//////////////////////////CRunVariableDumpTrigger.h file//////////////////////////////////

#ifndef __CRUNVARIABLEDUMPTRIGGER_H  
#define __CRUNVARIABLEDUMPTRIGGER_H
                               
#ifndef __CTIMEDEVENT_H
#include "CTimedEvent.h"
#endif

class CExperiment;
                               
/*!
   Encapsulates the trigger of a run variable buffer.
   Run variables are variables which are periodically
   logged throughout  an active run.  They can be used e.g.
   to log detector temperatures, slow control system values etc.
   This class encapsulates the production of a run variable
   buffer trigger.  The trigger is caught by the experiment
   readout component.
 */		
class CRunVariableDumpTrigger  : public CTimedEvent        
{ 
private:
  CExperiment& m_rExperiment;
public:
	// Constructors, destructors and other cannonical operations: 

    CRunVariableDumpTrigger (CExperiment& rExp); 
    CRunVariableDumpTrigger(const CRunVariableDumpTrigger& rhs); //!< Copy constructor.
     ~ CRunVariableDumpTrigger ( ) { } //!< Destructor.
private:
    CRunVariableDumpTrigger& operator= (const CRunVariableDumpTrigger& rhs); //!< Assignment
    int         operator==(const CRunVariableDumpTrigger& rhs) const; //!< Comparison for equality.
    int         operator!=(const CRunVariableDumpTrigger& rhs) const;
public:
	// Class operations:
public:
     void operator() ()  ;
 
};

#endif


