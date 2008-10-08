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
//////////////////////////CCAMACScalerLRS4434.h file//////////////////////////////////

#ifndef __CCAMACSCALERLRS4434_H  
#define __CCAMACSCALERLRS4434_H
                               
#ifndef __CSCALER_H
#include "CScaler.h"
#endif
                        
#ifndef __CCAMACMODULE_H
#include <CCamacModule.h>
#endif
       
/*!
   Encapsulates a CAMAC LRS 4439 scaler.
   This module is a 32 channel CAMAC scaler.
   Construction time determines the B,C,N of the module.
   The scaler must be configured as follows:

   LAD switch off.  This enables the scaler to operate in a dual rank 
   latched manner.
 */		
class CCAMACScalerLRS4434  : public CScaler        
{ 
private:
  CCamacModule m_Scaler;	//!< Camac module representing the scaler.
public:
  // Constructors, destructors and other cannonical operations: 
  
  CCAMACScalerLRS4434 (unsigned int b,
		       unsigned int c,
		       unsigned int n); //!< Default constructor.
  CCAMACScalerLRS4434(const CCAMACScalerLRS4434& rhs); //!< Copy constructor.
  ~ CCAMACScalerLRS4434 ( ) { } //!< Destructor.
  
  CCAMACScalerLRS4434& operator= (const CCAMACScalerLRS4434& rhs); //!< Assignment
  int         operator==(const CCAMACScalerLRS4434& rhs) const; //!< Comparison for equality.
  int         operator!=(const CCAMACScalerLRS4434& rhs) const {
    return !(operator==(rhs));
  }
  
  // Selectors for class attributes:
public:
  
  // Mutators:
protected:  
  
  // Class operations:
  
  virtual   void Initialize () ;
  virtual   void Read (std::vector<unsigned long>& Scalers);
  virtual   void Clear () ;
  virtual   unsigned int size ();
  
};

#endif
