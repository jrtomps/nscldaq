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


#ifndef __CBIRA1302CES8210_H
#define __CBIRA1302CES8210_H

#ifndef __CCAMACCRATE_H
#include "CCAMACCrate.h"
#endif

class CVMEAddressRange;
class CCESCBD8210;

/*!
  Concrete class that implements a BiRA 1302 crate contoller connected
  via branch highway to a CES8210 branch highway driver.

 */
class CBiRA1302CES8210 : public CCAMACCrate
{
  // Data:
private:
  CVMEAddressRange* m_pCrate;


  // Constructors etc.
public:
  CBiRA1302CES8210(CCAMACInterface& interface,
		   unsigned int     vmeCrate,
		   unsigned int     camacCrate);
  ~CBiRA1302CES8210();

  // These are not allowed:

private:
  CBiRA1302CES8210(const CBiRA1302CES8210& rhs);
  CBiRA1302CES8210& operator=(const CBiRA1302CES8210& rhs);
  int operator==(const CBiRA1302CES8210& rhs) const;
  int operator!=(const CBiRA1302CES8210& rhs) const;
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

private:
  unsigned long offset(size_t slot, unsigned int function, unsigned int subaddress,
		       bool shortTransfer=true);
  void validateRead(size_t n, unsigned int f, unsigned int a);
  void validateWrite(size_t n, unsigned int f, unsigned int a);
#ifdef UNIT_TEST_INCLUDE
 public:
#endif
  CCESCBD8210& getInterface();
};

#endif
