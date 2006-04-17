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
#include "CVMEPio.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

// Everything is pretty much a no-op and place holders for either later
// or chaining.

CVMEPio::CVMEPio() {}
CVMEPio::CVMEPio(const CVMEPio& rhs) {}
CVMEPio::~CVMEPio() {}

CVMEPio&
CVMEPio::operator=(const CVMEPio& rhs) 
{
  return *this;
}

// Any base class is just about as good as any other.
//
int
CVMEPio::operator==(const CVMEPio& rhs) const
{
  return 1;
}
int
CVMEPio::operator!=(const CVMEPio& rhs) const
{
  return 0;
}
