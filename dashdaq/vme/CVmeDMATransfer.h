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

#ifndef __CVMEDMATRANSFER_H
#define __CVMEDMATRANSFER_H

#ifndef __CVMEINTERFACE_H
#include "CVMEInterface.h"	// needed for widths.
#endif

// Below is to ensure that size_t gets defined:

#ifndef __CRT_UNISTD_H
#include <unistd.h>
#ifndef __CRT_UNISTD_H
#define __CRT_UNISTD_H
#endif
#endif

/*!
    The CVmeDMATransfer class is an abstract base class
    for DMA transfer objects.  The idea is that you
    interact with a specific interface to specify a DMA
    transfer you may want to do.  This DMA transfer is
    represented by an object from which you can issue
    reads or writes to perform the transfer.
*/
class CVmeDMATransfer
{
  // Member data that describes the transfer.
private:
  unsigned long                m_base;
  size_t                       m_length;
  unsigned short               m_modifier;
  CVMEInterface::TransferWidth m_width;
  
  // Canonicals: while we permit copy like stuff there's
  // no assurance derived classes will.

public:
  CVmeDMATransfer(unsigned short               addressModifier,
		  CVMEInterface::TransferWidth width,
		  unsigned long                base, 
		  size_t                       length);
  CVmeDMATransfer(const CVmeDMATransfer& rhs);
  virtual ~CVmeDMATransfer();

  CVmeDMATransfer& operator=(const CVmeDMATransfer& rhs);
  int operator==(const CVmeDMATransfer& rhs) const;
  int operator!=(const CVmeDMATransfer& rhs) const;

  // Base class methods.. mostly available for implementers of
  // derived classes:
public:
  unsigned long                base()     const;
  size_t                       length()   const;
  unsigned short               modifier() const;
  CVMEInterface::TransferWidth width()    const;

  // Methods that must be implemented by derived clases:

public:
  virtual size_t Read(void* buffer)   = 0;
  virtual size_t Write(void* buffer)  = 0;
  
};

#endif
