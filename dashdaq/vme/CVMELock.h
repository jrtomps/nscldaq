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

#ifndef __CVMELOCK_H
#define __CVMELOCK_H

/*! 
   Creating an object of this sort, locks the vme
   subsystem, while destroying it unlocks it.
   This allows code like;
   {
     CVMELock lock;          // locked...
   }                         // unlocked

   which presumably is less error prone than explicit lock/unlocks.
*/
class CVMELock
{
public:
  CVMELock();
  ~CVMELock();
};
#endif
