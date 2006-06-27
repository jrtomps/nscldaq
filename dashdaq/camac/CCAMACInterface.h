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

#ifndef __CCAMACINTERFACE_H
#define __CCAMACINTERFACE_H

#ifndef __CRT_UNISTD_H
#include <unistd.h>
#ifndef __CRT_UNISTD_H
#define __CRT_UNISTD_H
#endif
#endif



class CCAMACCrate;

/*!
    This is an abstract base class out of which concrete interfaces to
    CAMAC crates can be built.  An interface is something that can control
    one or more CAMAC crates.  Please make the distinction between an interface
    and a controller which is the right most module of a CAMAC crate and controls a
    single crate.  In general, an interface is the thing to which the front panel  of the
    controller is connected.
*/
class CCAMACInterface
{
protected:
  size_t m_nMaxCrates;		//!< Max number of crates this interface supports.

public:
  CCAMACInterface();
  virtual ~CCAMACInterface();

public:
  virtual size_t maxCrates() const;

  virtual bool         haveCrate(size_t crate)   = 0;
  virtual CCAMACCrate* removeCrate(size_t crate) = 0;
  virtual CCAMACCrate& operator[](size_t crate)  = 0;
  virtual bool         online(size_t crate)     = 0;


protected:
  void setCrateCount(size_t maxCrates);
};


#endif
