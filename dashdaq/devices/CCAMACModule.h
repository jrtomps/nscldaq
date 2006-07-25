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


#ifndef __CCAMACMODULE_H
#define __CCAMACMODULE_H

#ifndef __CRT_UNISTD_H
#include <unistd.h>
#ifndef __CRT_UNISTD_H
#define __CRT_UNISTD_H
#endif
#endif

class CCAMACCrate;


/*!
  Class that abstracts a single camac module.  A CAMAC module is an object
  that lives in a single CAMAC crate and has a slot number that identifies
  how to address it.  All other operations are specific to the module and how
  it operates.
*/
class CCAMACModule
{
  // Data members (attributes)
private:
  CCAMACCrate* m_pCrate;
  size_t       m_slot;

public:
  CCAMACModule(CCAMACCrate& crate, size_t slot);
  CCAMACModule(const CCAMACModule& rhs);
  virtual ~CCAMACModule();

  CCAMACModule& operator=(const CCAMACModule& rhs);
  int operator==(const CCAMACModule& rhs) const;
  int operator!=(const CCAMACModule& rhs) const;


  // Selectors are the only thing this class exports:
public:

  CCAMACCrate& getCrate();
  size_t       getSlot() const;
};
   


#endif
