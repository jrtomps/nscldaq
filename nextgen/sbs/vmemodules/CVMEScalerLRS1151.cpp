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

static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";//
// Implementation of the LRS1151 VME scaler module code in the production
// readout skeleton.  This module requires a VME mapping of 256 bytes in 
// A24D32 mode.
//
#include <config.h>
#include "CVMEScalerLRS1151.h"


using namespace std;


// Manifest constants;

static const unsigned int nChannels = 16;
static const unsigned int nVmeMapSize = 256;

// The offsets below define the device.  Note that the offsets are in 
// the si  CCamacModule  m_Scaler;	//!< The scaler module representations.  CCamacModule  m_Scaler;	//!< The scaler module representations.ze units of the register size: nameB  - Byte, nameW -word nameL - long
//
static const unsigned int ResetW        = 0;// Full Reset of the module.
static const unsigned int BIMW          = (0x10/sizeof(short));
static const unsigned int ChannelResetL = (0x40/sizeof(long));
static const unsigned int ChannelL      = (0x80/sizeof(long));
static const unsigned int ModIdL        = (0xfa/sizeof(long));

/*! 
  Construct a module at the designated site:
  */
CVMEScalerLRS1151::CVMEScalerLRS1151(UInt_t base, int crate) :
  CVMEScaler(),
  m_Scaler(CVmeModule::a24d32, base, nVmeMapSize, crate)
{}
/*!
   Copy construct a scaler from a right hand side (rhs):
   */
CVMEScalerLRS1151::CVMEScalerLRS1151(const CVMEScalerLRS1151& rhs) :
  CVMEScaler(rhs),
  m_Scaler(rhs.m_Scaler)
{
  
}
/*!
  Assignment operator:
  */
CVMEScalerLRS1151&
CVMEScalerLRS1151::operator=(const CVMEScalerLRS1151& rhs)
{
  if(this != &rhs) {
    CVMEScaler::operator=(rhs);
    m_Scaler = rhs.m_Scaler;
  }
  return *this;
}
/*!
  Equality comparison.
  */
int
CVMEScalerLRS1151::operator==(const CVMEScalerLRS1151& rhs) 
{
	return ((m_Scaler == rhs.m_Scaler) &&
		     CVMEScaler::operator==(rhs));
}
/*
  Initialize the scaler.  All channels are cleared and put
  into Count-up mode after the module is given a master 
  reset.
  */
void
CVMEScalerLRS1151::Initialize()
{
	m_Scaler.pokew(ResetW, 0);     // Reset the module and
	Clear();                       // Clear it.
}
/*!
   Read the module, appending the data to the end of an STL vector.
   \param data - STL Vector to which the data will be appended.
   */
void
CVMEScalerLRS1151::Read(vector<unsigned long>& rData)
{
	unsigned int nOffset = ChannelL;
	for(int i =0; i < nChannels; i++) {
		rData.push_back(m_Scaler.peekl(nOffset));
		nOffset++;
	}
}
/*!
  Clears the module.  
  */
void
CVMEScalerLRS1151::Clear()
{
	unsigned int nOffset = ChannelResetL;
	for(int i = 0; i < nChannels; i++) {
		m_Scaler.peekl(nOffset);
		nOffset++;
	}
}
/*!
    Returns the number of channels in the module:

*/
unsigned int
CVMEScalerLRS1151::size() {
  return nChannels;
}
