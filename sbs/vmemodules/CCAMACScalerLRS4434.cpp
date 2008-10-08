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
//////////////////////////CCAMACScalerLRS4434.cpp file////////////////////////////////////
#include <config.h>
#include "CCAMACScalerLRS4434.h"                  



using namespace std;



// Manifest constants:

static const unsigned int nChannels = 32;

// Single bit masks in the control register.

static const unsigned short LD  = (1 << 5); //!< Latch Data
static const unsigned short CL  = (1 << 6); //!< CLear.
static const unsigned short RD  = (1 << 7); //!< Read Data (prepare).
static const unsigned short BD  = (1 << 13); //!< auxilliary Bus Disable.
static const unsigned short T   = (1 << 15); //!< Test.


// Shift counts to position bit fields in the control register
// All shift counts are for left shifts (<< ).

static const unsigned int FAshift  = 0; //!< First address.
static const unsigned int FINshift = 8;	//!< Finishing address.
/*!
  Construct a LeCroy model LRS4434 scaler given the CAMAC location.
   \param b - Branch the module lives in.
   \param c - Crate the module lives in.
   \param n - Slot the module lives in.

*/
CCAMACScalerLRS4434::CCAMACScalerLRS4434 (unsigned int b,
					  unsigned int c,
					  unsigned int n) :
  CScaler(),
  m_Scaler(b,c,n)
 
{

} 
/*!
   Copy construction. This is invoked when e.g. an object is passed by value
   to a function.  The copy constructor makes a clone of the rhs object.
*/
CCAMACScalerLRS4434::CCAMACScalerLRS4434(const CCAMACScalerLRS4434& rhs) :
  CScaler(),
  m_Scaler(rhs.m_Scaler)
{
}

/*!
   Assignment operation.  This member function supports assignment of
   an object of this class to an object of the same class.
*/
CCAMACScalerLRS4434& CCAMACScalerLRS4434::operator= (const CCAMACScalerLRS4434& aCCAMACScalerLRS4434)
{ 
    if (this != &aCCAMACScalerLRS4434) {
       CScaler::operator= (aCCAMACScalerLRS4434);
       m_Scaler = aCCAMACScalerLRS4434.m_Scaler;
 
    }
    return *this;
}

/*!
   == operator.
*/
int
CCAMACScalerLRS4434::operator==(const CCAMACScalerLRS4434& rhs) const
{
  return ((m_Scaler == rhs.m_Scaler) &&
	  CScaler::operator==(rhs));
}
/*!
    Performs begin run initialization of the module.
    
*/
void 
CCAMACScalerLRS4434::Initialize()  
{
  m_Scaler.Write(16, 0, 0);	// Clear all ocntrol bits in the control reg.
}  

/*!
    Reads the scaler module into the 
    vector passed as a parameter
    \param Scalers - vector to which the read scalers
                                will be appended.
    

	\param  Scalers - vector into which the scalers will be appended.

*/
void 
CCAMACScalerLRS4434::Read(vector<unsigned long>& Scalers)  
{
  // construct and write the control register so that latch the data,
  // start reading at channel zero and for 32 channels.

  unsigned short creg = (0x1f << FINshift)|BD | LD | RD; // FA = 0.
  m_Scaler.Write(16, 0, creg);
 
  // Now read the scalers. Note we assume that since CAMAC cycles require
  // a usec, there's been the .8usec settling time required between the
  // latch operation above and the scaler being ready for readout.

  for(unsigned int i = 0; i < nChannels; i++) {
    Scalers.push_back(m_Scaler.Read(2, 0));
  }
 
}  

/*!
    Clear the scalers in the module.
    


*/
void 
CCAMACScalerLRS4434::Clear()  
{
  m_Scaler.Write(16, 0, CL); 
}  

/*!
    Return the number of scalers which 
    will be read.

	\param 

*/
unsigned int 
CCAMACScalerLRS4434::size()  
{
  return nChannels;
}
