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

///////////////////////////////////////////////////////////
//  CCAENV977Trigger.h
//  Implementation of the Class CCAENV977Trigger
//  Created on:      07-Jun-2005 04:42:55 PM
//  Original author: Ron Fox
///////////////////////////////////////////////////////////

#if !defined(__CCAENV977TRIGGER_H)
#define      __CCAENV977TRIGGER_H


#ifndef  __HISTOTYPES_H
#include <histotypes.h>
#endif


#ifndef __CTRIGGER_H
#include <CTrigger.h>
#endif

class CCAENV977;

/**
 * This module represents a trigger module based on the CAENV977 input register.
 * A trigger is assumed to happen whenever any bit in the single hit register is
 * active.
 * @author Ron Fox
 * @version 1.0
 * @created 07-Jun-2005 04:42:55 PM
 */
class CCAENV977Trigger : public CTrigger
{
private:
  CCAENV977& m_Module;
  UShort_t   m_LastPattern;
public:
  CCAENV977Trigger(ULong_t lBase, UShort_t nCrate = 0);
  CCAENV977Trigger(CCAENV977& module);
  CCAENV977Trigger(const CCAENV977Trigger& rhs);
  virtual ~CCAENV977Trigger();

  CCAENV977Trigger& operator=(const CCAENV977Trigger& rhs);
  int operator==(const CCAENV977Trigger& rhs) const;
  int operator!=(const CCAENV977Trigger& rhs) const;
  
  
  virtual bool operator()();
  UShort_t getTriggerPattern() const;

  // Utilities:

private:
  void SetupModule();		// Initialize module setup.
  
};


#endif 
