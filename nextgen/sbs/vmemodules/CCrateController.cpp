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
//////////////////////////CCrateController.cpp file////////////////////////////////////

#include <config.h>
#include "CCrateController.h"                  


using namespace std;



/*!
   Default constructor.  This is called when declarations of the form e.g.:
   -  CCrateController  object;
   are performed.
   \param b - branch the controller lives in.
   \param c - Crate the controller lives in.
*/
CCrateController::CCrateController (unsigned int b,
				    unsigned int c) :
  m_myBranch(b),
  m_Slot28(b,c,28),
  m_Slot30(b,c,30),
  m_Broadcast(b,c,26),
  m_Multicast(b,c,24)
{

} 
/*!
   Copy construction. This is invoked when e.g. an object is passed by value
   to a function.  The copy constructor makes a clone of the rhs object.
*/
CCrateController::CCrateController(const CCrateController& rhs) :
  m_myBranch(rhs.m_myBranch),
  m_Slot28(rhs.m_Slot28),
  m_Slot30(rhs.m_Slot30),
  m_Broadcast(rhs.m_Broadcast),
  m_Multicast(rhs.m_Multicast)
{

}

	//Operator= Assignment Operator alternative to compiler provided operator=  

/*!
   Assignment operation.  This member function supports assignment of
   an object of this class to an object of the same class.
*/
CCrateController& CCrateController::operator= (const CCrateController& rhs)
{ 
    if (this != &rhs) {
      m_myBranch = rhs.m_myBranch;
      m_Slot28   = rhs.m_Slot28;
      m_Slot30   = rhs.m_Slot30;
      m_Broadcast= rhs.m_Broadcast;
      m_Multicast= rhs.m_Multicast;
    }
    return *this;
}

// Functions for class CCrateController

/*!
    Performs a Crate Z operations.  The Z line in the
    crate  is strobed.   This is an N28.F26.A8
    


*/
void 
CCrateController::Z()  
{
  m_Slot28.Control(26,8);		// Z the crate.
}  

/*!
    Performs a Create C operations.
    The C line in the crate is strobed.
    This is N28.F26.A9
    


*/
void 
CCrateController::C()  
{
  m_Slot28.Control(26,9);	// C the crate.
}  

/*!
    Returns the contents of the Create Graded LAM
    register.  For NSCL configured controllers with the
    standard Graded LAM jumper header installed, the
    result is a bitmask which indicates which modules 
    have LAMs set in them.
    N30.F0.A0


*/
long 
CCrateController::Lams()  
{
  return m_Slot30.Read(0, 0);
}  

/*!
    Writes the Station number mask register.
    The 1302 suports multicast operations.  A
    multicast operation consists of selecting the
    modules to be affected by writing the SNR mask
    and then performing the desired F.A at N24.
    The member functions MulticastControl and MulticastWrite
    perform both of these operations and should be used.

	\param unsigned long nMask

*/
void 
CCrateController::WriteSnr(unsigned long nMask)  
{
  m_Slot30.Write(16, 8, nMask);
}  

/*!
    Removes the inhibit signal from the dataway.
    The state of the inhibit can be tested, and the I
    led on the module reflects it as well.
    

*/
void 
CCrateController::UnInhibit()  
{
  m_Slot30.Control(24, 9);
}  

/*!
    Disables the production of branch demands. 
    A branch demand is created whenever the
    GL register is non-zero when enabled.


*/
void 
CCrateController::DisableDemand()  
{
  m_Slot30.Control(24,10);
}  

/*!
    Sets the data way inhibit.


*/
void 
CCrateController::Inhibit()  
{
  m_Slot30.Control(26, 9);
}  

/*!
    Enables the Branch demand from this crate.
    When the BD is enabled, anytime there's a non zero
    value in the Gl register, a branch demand is created.
    
*/
void 
CCrateController::EnableDemand()  
{
  m_Slot30.Control(26, 10);
}  

/*!
    true if the crate is inhibited.  Note that this
    operation invovles N30.f27.a9 followed by
    a Q test on the branch.  If the multiple
    processes are accessing the same crate 
    concurrently, this function is not reliable.
    


*/
bool 
CCrateController::isInhibited()  
{
  m_Slot30.Control(27, 9);
  return m_myBranch.Qtest();
}  

/*!
    Returns true if the crate is currently
    issuing a demand. This funciton requires
    a n30.f24.a10 followed by a Qtest.
    If other processes or threads are concurrently
    accessing the crate this may return incorrect results.
    
*/
bool 
CCrateController::isDemanding()  
{
  m_Slot30.Control(27, 11);
  return m_myBranch.Qtest();
}  

/*!
    Returns true if the Branch Demand is enabled.  When the
    branch demand is enabled, this module can initiate
    branch demands whenever the GL register is nonzero.
    This function requires a n30.f27.a11 followed by a Q test
    in the branch controller.  This operation is not gaurenteed to
    work if other threads or processes are concurrently accessing
    the branch.

*/
bool 
CCrateController::isDemandEnabled()  
{
  m_Slot30.Control(27, 10);
  return m_myBranch.Qtest();
}  

/*!
    Broadcasts the same control F.A to all
    modules in the crate.  Note that F must be in 
    the range of valid control functions.
    \param f  - Function code to broadcast.
    \param a - Subaddress to broadcast at.
    
    \exception CRangeError - If F is not a control function [8-15,24-31], or A is not a
          valid subaddress [0,16).
    

	\param unsigned int f, unsigned int a

*/
void 
CCrateController::BroadcastControl(unsigned int f, unsigned int a)  
{
  m_Broadcast.Control(f, a);
 
}  

/*!
    Broadcasts the same write F and A to 
    all modules in the crate.
    
    \param f - Function code to broadcast.
    \param a - Subaddress of broadcast.
    \param d - longword data to write.
    
    \exception CRangeException - if either
             f is not a write operation [16-24)
            or a is not a valid subaddress [0,16).
    

	\param unsigned int f, unsigned int a

*/
void 
CCrateController::BroadcastWrite(unsigned int f, unsigned int a, unsigned long d)  
{
  m_Broadcast.Write(f, a, d);
}  

/*!
    Multicasts the same control F.A to a set 
    of modules.  The modules are selected by a 
    bitmask. Low order bit represents slot1.
    e.g. a mask value of: 0x15  multicasts to
    slots 1,3,5
    
    \param f - Function to broadcast.  Must be a 
           valid control function.
    \param a - Subaddress associated with the function.
    \param nMask - Mask of slots to which the operation is
          multicast.
    
    \exception CRangeError - if f not in {[8-16), [24,32)} or
              a is not in [0,16).
    
    \note This operation is not atomic.  It requires writing the Slot mask
            register and then performing the multicast.  If more than one thread
            or process is concurrently accessing the crate, this operation is not reliable.
            this is especially true if the other threads/processes are also doing multicasts.
    



*/
void 
CCrateController::MulticastControl(unsigned int f, unsigned int a, 
				   unsigned long nMask)  
{
  WriteSnr(nMask);
  m_Multicast.Control(f, a);
}  

/*!
    Performs a multicast write.  The specified data is
    written to the set of modules selected by a mask.
    See MulticastControl for information about the
    format of this mask.
    
    \param f  - Function code to execute.
    \param a - Subaddress at which the function is done.
    \param nMask - Mask of slots describing the multicast.
    \param nData - 24 bit data to write.
    
    \note  This function is not atomic with respect to the
         crate.  This means that if other threads or processes
        are concurrently attempting to multicast operations, this
        operation can fail.


*/
void 
CCrateController::MulticastWrite(unsigned int f, unsigned int a, unsigned long nMask, 
				 unsigned long nData)  
{
  WriteSnr(nMask);
  m_Multicast.Write(f, a, nData);
}  

/*!
    The crate is returned to a known configuration.
    This means that C/Z are done, the crate I is removed
    and branch demands are disabled.


*/
void 
CCrateController::InitializeCrate()  
{
 C();
 Z();
 DisableDemand();
 UnInhibit();
}
