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
#include "CVMEInterface.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

// Implementation of the impure members of the CVMEInterface class.

/*!
   Destructor is a place holder to ensure that there's
   destructor chaining through the hierarchy
*/
CVMEInterface::~CVMEInterface()
{
}

/*!
   Determines if the interface can do memory maps.
   This will need to be overridden for any subsystem that supports mapping:

   The default implementation can't do anything so:
   \return bool
   \retval false.
*/
bool
CVMEInterface::canMap() const
{
  return false;
}

/*!
   Determines if the interface has an on board list processor.
   This will need to be overridden for any subsystem that supports
   a true hardware list processor.

   The default implementation can't do anything so:

   \return bool
   \retval false
*/
bool
CVMEInterface::hasListProcessor() const
{
  return false;
}
/*!
   Determines if the interface is capable of doing DMA block transfers.
   This will need to be overridden for any subsystem that supports
   true DMA block transfers.

   The default implementation has no capabilities so:
   \return bool
   \retval false
*/
bool
CVMEInterface::hasDMABlockTransfer() const
{
  return false;
}
/*!
   Called when the VME interface is being locked down by the process.
   This should be overridden by the caller.
*/
void
CVMEInterface::onLock()
{
}
/*!
   Called when the VME Interfaces are being unlocked.
*/
void
CVMEInterface::onUnlock()
{
  
}
