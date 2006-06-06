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
#include "CCAMACInterface.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

/*!
   Construction just initializes the m_nMaxCrates member to 0:
   Subclasses are supposed to call setCrateCount() to update this
   to reflect the actual number of crates each concrete interface type
   supports.
*/
CCAMACInterface::CCAMACInterface() :
  m_nMaxCrates(0)
{
}
/*!
   Destruction is only needed to make virtual destruction in the hierarchy work.
*/
CCAMACInterface::~CCAMACInterface()
{
}

/*!
   Return the maximum number of crates the interface supports.
*/
size_t
CCAMACInterface::maxCrates() const
{
  return m_nMaxCrates;
}

//---------------------------------------------------------------------------

/*!
   Set the number of crates supported by the interface.  Subclasses are
   expected to call this to inform us as to the number of CAMAC crates they
   each can actually support.
*/
void
CCAMACInterface::setCrateCount(size_t maxCrates)
{
  m_nMaxCrates = maxCrates;
}
