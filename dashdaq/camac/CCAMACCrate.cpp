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

// Implementation of the CCAMACCrate class.

#include <config.h>
#include "CCAMACCrate.h"
#include <RangeError.h>
#include <CBadValue.h>
#include <stdio.h>

using namespace std;

/*!
  Construction just means tucking away my interface.
  \param interface CCAMACInterface&
     The interface that is connected to us.
*/
CCAMACCrate::CCAMACCrate(CCAMACInterface& interface) :
  m_Interface(interface)
{}

/*!
   Destruction is a no-op.
*/
CCAMACCrate::~CCAMACCrate()
{
}

/////////////////////////////////////////////////////////////////////////////

/*!
    If the a function code is not a read throw a range error.
    \param f : unsigned int
        The function code to check.
    \throw CRangeError - if the function is not a valid read.
*/
void
CCAMACCrate::requireRead(unsigned int f)
{
  if(f > 7) {
    throw CRangeError(0,7, f, "Read function code required");
  }
}

/*!
   Require that the function code be a write operation or throw.
   see requireRead etc...
*/
void
CCAMACCrate::requireWrite(unsigned int f)
{
  if ( (f < 16) || (f > 23) ) {
    throw CRangeError(16, 23, f, "Write function code required");
  }
}
/*!
  Require that the function code be a valid control operation.
  same as the last two but since there are two valid disjoint ranges
  we throw CBadValue instead on error.
*/

void
CCAMACCrate::requireControl(unsigned int f)
{
  if ( ((f > 7) &&  (f < 16))           ||
       ((f > 23) && (f < 32))) {
    return;
  }
  char fstring[100];
  sprintf(fstring, "%d", f);
  throw CBadValue("[8-16] or [24-31]", fstring, "Control function code required");
}

/*!
   Require a valid slot number.. We're going to include controller slots here 
   which I think can be as high as 30.
*/
void
CCAMACCrate::requireSlot(unsigned int n)
{
  if ((n == 0) || (n > 30)) {
    throw CRangeError(1, 30, n, "Must be a CAMAC Crate slot");
  }
}
/*!
   Require a valid subaddress or throw.
*/
void
CCAMACCrate::requireSubaddress(unsigned int a)
{
  if (a > 15) {
    throw CRangeError(0, 15, a, "Must be a subaddress");
  }
}

/*!
   Get the interface that owns us.. 

*/
CCAMACInterface&
CCAMACCrate::getInterface()
{
  return m_Interface;
}
