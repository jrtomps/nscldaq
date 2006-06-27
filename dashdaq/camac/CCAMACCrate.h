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

#ifndef __CCAMACCRATE_H
#define __CCAMACCRATE_H

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __CRT_UNISTD_H
#include <unistd.h>
#ifndef __CRT_UNISTD_H
#define __CRT_UNISTD_H
#endif
#endif


class CCAMACInterface;

/*!
   Abstract base class for a CAMAC crate.  A CAMAC crate is attached to
   an interface and vica versa.  CAMAC crate objects provide both interface
   and crate controller specific mechanisms for manipulating the crate.
   
*/
class CCAMACCrate
{
private:
  CCAMACInterface& m_Interface;	// Who I'm plugged into.
public:
  // Constructors and canonicals:

  CCAMACCrate(CCAMACInterface& interface);
  virtual ~CCAMACCrate();

  // these are not implemented and hence prevented:
private:
  CCAMACCrate(const CCAMACCrate& rhs);
  CCAMACCrate& operator=(const CCAMACCrate& rhs);
  int operator==(const CCAMACCrate& rhs) const;
  int operator!=(const CCAMACCrate& rhs) const;
public:

  // Final members (these are utilities for derived classes).

protected:
  void requireRead(unsigned int f);
  void requireWrite(unsigned int f);
  void requireControl(unsigned int f);
  void requireSlot(unsigned int  n);
  void requireSubaddress(unsigned int a);

  CCAMACInterface& getInterface();

  // Pure virtual members:

public:
  virtual void C()           = 0;
  virtual void Z()           = 0;
  virtual void Inhibit()     = 0;
  virtual void Uninhibit()   = 0;
  virtual bool isInhibited() = 0;

  virtual unsigned long readGL()  = 0;

  virtual unsigned int    read(size_t slot, unsigned int f, unsigned int a)     = 0;
  virtual unsigned short  read16(size_t sllot, unsigned int f, unsigned int a)  = 0;
  virtual void            write(size_t slot, unsigned int f, unsigned int a, 
				unsigned int datum)                             = 0;
  virtual void            write16(size_t slot, unsigned int f, unsigned int a,
				  unsigned short datum)                         = 0;
  virtual void            control(size_t slot, unsigned int f, unsigned int a);

  virtual bool X()    = 0;
  virtual bool Q()    = 0;

};


#endif
