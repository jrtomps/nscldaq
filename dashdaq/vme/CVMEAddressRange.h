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

#ifndef __CVMEADDRESSRANGE_H
#define __CVMEADDRESSRANGE_H

#ifndef __SYS_TYPES_H
#include <sys/types.h>		// for size_t
#ifndef __SYS_TYPES_H
#define __SYS_TYPES_H
#endif
#endif


/*!
   Abstract base class that defines the interfaces
   implemented by a VME address space class. VME address spacese
   are intended to abstract memory mapped access between the
   VME bus and an application.
*/
class CVMEAddressRange
{
private:
  unsigned long    m_nBase;
  size_t           m_nBytes;
  unsigned short   m_nAddressModifier;
public:
  CVMEAddressRange(unsigned short am, unsigned long base, size_t bytes);
  virtual ~CVMEAddressRange();
  
  // We support copy etc. but other derived classes may not.

  CVMEAddressRange(const CVMEAddressRange& rhs);
  CVMEAddressRange& operator=(const CVMEAddressRange& rhs);

  int operator==(const CVMEAddressRange& rhs) const;
  int operator!=(const CVMEAddressRange& rhs) const;

  // Member functions we implement:

  unsigned long  base() const;
  size_t         size() const;
  unsigned short addressModifier() const;
  void           rangeCheck(size_t offset) const;

  // Pure virtual functions to be implemented by the derived concrete classes:

  virtual void* mappingPointer() = 0;
  
  virtual void pokel(size_t offset, long  data) = 0;
  virtual void pokew(size_t offset, short data) = 0;
  virtual void pokeb(size_t offset, char  data) = 0;

  virtual unsigned long  peekl(size_t offset)    = 0;
  virtual unsigned short peekw(size_t offset)    = 0;
  virtual unsigned char  peekb(size_t offset)    = 0;
  
  
};


#endif
