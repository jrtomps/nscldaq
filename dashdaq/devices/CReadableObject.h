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

#ifndef __CREADABLEOBJECT_H
#define __CREADABLEOBJECT_H


#ifndef __CRT_UNISTD_H
#include <unistd.h>
#ifndef __CRT_UNISTD_H
#define __CRT_UNISTD_H
#endif
#endif

/*!
   This is the ultimate base class of the device support class library for
   objects that can be read in response to triggers.  This class
   defines a set of interfaces these classes must implement:
   - initialize must implement code to make the module ready to read the first
                event.  Hidden from view here are all the configuration concerns.
   - read       must implement the actual readout of the module in response to a 
                trigger into a buffer.  The return value is the number of 
		\em bytes read by the module.
   - largestEvent must return the largest possible return value from read.
                this must be at least correct as the DAQ system will likely
		use this to ensusre that the buffer passed to read will be
		sized large enough to accept the data this and other modules
		will produce.

\note This is an abstract base class (ABC).
*/
class CReadableObject 
{
public:
  virtual void    initialize()   = 0;
  virtual size_t  read(void*)    = 0;
  virtual sizse_t largestEvent() = 0;
};

#endif
