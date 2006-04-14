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

#ifndef __CVMEPIO_H
#define __CVMEPIO_h


/*!
   This class is an abstract base class for all programmed I/O accesses
   to a VME bus.   This class supports single shot, most likely expensive
   accesses to arbitrary locations in VME bus with arbitrary address modifiers.
*/
class CVMEPio 
{
  // Canonicals.. we support all the copy like operations but it's possible
  // the concrete classes may not.

public:
  CVMEPio();
  CVMEPio(const CVMEPio& rhs);
  virtual ~CVMEPio();

  CVMEPio& operator=(const CVMEPio& rhs);
  int      operator==(const CVMEPio& rhs) const;
  int      operator!=(const CVMEPio& rhs) const;

  // Virtual functions that concrete classes must override:

public:
  virtual void write32(unsigned short modifier, unsigned long address, long value) = 0;
  virtual void write16(unsigned short modifier, unsigned long address, short value)= 0;
  virtual void write8(unsigned short modifier,  unsigned long address, char value) = 0;

  virtual unsigned long  read32(unsigned short modifier, unsigned long address) = 0;
  virtual unsigned short read16(unsigned short modifier, unsigned long address) = 0;
  virtual unsigned char  read8 (unsigned short modifier, unsigned long address) = 0;

};

#endif
