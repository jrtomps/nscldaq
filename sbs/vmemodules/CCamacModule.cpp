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
static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";//////////////////////////CCamacModule.cpp file////////////////////////////////////

#include <config.h>
#include "CCamacModule.h"                  
#include <RangeError.h>
#include <CCamac.h>

#include <assert.h>


using namespace std;



// Manifest constants:

static const int BRANCH_RANGE[2] = {0,  7};
static const int CRATE_RANGE[2]  = {0,  7}; // Crate 0 is the branch controller
static const int SLOT_RANGE[2]   = {1, 31};
static const int SUB_RANGE[2]    = {0, 15};
static const int F_RANGE[2]      = {0, 31};
static const int READ_RANGE[2]   = {0,  7};
static const int WRITE_RANGE[2]  = {16, 23};
//                                  L  H   L   H
static const int CTL_RANGES[4]   = {8, 15, 16, 31}; // 2 sets of control ranges


static const int ShortBit        = 2; // Bit to turn on 16 bit operation.
static const int FShift          = 2; // Shift F by this left.
static const int AShift          = 7; // SHift A by this left.
static const int NShift          = 11; // SHift N byt this left.
static const int CShift          = 16; // Shift C by this left.
static const int BShift          = 23; // SHift B by this left.
/*!
  Construct a module in a crate somewhere.
  \param branch - CAMAC Branch in which the module lives.
  \param crate  - Crate within the branch.
  \param slot   - Slot within the crate

  \exception CRangeError - if the branch, crate or slot are illegal.
     
  \note - Crate controllers can be constructed with full slot addressing
          up to n=31.
*/
CCamacModule::CCamacModule (unsigned int branch,
			    unsigned int crate,
			    unsigned int slot) :
  m_nBranch(branch),   
  m_nCrate(crate),   
  m_nSlot(slot),
  m_pBase(0) 
{
  // validate the branch, crate, slot:

  if(!ValidBranch(branch)) {
    throw CRangeError(BRANCH_RANGE[0], BRANCH_RANGE[1], branch,
		      "Instantiating a CAMAC Module");
  }
  if(!ValidCrate(crate)) {
    throw CRangeError(CRATE_RANGE[0], CRATE_RANGE[1], crate,
		      "Instantiating a CAMAC Module");
  }
  if(!ValidSlot(slot)) {
    throw CRangeError(SLOT_RANGE[0], SLOT_RANGE[1], slot,
		      "Instantiating a CAMAC module");
  }

  // If necessary, map the branch.

  CCamac::BranchInit(branch);

  // Create the pointer to the module using MakePointer (second version).
  // the pointer is stored as a long* with the D16 bit cleared.

  m_pBase = MakePointer(crate, slot, 0, 0);
  assert(m_pBase);		// Should never be zero!.

  

  

} 
/*!
   Copy construction. This is invoked when e.g. an object is passed by value
   to a function.  The copy constructor makes a clone of the rhs object.

   \param rhs - the source of the copy.
*/
CCamacModule::CCamacModule(const CCamacModule& rhs) :
  m_nBranch(rhs.m_nBranch),
  m_nCrate(rhs.m_nCrate),
  m_nSlot(rhs.m_nSlot),
  m_pBase(rhs.m_pBase)
{

}


/*!
   Assignment operation.  This member function supports assignment of
   an object of this class to an object of the same class.

   \param aCCamacModule - rhs of the assignment.

   \return Reference to the lhs of the assignment. This allows = to be chained.
*/
CCamacModule& CCamacModule::operator= (const CCamacModule& aCCamacModule)
{ 
    if (this != &aCCamacModule) {

       m_nBranch = aCCamacModule.m_nBranch;
       m_nCrate = aCCamacModule.m_nCrate;
       m_nSlot = aCCamacModule.m_nSlot;
       m_pBase = aCCamacModule.m_pBase;

    }
    return *this;
}


/*!
  Compare for equality.  If equal, all members are equal.  The pointer
  is derived from b,c,n so it is not compared:
  */
int
CCamacModule::operator==(const CCamacModule& rhs) const
{
  return ((m_nBranch == rhs.m_nBranch)      &&
	  (m_nCrate  == rhs.m_nCrate)       &&
	  (m_nSlot   == rhs.m_nSlot));
}

// Functions for class CCamacModule

/*!
    Reads 24 bits of data from the module:
    \param f   - CAMAC function code must be in the set [0-7]
    \param a  - Subaddress to assert must be in the set [0-15].
    
    \exception CRangeError - if either the function code or the 
            subaddress are out of range.
    

	\param unsigned int f, unsigned int a

*/
unsigned long 
CCamacModule::Read(unsigned int f, unsigned int a)   const
{
  // Validate the function code:

  if(!isRead(f)) {
    throw CRangeError(READ_RANGE[0], READ_RANGE[1],
		      f, "CCamacModule::Read function code is not a read");
  }
  if(!ValidSubaddress(a)) {
    throw CRangeError(SUB_RANGE[0], SUB_RANGE[1], a,
		      "CCamacModule::Read Subaddress invalid");
  }

  //  Calculate the bcnaf pointer and dereference it.  Note that the
  // transfer is d16 so two transfers are required:

  volatile unsigned short* pFna = (unsigned short*)MakePointer(f,a);
  unsigned long   result = ((long)*pFna << 16) | ((long)pFna[1] & 0xffff);
  return result;
}  

/*!
    Transfers a 24 bit data item to the module. 
    \param f  - Function code to use for the transfer.
                      Must be in the set [16-23].
    \param a - Subaddress to use for transfer. Must be
                     in the set [0-15]
    \param d - Data to write to the module.
    
    \exception CRangeError - Thrown if either f or a are invalid for a write.
    
*/
void 
CCamacModule::Write(unsigned int f, unsigned int a, unsigned long d) const
{
  // Validate the f and a values:

  if(!isWrite(f)) {
    throw CRangeError(WRITE_RANGE[0], WRITE_RANGE[1], f,
		      "CCamacModule::Write f is not a write function");
  }
  if(!ValidSubaddress(a)) {
    throw CRangeError(SUB_RANGE[0], SUB_RANGE[1], a,
		      "CCamacModule::Write a is not a valid subaddress");
  }
  //  Compute the pointer for the write function. Since the
  //  operation is on a 16  bit bus we still must do 2 operations via a 
  //  short* pointer. Note VME is bigendian.

  volatile unsigned short* pF  = (unsigned short*)MakePointer(f,a);

  *pF++ = (unsigned short)((d >> 16) & 0xffff);
  *pF++ = (unsigned short)(d & 0xffff);
}  

/*!
    Performs a CAMAC control (non data transfer)
    operation on the module.  
    
    \param f  - Function code to execute.  This must
                     be in the set: [8-15,24-31].
    \param a - Subaddress at which the function is directed.
                    This must be in the range of [0-15].
    
    \exception CRangeError is thrown if f is not a control code or if
                      a is an invalid subadress.  

\note
   A minor kludge:  There are two valid ranges for f.  These will
   be encoded in the message and the lowest set will be supplied as the
   'valid lower/upper' ranges for the CRangeError throw.

  */
unsigned short 
CCamacModule::Control(unsigned int f, unsigned int a)   const
{

  // Validate f and A values:

  if(!isControl(f)) {
    throw CRangeError(CTL_RANGES[0], CTL_RANGES[1], f,
	"CCamacModule::Write f is not a control function {[8-15],[24-31]}");
  }
  if(!ValidSubaddress(a)) {
    throw CRangeError(SUB_RANGE[0], SUB_RANGE[1], a,
		      "CCamacModule::Write a is not a valid subaddress");
  }
 
  // Control functions are most efficiently executed by doing a 16 bit
  // write to the address:

  volatile unsigned short* pF = (volatile unsigned 
				 short*)MakePointer(f,a, true);
  return *pF;

}  

/*!
    Returns a pointer which when dereferenced appropriately
    will perform the selected CAMAC function.
    
    \param f  - Function to perform.
    \param a - Subaddress to which the function will be directed.
    \param isshort - true if the pointer is for a 16 bit camac operation false if for 24 bits.

	\param unsigned int f, unsigned int n, bool isshort=false

*/
unsigned long* 
CCamacModule::MakePointer(unsigned int f, unsigned int a, bool isshort)  const
{
  // The pointer is produced biased relative to m_pBase

  unsigned long pResult = (unsigned long)m_pBase;
  pResult += (unsigned long)(
			     (f << FShift) | (a << AShift) | 
			     (isshort ? ShortBit : 0));
  return (unsigned long*)pResult;
}  

/*!
    Returns a pointer which when derererenced will
    perform the appropriate CAMAC function on the 
    branch the module lives in.  This overload differs
    from the prior MakePointer by allowing any crate
    or slot in the branch to be referenced.
    
    \param c  - Crate to reference.
    \param n   - Slot to reference.
    \param f   -  Function code.
    \param a  - Subaddress.
    \param isshort - When true, the pointer is generated for a short access, when not, for a long.
    

	\param 

*/
unsigned long* 
CCamacModule::MakePointer(unsigned int c,
			  unsigned int n,
			  unsigned int f,
			  unsigned int a,
			  bool isshort)  const
{

  CCamac::BranchInit(m_nBranch);
  unsigned char* pBranch = (unsigned char*)CCamac::Base(m_nBranch);

  unsigned long offset  = (unsigned long)(  (c         << CShift) |
					    (n         << NShift) |
					    (a         << AShift) |
					    (f         << FShift) |
					    (isshort ? ShortBit : 0));
  unsigned char* pResult = pBranch + offset;
  return (unsigned long*)pResult;
 
}
/*!
**  Determines if a branch number is valid. This is a static member
    functionl

  \param branch - The branch number to check.
*/
bool 
CCamacModule::ValidBranch(unsigned int branch)
{
  return ((branch >= BRANCH_RANGE[0]) && (branch <= BRANCH_RANGE[1]));
}
bool 
CCamacModule::ValidCrate(unsigned int crate)
{
  return ((crate >= CRATE_RANGE[0]) && (crate <= CRATE_RANGE[1]));
}
/*!
   Determines if slot is a valid slot number.

   \param slot - Slot number to check.
   */
bool 
CCamacModule::ValidSlot(unsigned int slot)
{
  return ((slot >= SLOT_RANGE[0]) && (slot <= SLOT_RANGE[1]));
}
/*!
   Determines if a is  valid subaddress.
   \param a - Subaddress to check.
   */
bool 
CCamacModule::ValidSubaddress(unsigned int a)
{
  return((a >= SUB_RANGE[0]) && (a <= SUB_RANGE[1]));
}
/*!
   Determines if the selected function code is valid.
   \pararm f - the function code to check.
   */
bool 
CCamacModule::ValidFunction(unsigned int f)
{
  return ((f >= F_RANGE[0] ) && (f <= F_RANGE[1]));
}
/*!
  Determines if the specified function code is a Read function:
  \param f - Function to check.
  */
bool 
CCamacModule::isRead(unsigned int f)
{
  return ((f >= READ_RANGE[0]) && (f <= READ_RANGE[1]));
}
/*!
  Determines if the function is a write function code:
  \param f - Function code to check.
  */
bool 
CCamacModule::isWrite(unsigned int f)
{
  return ((f >= WRITE_RANGE[0]) && (f <= WRITE_RANGE[1]));
}
/*!
   Determines if a function code is a control function.
   \param f - The function to check.
   */
bool 
CCamacModule::isControl(unsigned int f)
{
  return ((!isRead(f)) && (!isWrite(f)));
}

