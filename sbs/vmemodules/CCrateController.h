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

//////////////////////////CCrateController.h file//////////////////////////////////

#ifndef __CCRATECONTROLLER_H  
#define __CCRATECONTROLLER_H
                               
#ifndef __CBD8210_H
#include "CBD8210.h"
#endif
                                                               
#ifndef __CCAMACMODULE_H
#include "CCamacModule.h"
#endif
                                                               
/*!
   Encapsulates a BiRa 1302 parallel branch
   highway crate controller.
   This controller answers in three slots:
   N28 - C/Z operations.
   N30 - Most of the other operations.
   N26 Broadcast FNAD
   N24 Multicast FNAD as pser the SNR.
   
 */		
class CCrateController      
{ 
private:
  
  CBD8210        m_myBranch;	//!< Branch highway driver we live in.
  CCamacModule   m_Slot28;	//!< C/Z happen via this slot.
  CCamacModule   m_Slot30;	//!< Most other controller ops are here.
  CCamacModule   m_Broadcast;	//!< Broadcast operations via this slot.
  CCamacModule   m_Multicast;	//!< Multicast operations via SNR and this slot
  
public:
	// Constructors, destructors and other cannonical operations: 

    CCrateController (unsigned int b, unsigned int c);
    CCrateController(const CCrateController& rhs);
  ~ CCrateController ( ) { } //!< Destructor.

    CCrateController& operator= (const CCrateController& rhs); //!< Assignment
    int         operator==(const CCrateController& rhs) const; //!< Comparison for equality.
    int         operator!=(const CCrateController& rhs) const {
       return !(operator==(rhs));
    }

	// Selectors for class attributes:
public:

  CBD8210 getCBD8210() const
  { return m_myBranch;
  }
  CCamacModule getSlot28() const
  {
    return m_Slot28;
  }
  CCamacModule getSlot30() const
  { 
    return m_Slot30;
  }
  CCamacModule getBroadcastSlot() const
  {
    return m_Broadcast;
  }
  CCamacModule getMulticastSlot() const
  {
    return m_Multicast;
  }
  


	// Class operations:
public:
     void Z ()  ;
     void C ()  ;
     long Lams ()  ;
     void WriteSnr (unsigned long nMask)  ;
     void UnInhibit ()  ;
     void DisableDemand ()  ;
     void Inhibit ()  ;
     void EnableDemand ()  ;
     bool isInhibited ()  ;
     bool isDemanding ()  ;
     bool isDemandEnabled ()  ;
     void BroadcastControl (unsigned int f, unsigned int a)  ;
     void BroadcastWrite (unsigned int f, unsigned int a, unsigned long d)  ;
     void MulticastControl (unsigned int f, unsigned int a, unsigned long nMask)  ;
     void MulticastWrite (unsigned int f, unsigned int a, unsigned long nMask, unsigned long nData)  ;
     void InitializeCrate ()  ;
 
};

#endif
