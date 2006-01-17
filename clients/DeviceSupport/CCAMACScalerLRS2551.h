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


//////////////////////////CCAMACScalerLRS2551.h file//////////////////////////////////

#ifndef __CCAMACSCALERLRS2551_H  
#define __CCAMACSCALERLRS2551_H
                               
#ifndef __CSCALER_H
#include "CScaler.h"
#endif

#ifndef __CCAMACMODULE_H
#include <CCamacModule.h>
#endif
                               
/*!
   Encapsulates an LRS 2551
   CAMAC scaler.  This is a 12 channel
   camac scaler.  Instantiating the scaler
   allows you to specify the camac address.
   
 */		
class CCAMACScalerLRS2551  : public CScaler        
{ 
private:
  CCamacModule  m_Scaler;	//!< The scaler module representations.
public:
	// Constructors, destructors and other cannonical operations: 

  CCAMACScalerLRS2551 (unsigned int b,
		       unsigned int c,
		       unsigned int n); //!< Default constructor.
  CCAMACScalerLRS2551(const CCAMACScalerLRS2551& rhs); //!< Copy constructor.
  ~CCAMACScalerLRS2551 ( ) { } //!< Destructor.
  
  CCAMACScalerLRS2551& operator= (const CCAMACScalerLRS2551& rhs); //!< Assignment
  int         operator==(const CCAMACScalerLRS2551& rhs) const; //!< Comparison for equality.
  int         operator!=(const CCAMACScalerLRS2551& rhs) const {
    return !(operator==(rhs));
  }
  
  // Selectors for class attributes:
public:
  
  // Mutators:
protected:  
  
  // Class operations:
public:
  virtual   void Initialize ()  ;
  virtual   void Read (STD(vector)<unsigned long>& Scalers) ;
  virtual   void Clear ()  ;
  virtual   unsigned int size ()  ;
  
};

#endif
