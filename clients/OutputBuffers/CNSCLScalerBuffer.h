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

   
//////////////////////////CNSCLScalerBuffer.h file//////////////////////////////////

#ifndef __CNSCLSCALERBUFFER_H  
#define __CNSCLSCALERBUFFER_H
                               
#ifndef __CNSCLOUTPUTBUFFER_H
#include "CNSCLOutputBuffer.h"
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __CRT_STDINT
#include <stdint.h>
#ifndef __CRT_STDINT
#define __CRT_STDINT
#endif
#endif 
                               
/*!
   This class encapsulates the formatting of an NSCL scaler buffer.
   NSCL Scaler buffers include:
   - A standard buffer header with either type SCALERBF or SNAPSCBF
   - An interval start time indicating when the time interval during which 
     these scalers were accumuated began relative to the start of the run.
     (Note that this clock stops when the run is paused).
   - An interval end time indicating when the time interval during which 
     these scalers were accumulated ended relative to the start of the run.
     (Note that this clock stops when the run is paused).
   - A vector of 32 bit incremental scalers.  For the purpose of entity 
     counting,
     Each scaler is considered an entity.
 */		
class CNSCLScalerBuffer  : public CNSCLOutputBuffer        
{ 
private:
  
public:
  // Constructors, destructors and other cannonical operations: 
  
  CNSCLScalerBuffer (unsigned nWords=4096); //!< Default constructor.
  ~ CNSCLScalerBuffer ( ) { } //!< Destructor.
  
private:
  CNSCLScalerBuffer(const CNSCLScalerBuffer& rhs); //!< Copy constructor.
  CNSCLScalerBuffer& operator= (const CNSCLScalerBuffer& rhs); //!< Assignment
  int         operator==(const CNSCLScalerBuffer& rhs) const; //!< Comparison for equality.
  int         operator!=(const CNSCLScalerBuffer& rhs);
public:
  
  // Selectors for class attributes:
public:
  
  // Mutators:
protected:  
  
  // Class operations:
public:  
  void PutScalerVector (const STD(vector)<unsigned long>& vScalers)  ;
  void PutScalerVector(const STD(vector)<uint32_t>& vScalers);
  void SetStartTime (unsigned long nStart)  ;
  void SetEndTime (unsigned long nEndTime)  ;
  
};

#endif
