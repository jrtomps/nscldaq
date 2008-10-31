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
#include "CEventSegment.h"

/*!
   Concrete classes are expected to override this
   method by providing code that does one-time star to data taking
   initialization of the data taking modules read by this segment.
*/
void
CEventSegment::initialize()
{
}

/*!
   concrete classes are expected to override this method by
   providing code that clears pending data latched in the module.
   Note that this is called after reading each event, and is intended
   to be used for modules for which readout is non-destructive.
*/
void
CEventSegment::clear()
{
}
/*!
   Concreate sub classes are expected to override this method and
   provide code that disables the module as data taking is turned off.
*/
void
CEventSegment::disable()
{
}
//
// The C++ standards provides for the implementation of abstract members.
// we use this to produce a doxygen description of the expectations
// for this member.
/*!
   Concrete subclasses are expecdted to read out the hardware in response to
   an event ttrigger inthis function.
   \param pBuffer - points to ordinary storage into which the method should put
                    its data.
   \parm maxwords - Is the maximum number of uint16_t's that can be read into
                    pBuffer without risking an overrun.
 
   \return size_t
   \retval The number of 16 (uint16_t e.g.) values inserted in pBuffer.

   \throw  CRangeError  if the function detects that it must insert more than maxwords
           into pBuffer.
*/
size_t
CEventSegment::read(void* pBuffer, size_t maxwords)
{
  return 0;
}
