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

#ifndef __CVMEMODULE_H
#define __CVMEMODULE_H

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

// forward class definitions.

class CVMEInterface;

/*!
   This class is a base class for all VME modules.
   A VME module is installed in a VME crate and
   has a single primary base address.  Note that while
   some modules have several chunks of disjoint address space,
   in general, these are all related in some way to the single
   base address.

*/
class CVMEModule
{
private:
  CVMEInterface*   m_pInterface;
  uint32_t         m_base;

public:
  CVMEModule(CVMEInterface& interface, uint32_t base);
  CVMEModule(const CVMEModule& rhs);
  ~CVMEModule();
  
  CVMEModule& operator=(const CVMEModule& rhs);
  int operator==(const CVMEModule& rhs) const;
  int operator!=(const CVMEModule& rhs) const;

  // Selectors:

public:
  CVMEInterface& getInterface();
  uint32_t       getBase() const;
};
   

#endif

