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

#ifndef __CSBSPIO_H
#define __CSBSPIO_H

//  base class include:

#ifndef __CVMEPIO_H
#include "CVMEPio.h"
#endif

#ifndef BT1003
#define BT1003
#endif

#ifndef __SBS_BTAPI_H
extern "C" {
#include <btapi.h>
}
#ifndef __SBS_BTAPI_H
#define __SBS_BTAPI_H
#endif
#endif


/*!
   This class implements single shot, arbitrary I/O to the VME bus.
   This I/O is not very efficient and should only be used when
   peforming non=speed critical functions like device setup,
   or perhaps as the basis of a Tcl access extension.
*/
class CSBSPio : public CVMEPio
{
private:
  bt_desc_t    m_handle;	// Handle to the SBS API.

  // Canonicals.  We don't support copy like operations.
public:
  CSBSPio(bt_desc_t m_handle);
  virtual ~CSBSPio();
private:
  CSBSPio(const CSBSPio& rhs);
  CSBSPio& operator=(const CSBSPio& rhs);
  int operator==(const CSBSPio& rhs) const;
  int operator!=(const CSBSPio& rhs) const;
  
  //  Class operations:

public:
  virtual void write32(unsigned short modifier, unsigned long address, long value);
  virtual void write16(unsigned short modifier, unsigned long address, short value);
  virtual void write8(unsigned short modifier,  unsigned long address, char value);

  virtual unsigned long  read32(unsigned short modifier, unsigned long address);
  virtual unsigned short read16(unsigned short modifier, unsigned long address);
  virtual unsigned char  read8 (unsigned short modifier, unsigned long address);

  // Utilities:
private:
  void writexx(unsigned short modifier, unsigned long address, 
	       void* value, size_t bytes);
  void readxx(unsigned short modifier, unsigned long address,
	      void*  valueRead, size_t bytes);


};


#endif
