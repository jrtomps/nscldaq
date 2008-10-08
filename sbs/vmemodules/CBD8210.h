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

//////////////////////////CBD8210.h file//////////////////////////////////

#ifndef __CBD8210_H  
#define __CBD8210_H
                               
#ifndef __CCAMACMODULE_H
#include "CCamacModule.h"
#endif
                               
/*!
   Encapsulates a single CES CBD 8210 
   branch highway driver.  This appears
   as a module in slot 29 of crate 0.  The function
   codes correspond to device registers.  Some of the
   function codes do data transfer which should not do
   data transfer.  If that bothers you, then just think of
   the F.N as a way to describe a VME address rather than
   as F.N's.
   
 */		
class CBD8210  : public CCamacModule        
{
public:
  // Bits in the Interrupt flag register and CSR for interrupts:

  static const int IT2BIT = 2;
  static const int IT4BIT = 1;
private:
      volatile unsigned short* m_pCsr; //!< Pointer to the Control Status Register.
      volatile unsigned short* m_pITF; //!< Pointer to the Interrupt flag register.
      volatile unsigned short* m_pCAR; //!< Pointer to the Crate Address Register.
      volatile unsigned short* m_pBTB; //!< Pointer to the branch timing register.
      volatile unsigned short* m_pBZ; //!< Pointer to the Branch Zero register.
      volatile unsigned long* m_pGL; //!< Graded LAM register.
 
public:
	// Constructors, destructors and other cannonical operations: 

  CBD8210 (unsigned int b);	//!< Default constructor.
  CBD8210(const CBD8210& rhs);	//!< Copy constructor.
  ~ CBD8210 ( ) { }		//!< Destructor.
  
  CBD8210& operator= (const CBD8210& rhs); //!< Assignment
  int     operator==(const CBD8210& rhs) const; //!< Comparison for equality.
  int    operator!=(const CBD8210& rhs) const {
    return !(operator==(rhs));
  }
  
  // Selectors for class attributes:
public:
  
  volatile unsigned short* getCsr() const {
    return m_pCsr;
  }
  
  volatile unsigned short* getITF()  {
    return m_pITF;
  }
  
  volatile unsigned short* getCAR()  {
    return m_pCAR;
  }
  
  volatile unsigned short* getBTB()  {
    return m_pBTB;
  }
  
  volatile unsigned short* getBZ()  {
    return m_pBZ;
  }
  
  volatile unsigned long* getGL()  {
    return m_pGL;
  }
  
  // Mutators:
protected:  
  
  // Class operations:
public:  
  bool Xtest ()  const;
  bool Qtest ()  const;
  bool TimedOut ()  const;
  bool BranchDemand ()  const;
  void MNoX (bool fSet=true) ;
  void MTo (bool fSet=true)  ;
  void MLAM (bool fSet=true)  ;
  void MIT2 (bool fSet=true)  ;
  void MIT4 (bool fSet=true)  ;
  bool IT2 ()  const;
  bool IT4 ()  const;
  unsigned short ReadCsr ()  const;
  void WriteCsr (unsigned short nMask)  ;
  void WriteIFR (unsigned short nMask)  ;
  unsigned short ReadBTB ()  const;
  unsigned long ReadGl ()  const;
  void  InitBranch() const;
  
};

#endif
