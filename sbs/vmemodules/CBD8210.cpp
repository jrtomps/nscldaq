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

static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";   
//////////////////////////CBD8210.cpp file////////////////////////////////////

#include "CBD8210.h"                  

// Manifest Constants:
				// Bits in the csr:
const static short csrIT4  =       1;// Int4 triggered.
const static short csrIT2  =       2; // Int2 triggered.
const static short csrMIT4 =       4; // int 4 masked off.
const static short csrMIT2 =       8; // Int 2 masked off.
const static short csrMLAM =    0x10; // LAM interrupts masked off.
const static short csrMTO  =    0x20; // Timeout buserro masked off.
const static short csrSY1  =    0x40; // SY1 jumper on.
const static short csrSY2  =    0x80; // SY2 jumper on.
const static short csrSY3  =   0x100; // SY3 jumper on.
const static short csrSY4  =   0x200; // SY4 jumper in.
const static short csrSY5  =   0x400; // SY5 jumper in.
const static short csrMNOX =   0x800; // NO X Bus error masked off.
const static short csrBD   =  0x1000; // Branch demand set.
const static short csrTO   =  0x2000; // Last operation timed out.
const static short csrX    =  0x4000; // X response from last branch operation.
const static short csrQ    =  0x8000; // Q Response from last branch operation.

/*!
   Default constructor.  This is called when declarations of the form e.g.:
   -  CBD8210  object;
   are performed.
*/
CBD8210::CBD8210 (unsigned int b) : 
  CCamacModule(b,0,29),
  m_pCsr(0),   
  m_pITF(0),  
  m_pCAR(0),
  m_pBTB(0),
  m_pBZ(0),
  m_pGL(0) 
{
  m_pCsr = (volatile unsigned short*)
                   MakePointer(0, 0, true);
  m_pITF = (volatile unsigned short*)
                   MakePointer(4,0, true);
  m_pCAR = (volatile unsigned short*) 
                   MakePointer(8, 0, true);
  m_pBTB = (volatile unsigned short*)
                   MakePointer(9, 0, true);
  m_pBZ  = (volatile unsigned short*)
                   MakePointer(9, 0, true);
  m_pGL  = (volatile unsigned long*)
                   MakePointer(10, 0);
} 
/*!
   Copy construction. This is invoked when e.g. an object is passed by value
   to a function.  The copy constructor makes a clone of the rhs object.
*/
CBD8210::CBD8210(const CBD8210& rhs) :
  CCamacModule(rhs),
  m_pCsr(rhs.m_pCsr),
  m_pITF(rhs.m_pITF),
  m_pCAR(rhs.m_pCAR),
  m_pBTB(rhs.m_pBTB),
  m_pBZ(rhs.m_pBZ),
  m_pGL(rhs.m_pGL)
{

}

/*!
   Assignment operation.  This member function supports assignment of
   an object of this class to an object of the same class.
*/
CBD8210& CBD8210::operator= (const CBD8210& aCBD8210)
{ 
    if (this != &aCBD8210) {
       CCamacModule::operator= (aCBD8210);
 
       m_pCsr = aCBD8210.m_pCsr;
       m_pITF = aCBD8210.m_pITF;
       m_pCAR = aCBD8210.m_pCAR;
       m_pBTB = aCBD8210.m_pBTB;
       m_pBZ = aCBD8210.m_pBZ;
       m_pGL = aCBD8210.m_pGL;

    }
    return *this;
}
/*!
   Equality test:
   */
int
CBD8210::operator==(const CBD8210& rhs) const
{
  return ((m_pCsr == rhs.m_pCsr)          &&
	  (m_pITF == rhs.m_pITF)          &&
	  (m_pCAR == rhs.m_pCAR)          &&
	  (m_pBTB == rhs.m_pBTB)          &&
	  (m_pBZ  == rhs.m_pBZ)           &&
	  (m_pGL  == rhs.m_pGL)           &&
	  CCamacModule::operator==(rhs));
}

// Functions for class CBD8210

/*!
    Returns true if the last operation on this branch 
    returned an X response.  The X response indicates
    that the module has 'responded' to the function code.


*/
bool 
CBD8210::Xtest()  const
{
  return (*m_pCsr & csrX);
}  

/*!
    Returns true if the last operation on this
    branch returned a valid Q. The meaning of
    the Q bit depends on the module and the
    function being performned.  Q's are often used
    to send the result of a test function back (e.g.
    test for lam), indicate the end of a block transfer
    and other module specific functions.
    


*/
bool 
CBD8210::Qtest()  const
{
  return (*m_pCsr & csrQ);
}  

/*!
    Returns true if the last operation on the branch timed out
    (The TO bit is set in the CSR).  This usually indicates
    that a crate which was offline was addressed.
    


*/
bool 
CBD8210::TimedOut()  const
{
  return (*m_pCsr & csrTO);
}  

/*!
    Returns true if a global demand is pending on the 
    branch. 


*/
bool 
CBD8210::BranchDemand()  const
{
  return (*m_pCsr & csrBD);
}  

/*!
    Sets the state of the MNox bit in the status register.
    When this bit is clear, operations which don't provoke
    an X response result in a VME bus error.  In general,
    this bit should be set disabling this bus error.
    
    \param fSet - true if the bit should be set.


*/
void 
CBD8210::MNoX(bool fset)  
{
  unsigned short csr = *m_pCsr;
  if(fset) {
    csr |= csrMNOX;		// Or in the MNOX bit.
  }
  else {
    csr &= ~csrMNOX;		// Remove the MNOX bit.
  }
  *m_pCsr = csr;

}  

/*!
    Controls the state of the MTO bit in the control
    status register.  When this bit is clear, Branch timeouts
    (accesses to nonexistent cratss) result in bus errors on 
    the VME bus.  This bit should normally be set as there's
    no way to catch these bus errors as an inline trap.
    
    \param fset - true to set the bit.


*/
void 
CBD8210::MTo(bool fset)  
{
  unsigned short csr = *m_pCsr;
  if(fset) {
    csr |= csrMTO;		// Or in the MNOX bit.
  }
  else {
    csr &= ~csrMTO;		// Remove the MNOX bit.
  }
  *m_pCsr = csr;
 
}  

/*!
    Controls the state of the MLAM bit in the CSR.
    When this bit is clear, a LAM produces an VME interrupt
    at IPL3.  This bit should normally be set as there is no
    time efficient way to catch these interrupts at process level.
    
    \param fset  - When true the MLAM bit will be set.




*/
void 
CBD8210::MLAM(bool fset)  
{
  unsigned short csr = *m_pCsr;
  if(fset) {
    csr |= csrMLAM;		// Or in the MNOX bit.
  }
  else {
    csr &= ~csrMLAM;		// Remove the MNOX bit.
  }
  *m_pCsr = csr;
 
}  

/*!
    When clear, a NIM true on the IT2 will create
    a VME interrupt at IPL2.  This bit should normally
    be set as there is no time efficient way to catch
    VME interrupts at process level.
    
    \param fset - If true MIT2 is set.


*/
void 
CBD8210::MIT2(bool fset)  
{
  unsigned short csr = *m_pCsr;
  if(fset) {
    csr |= csrMIT2;		// Or in the MNOX bit.
  }
  else {
    csr &= ~csrMIT2;		// Remove the MNOX bit.
  }
  *m_pCsr = csr;
 
}  

/*!
    When clear, a NIM true on IT4 will  produce
    a VME bus interrupt at IPL4.  Since there's no
    good time efficient way to catch these interrupts at
    process level, this bit should be set.
    
    \param fset - If true, the MIT4 bit will be set.


*/
void 
CBD8210::MIT4(bool fset)  
{
  unsigned short csr = *m_pCsr;
  if(fset) {
    csr |= csrIT4;		// Or in the MNOX bit.
  }
  else {
    csr &= ~csrIT4;		// Remove the MNOX bit.
  }
  *m_pCsr = csr;
 
}  

/*!
    Returns true if the IT2 bit is set in the CSR. 
    This bit is set to indicate that the IT2 input has 
    received a falling edge.  To reset the edge, 
    you must write the IFR with the IT2  bit.
    


*/
bool 
CBD8210::IT2()  const
{
  
  return (*m_pCsr & csrIT2);
}  

/*!
    True if the IT4 bit is set in the CSR.  This bit indicates that
    a falling NIM edge was presented to the IT4 input of the
    module.  To clear this latched bit the IFR must be written
    with the IT4 bit.
    


*/
bool 
CBD8210::IT4()  const
{
  return (*m_pCsr & csrIT4);
}  

/*!
    Returns the current value of the CSR.


*/
unsigned short 
CBD8210::ReadCsr()  const
{
  return *m_pCsr;
}  

/*!
    Writes the CSR with the mask of bits supplied:
    
    \param nMask - The mask of bits to write.


*/
void 
CBD8210::WriteCsr(unsigned short nMask)  
{
  *m_pCsr = nMask;
}  

/*!
    Writes the IFR (interrupt flag register) with  the
    specified mask.  The bits in this register reset the
    IT2 and IT4 bits in the CSR.
    
    \param nMask - Mask to write in the IFR.


*/
void 
CBD8210::WriteIFR(unsigned short nMask)  
{
  *m_pITF = nMask;
}  

/*!
    Returns the value of the BTB register.
    In practice, the bits in this register specify
    which crates are powered up and online.
    The map of these bits is:
    
               7      6      5      4       3      2      1    0
    +-----------------------------------------------------------------+
    |    ... Cr7 |  Cr6 | Cr5 | Cr4 | Cr3 | Cr2 | Cr1 | 0|
    +------------------------------------------------------------------+


*/
unsigned short 
CBD8210::ReadBTB()  const
{
  return *m_pBTB;
}  

/*!
    Returns the current value of the branch 
    graded demand.  This may have nothing
    to do with crate LAMs. To get a crate
    LAM, you should read the individual crate
    GL registers.


*/
unsigned long 
CBD8210::ReadGl()  const
{
  return *m_pGL;
}

/*!
   Performs a BZ cycle on the branch.
   */
void
CBD8210::InitBranch() const
{
  *m_pBZ = 0;
}
