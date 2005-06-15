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
//  CCAENV977Status.h
//  Implementation of the Class CCAENV977Status
//  Created on:      07-Jun-2005 04:42:54 PM
//  Original author: Ron Fox
///////////////////////////////////////////////////////////

#if !defined(__CCAENV977STATUS_H)
#define      __CCAENV977STATUS_H

#ifndef __CSTATUSMODULE_H
#include <CStatusModule.h>
#endif

#ifndef __HISTOTYPES_H
#include <histotypes.h>
#endif

// forward definitions:

class CCAENV977;

/**
 * A status/busy class based on the CV977 module.  Outputs are used as follows:
 * Going ready is output 0
 * Going busy is output 1
 * @author Ron Fox
 * @version 1.0
 * @created 07-Jun-2005 04:42:54 PM
 */
class CCAENV977Status : public CStatusModule
{
private:
  CCAENV977& m_Module;
  
public:

  // Canonical operations.

  CCAENV977Status(ULong_t lBase, UShort_t nCrate=0);
  CCAENV977Status(CCAENV977& module);
  CCAENV977Status(const CCAENV977Status& rhs);
  virtual   ~CCAENV977Status();
  
  CCAENV977Status& operator=(const CCAENV977Status& rhs);
  int operator==(const CCAENV977Status& rhs) const;
  int operator!=(const CCAENV977Status& rhs) const;


  // Object operations.

  virtual   void      GoBusy();
  virtual   void      GoClear();
  virtual   void      ModuleClear();

  // Selector.

  CCAENV977& getModule();
 
  // Utility:

private:
  void PulseOutputs(UShort_t mask);
  
};


#endif
