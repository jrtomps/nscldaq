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

static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";//////////////////////////CCAMACScalerLRS2551.cpp file////////////////////////////////////

#include <config.h>
#include "CCAMACScalerLRS2551.h"                  


using namespace std;


static const unsigned int nChannels  = 12;

/*!
  Contruct a CAMAC LeCroy model LRS 2551 scaler 
  \param b - Branch the scaler lives in.
  \param c - Crate the scaler lives in within branch b.
  \param n - Slot the scaler lives in within the branch and crate b,c.
*/
CCAMACScalerLRS2551::CCAMACScalerLRS2551 (unsigned int b,
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
CCAMACScalerLRS2551::CCAMACScalerLRS2551(const CCAMACScalerLRS2551& rhs) :
  CScaler(rhs),
  m_Scaler(m_Scaler)
{
}


/*!
  Assignment operation.  This member function supports assignment of
  an object of this class to an object of the same class.
*/
CCAMACScalerLRS2551& CCAMACScalerLRS2551::operator= (const CCAMACScalerLRS2551& aCCAMACScalerLRS2551)
{ 
  if (this != &aCCAMACScalerLRS2551) {

    CScaler::operator= (aCCAMACScalerLRS2551);
    m_Scaler = aCCAMACScalerLRS2551.m_Scaler;
  }
  return *this;
}

/*!
   Equality checks.
*/
int
CCAMACScalerLRS2551::operator==(const CCAMACScalerLRS2551& rhs) const
{
  return ((m_Scaler == rhs.m_Scaler)    &&
	  CScaler::operator==(rhs));
}

// Functions for class CCAMACScalerLRS2551

/*!
  Performs begin run initialization of the module.
  
*/
void 
CCAMACScalerLRS2551::Initialize()  
{
				// No initialization required.
}  

/*!
    Reads the scaler module into the 
    vector passed as a parameter
    \param Scalers - vector to which the read scalers
    will be appended.
    
    
    \param  Scalers - Vector into which the scalers will be appended.
    
*/
void 
CCAMACScalerLRS2551::Read(vector<unsigned long>& Scalers)  
{
  for(unsigned int i = 0; i < nChannels; i++) {
    Scalers.push_back(m_Scaler.Read(0, i)); // Needs channel by channel reads.
  }
}  

/*!
  Clear the scalers in the module. 
  
*/
void
CCAMACScalerLRS2551::Clear()  
{
  m_Scaler.Control(9,0);	// Clear channels and lam.
}  

/*!
  Return the number of scalers which 
  will be read.
  
  
*/
unsigned int 
CCAMACScalerLRS2551::size()  
{
  return nChannels;
  
}


