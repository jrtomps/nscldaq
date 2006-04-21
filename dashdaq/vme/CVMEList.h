
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

#ifndef __CVMELIST_H
#define __CVMELIST_H

/*!
   Abstract base class for list processors.  A noted subclass is
   CSimulatedVMEList which builds on PIO access to the VME crate
   to provide a simulated list environment in which one may
   be able to test some list operations.
*/
class CVMEList 
{
public:

  // Canonical methods Since the class is so purely abstract we implement
  // inline in order to do away with the need for a .cpp file that would
  // be mostly empty air.

  CVMEList() {}
  CVMEList(const CVMEList& rhs) {}
  virtual ~CVMEList() {}

  CVMEList operator=(const CVMEList& rhs) {return *this;}
  int operator==(const CVMEList& rhs) const {return 1; }
  int operator!=(const CVMEList& rhs) const {return !(*this==rhs);}



  // Pure virtual methods:
};

#endif
