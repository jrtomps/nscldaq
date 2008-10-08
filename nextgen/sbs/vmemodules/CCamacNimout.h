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

//////////////////////////CCamacNimout.h file//////////////////////////////////

#ifndef __CCAMACNIMOUT_H  
#define __CCAMACNIMOUT_H
                               
#ifndef __CCAMACMODULE_H
#include "CCamacModule.h"
#endif
                               
/*!
   Encapsulates an SEC NIM output register.
   This module has 12 bits of NIMOUT.  The output
   bits are pulsed when the module is written with
   an F0.A0.  The Module's Write pointer is stored
   and all members are inline so that this class can be used
   in timing critical code.
 */		
class CCamacNimout  : public CCamacModule        
{ 
private:
      unsigned short* m_pWrite; //!< Pointer to the nimout data register.
 
public:
	// Constructors, destructors and other cannonical operations: 

    CCamacNimout (unsigned int b,
		  unsigned int c,
		  unsigned int n);
    CCamacNimout(const CCamacNimout& rhs); //!< Copy constructor.
     ~ CCamacNimout ( ) { }                //!< Destructor.

    CCamacNimout& operator= (const CCamacNimout& rhs); //!< Assignment
    int           operator==(const CCamacNimout& rhs) const; //!< Comparison for equality.
    int           operator!=(const CCamacNimout& rhs) const {
      return !(operator==(rhs));
    }

  // Selectors for class attributes:
public:
  
  unsigned short* getWrite() const {
    return m_pWrite;
  }
  
  // Mutators:
protected:  
  
  // Class operations:
public:  
  void WriteMask (unsigned short mask)  ;
  void WriteBit (unsigned int nBitno)  ;
  
};

#endif
