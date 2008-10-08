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

#ifndef __CMODULE32_H
#define __CMODULE32_H
#ifndef __VMEMODULE_H
#include "VmeModule.h"
#endif


/*!
   Base class for modules that have 32 bit wide register sets.
   This module is especially useful as a base class for several
   Struck (SIS) modules as their register sets have some
   common characteristics:
   - Full 32 bit.
   - Existence of 'key' registers.

*/

class CModule32
{
  // Data:
private:
  mutable CVmeModule  m_Module;

  // Constructors and other canonicals
public:
  CModule32(unsigned long base, unsigned int size,int crate = 0);
  ~CModule32();

  // Various copy operations are not now supported...although
  // in retrospect the CVmeModule does support this.
private:
  CModule32(const CModule32&);
  CModule32& operator=(const CModule32&);
  int       operator==(const CModule32&);
  int       operator!=(const CModule32&);
public:

  // Selectors

  CVmeModule& getModule() {
    return m_Module;
  }


  // Class functions - not intended for public use, but
  // for the derived actual device classes.

protected:
  unsigned long peek(unsigned long byteoff) const {
    return m_Module.peekl(Offset(byteoff));
  }
  unsigned long poke(unsigned long data, 
		     unsigned long byteoff) const {
    m_Module.pokel(data, Offset(byteoff));
  }
  void Key(unsigned long offset) const; //!< write a key register.

  static unsigned long Offset(unsigned long off)  {
    return off/sizeof(long);
  }
  static void ThrowString(const char*  pLeader,
		     const char*  pMessage) throw (std::string);

};
#endif
