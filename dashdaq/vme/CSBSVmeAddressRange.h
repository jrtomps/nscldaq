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

#ifndef __CSBSVMEADDRESSRANGE_H
#define __CSBSVMEADDRESSRANGE_H

#ifndef __CVMEADDRESSRANGE_H
#include "CVMEAddressRange.h"
#endif

// Required for sbs bit3:


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
   This class represents an SBS address range.  On construction,
   a memory map is created to an address range specified.  This
   map is then used as the target of the standard peek* /poke* calls.
   The address modifier used is set prior to making the map for the unit.
   It is restored after the map is made.
*/
class CSBSVmeAddressRange : public CVMEAddressRange
{
private:
  bt_desc_t m_handle;		// For the unmap.
  void*     m_pMap;

public:
  CSBSVmeAddressRange(bt_desc_t handle, 
		      unsigned short addressModifier,
		      unsigned long  base,
		      size_t         bytes);
  ~CSBSVmeAddressRange();

  // We can't really support the copy/compare operations.
private:
  CSBSVmeAddressRange(const CSBSVmeAddressRange& rhs);
  CSBSVmeAddressRange operator=(const CSBSVmeAddressRange& rhs);
  int operator==(const CSBSVmeAddressRange& rhs) const;
  int operator!=(const CSBSVmeAddressRange& rhs) const;
public:

  // Members we implement:

  virtual void* mappingPointer();
  
  virtual void pokel(size_t offset, long  data);
  virtual void pokew(size_t offset, short data);
  virtual void pokeb(size_t offset, char  data);

  virtual unsigned long  peekl(size_t offset);
  virtual unsigned short peekw(size_t offset);
  virtual unsigned char  peekb(size_t offset);

  // Utilties:
private:
  void unmap();			// Unmap from the region.
  template<class T> void  poke(size_t offset, T data) {
    rangeCheck(offset*sizeof(T));
    T* p = static_cast<T*>(m_pMap);
    p[offset] = data;
  }

  template<class T> T peek(size_t offset) {
    rangeCheck(offset*sizeof(T));
    T* p = static_cast<T*>(m_pMap);
    return p[offset];
  }
		      
};


#endif
