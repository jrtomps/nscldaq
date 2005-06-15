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
//  CCAENV977.h
//  Implementation of the Class CCAENV977
//  Created on:      07-Jun-2005 04:42:54 PM
//  Original author: Ron Fox
///////////////////////////////////////////////////////////

#if !defined(__CCAENV977_H)
#define __CCAENV977_H

#ifndef __HISTOTYPES_H
#include <histotypes.h>
#endif


// Forward definitions.

class CVmeModule;

/*!
 * Low level driver for the CCAENV977 I/O register module.  This class provides
 * member functions and state to interact with a single module.In addition to the
 * member shown, we'll need several static const members representing bits in the
 * control/status registers:
 * - Test Register:
 * - test_Clear    - clear test channel
 * - test_Mask    - Mask the test channel.
 * - test_OrMask - Mask the or of test into the or output.
 * - test_IrqMask - Mask test's generation of interrupts.
 * - test_Read     - The value of the test register.
 * 
 * ControlRegister:
 * 
 * - control_Pattern      - Run module in pattern mode.
 * - control_gateMask - Disable the gate.
 * - control_OrMask     -  Disable or output on front panel.
 * @author Ron Fox
 * @version 1.0
 * @created 07-Jun-2005 04:42:54 PM
 */
class CCAENV977
{
private:
	/**
	 * Device indpedendent way to access a set of VME addresses.
	 */
  CVmeModule& m_Module;


// Constants with class scope:
public:
    // bits in the test control register:
    
    static const UShort_t  test_Clear   = 1;
    static const UShort_t  test_Mask    = 2;
    static const UShort_t  test_OrMask  = 4;
    static const UShort_t  test_IrqMask = 8;
    static const UShort_t  test_Read    = 0x10;
    
    // Bits in the module constrol register:
    
    static const UShort_t control_Pattern  = 1;
    static const UShort_t control_gateMask = 2;
    static const UShort_t control_OrMask   = 4;

public:
    // Constructors and other canonical operations:
    
  CCAENV977(ULong_t lBase, UShort_t nCrate = 0);
  virtual ~CCAENV977();
  CCAENV977(const CCAENV977& rhs);
  CCAENV977& operator=(const CCAENV977& rhs);
  int operator==(const CCAENV977& rhs) const;
  int operator!=(const CCAENV977& rhs) const;
  
  // Functions of the object itself:
  
  UShort_t inputSet();
  void     inputSet(UShort_t value);
  UShort_t inputMask();
  void     inputMask(UShort_t mask);
  UShort_t inputRead();
  UShort_t singleHitRead();
  UShort_t multihitRead();
  UShort_t outputSet();
  void     outputSet(UShort_t pattern);
  UShort_t outputMask();
  void     outputMask(UShort_t mask);
  UShort_t interruptMask();
  void     interruptMask(UShort_t mask);
  void     outputClear();
  UShort_t singleHitReadAndClear();
  UShort_t multiHitReadAndClear();
  UShort_t testControlRegister();
  void     testControlRegister(UShort_t mask);
  UShort_t serialNumber();
  UShort_t firmwareLevel();
  void     controlRegister(UShort_t mask);
  UShort_t controlRegister();
  void     Reset();
  

};


#endif 
