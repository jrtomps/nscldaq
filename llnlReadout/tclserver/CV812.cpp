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

#include <config.h>
#include "CV812.h"
#include "CControlModule.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"	// for the AM codes.

#include <stdint.h>
#include <stdio.h>


using namespace std;

// Register offsets:

#define CONST(name) static const uint32_t name = 

static const uint8_t  am (CVMUSBReadoutList::a32UserData); // Address modifier.



CONST(Thresholds)   0x0;	// There are 16 16-bit words of these.
CONST(Widths)       0x40;	// There are 2 16-bit words of these.
CONST(DeadTimes)    0x44;	// There are 2 16 bit words of these.
CONST(Majority)     0x48;	// There's only one of these.
CONST(Inhibits)     0x4a;	// Only one of these.
CONST(TestPulse)    0x4c;
CONST(FixedCode)    0xfa;
CONST(MfgAndModel)  0xfc;
CONST(VersionAndSerial) 0xfe;

/*!
   construct the beast.. The shadow registers will all get set to zero
*/
CV812::CV812(string name) :
  CControlHardware(name),
  m_pConfiguration(0)
{
  for (int i =0; i < 16; i++) {
    m_thresholds[i] = 0;
  }
  for (int i=0; i < 2; i++) {
    m_widths[i]    = 0;
    m_deadtimes[i] = 0;
  }
  m_inhibits = 0xffff;		// 0 means inhibited.
}

/*!

  Copy construction:
*/
CV812::CV812(const CV812& rhs) :
  CControlHardware(rhs)
{
  clone(this);
}
/*!
  While destruction could leak I seem to recall problems if I destroy
  the configuration..
*/
CV812::~CV812()
{
}

/*!
  Assignment is a clone:
*/
CV812&
CV812::operator=(const CV812& rhs)
{
  if(this != &rhs) {
    clone(rhs);
  }
  return *this;
}

/*!
  Same configuration implies equality:
*/
int 
CV812::operator==(const CV812& rhs) const
{
  returrn CControlHardware::oeprator==(rhs);
}
/*
   != is the logical inverse of ==
*/
int
CV812::operator!=(const CV812& rhs) const
{
  return !(*this == rhs);
}

///////////////////////////////////////////////////////////////////////////


