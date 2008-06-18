#ifndef __CRINGITEM_H
#define __CRINGITEM_H

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

#ifndef __CRT_UNISTD_H
#include <unistd.h>
#ifndef __CRTUNISTD_H
#define __CRTUNISTD_H
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

struct _RingItem;
class CRingBuffer;
class CRingSelectionPredicate;

// Constants:

static const uint32_t CRingItemStaticBufferSize(8192);

/*!  
  This class is a base class for objects that encapsulate ring buffer items
  (as defined in DataFormat.h).  One interesting wrinkle is used to optimize.
  Most items will be small.  For bunches of small items, data allocation/free can
  dominate the performance over the transfer of data into smaller items.
  Therefore, each object will have a 'reasonably size' local, static storage
  for data.  If the requested body size fits in this local static storage, it will
  be used rather than doing an extra memory allocation on construction and free on
  destruction.  

  The body is meant to be filled in by getting the cursor, referencing/incrementing
  it and then storing the cursor back.

*/
class CRingItem {
  // Private data:

private:
  _RingItem*     m_pItem;
  uint8_t*      m_pCursor;
  uint32_t      m_storageSize;
  bool          m_swapNeeded;
  uint8_t       m_staticBuffer[CRingItemStaticBufferSize + 100];

  // Constructors and canonicals.

public:
  CRingItem(uint16_t type, size_t maxBody = 8192);
  CRingItem(const CRingItem& rhs);
  virtual ~CRingItem();
  
  CRingItem& operator=(const CRingItem& rhs);
  int operator==(const CRingItem& rhs) const;
  int operator!=(const CRingItem& rhs) const;



  // Selectors:

public:
  size_t getStorageSize() const;
  size_t getBodySize()    const;
  void*  getBodyPointer();
  void*  getBodyCursor();
  _RingItem*  getItemPointer();
  uint32_t type() const;

  // Mutators:

public:
  void setBodyCursor(void* pNewCursor);

  // Object actions:

  void commitToRing(CRingBuffer& ring);
  bool mustSwap() const;
  

  // class level methods:

  static CRingItem* getFromRing(CRingBuffer& ring, CRingSelectionPredicate& predicate);

  // Utilities.
private:
  void copyIn(const CRingItem& rhs);
  void deleteIfNecessary();
  void newIfNecessary(size_t size);
  
};
#endif
