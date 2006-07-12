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

#ifndef __CVC32CC32_H
#define __CVC32CC32_H

#ifndef __CCAMACCRATE_H
#include "CCAMACCrate.h"
#endif

class CVMEAddressRange;
class CWienerVC32;

/*!
    Concrete class that implements  the connection between a Vc32
    and the CC32.  The distinction between interface and crate is a bit blurred
    in the functionality of this module set.
*/
class CVC32CC32 : public CCAMACCrate 
{
private:
  CVMEAddressRange* m_pCrate;
  CWienerVC32&       m_Interface;
public:
  CVC32CC32(CWienerVC32& interface);
  virtual ~CVC32CC32();
private:
  CVC32CC32(const CVC32CC32& rhs);
  CVC32CC32& operator=(const CVC32CC32& rhs);
  int operator==(const CVC32CC32& rhs) const;
  int operator!=(const CVC32CC32& rhs) const;
public:

  //  Implementation of the pure virtual functions:

public:
  // Implementation of pure virtual members:
  virtual void C()          ;
  virtual void Z()          ;
  virtual void Inhibit()    ;
  virtual void Uninhibit()  ;
  virtual bool isInhibited();

  virtual unsigned long readGL() ;

  virtual unsigned int    read(size_t slot, unsigned int f, unsigned int a)    ;
  virtual unsigned short  read16(size_t slot, unsigned int f, unsigned int a) ;
  virtual void            write(size_t slot, unsigned int f, unsigned int a, 
				unsigned int datum)                            ;
  virtual void            write16(size_t slot, unsigned int f, unsigned int a,
				  unsigned short datum)                        ;
  virtual void            control(size_t slot, unsigned int f, unsigned int a);

  virtual bool X()   ;
  virtual bool Q()   ;

  // Utilities:

private:
  off_t validReadOffset(size_t n, unsigned int f, unsigned int a);
  off_t validWriteOffset(size_t n, unsigned int f, unsigned int a);
  off_t validControlOffset(size_t n, unsigned int f, unsigned int a);


  // Testing support:
#ifdef UNIT_TEST_INCLUDE
public:
#else
private:
#endif
  CWienerVC32& getInterface();
};



#endif
