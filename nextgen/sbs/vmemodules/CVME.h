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

/*
  \class CVME
  \file CVME.h

  Encapsulates a CVME object. CVMEs purpose for existence is so that
  CVMEptr's wont cause memory leaks. A CVME contains a reference counted
  pointer which points to a CVMEptr object which holds the actual mapped
  address and performs the actual reads and writes. When a CVME is
  constructed, its reference count is incremented, and when one is
  destructed, its reference count is decremented. When the reference count
  falls to zero, the object is deleted. This way, the object wont be
  deleted before we are done with it.

  Author:
     Jason Venema
     NSCL
     Michigan State University
     East Lansing, MI 48824-1321
     mailto: venemaja@msu.edu
*/

#ifndef __CVME_H
#define __CVME_H

#ifndef __CVMEPTR_H
#include <CVMEptr.h>
#endif

#ifndef __REFPTR_H
#include <Refptr.h>
#endif

#ifndef __CRTL_STDINT_H
#include <stdint.h>
#ifndef __CRTL_STDINT_H
#define __CRTL_STDINT_H
#endif
#endif

template<class T>
class CVME
{
private:
  CRefcountedPtr<CVMEptr<T> > m_pRCptr;  // a reference counted pointer to the
                                         // CVMEptr which holds the mapping.
  uint32_t   m_nOffset;                    // We need our own offset to do ++ e.g. 

 public:
  // Enumeration of possible Vme devices to access
  enum VmeSpace {
    a16d16,
    a24d16,
    a24d32,
    a32d32,
    geo
  };

  // Default constructor
  CVME<T>(VmeSpace space, uint32_t base, uint32_t length, UInt_t crate=0);
  CVME<T>();
  CVME<T>(CVMEptr<T>* aCVMEptr);
  // Copy constructor
  CVME<T>(const CVME& aCVME);
  // Operator= assignment operator
  CVME<T>& operator= (const CVME<T>& aCVME);

  // Operator== equality operator
  int operator== (const CVME<T>& aCVME)
    {
      return (m_pRCptr == aCVME.m_pRCptr);
    }

  // Mutator functions
 protected:
  void setRCptr(CVMEptr<T> aRCptr) { m_pRCptr = aRCptr; }

  // Public accessor functions
 public:
  uint32_t getOffset() { return m_nOffset;  }
  UInt_t getLength() { return m_pRCptr.operator*().getLength(); }
  Address_t getStart() { return m_pRCptr.operator*().getStart(); }
  Address_t getgenptr(uint32_t nOffset);
  Address_t getcurrptr();

  // Public member functions
 public:
  // Pointer dereference/indexing
  T& operator*();
  T* operator->();
  T& operator[] (UInt_t nOffset);
  T& operator[] (UInt_t nOffset) const;

  // Pointer addition/subtraction
  CVME<T>& operator+(UInt_t nOffset);
  CVME<T>& operator-(UInt_t nOffset);
  CVME<T>& operator+=(UInt_t nOffset);
  CVME<T>& operator-=(UInt_t nOffset);

  // Pointer pre-increment/decrement
  CVME<T>& operator++();
  CVME<T>& operator--();

  // Pointer post-increment/decrement
  CVME<T>& operator++(Int_t);
  CVME<T>& operator--(Int_t);

  // Type conversion operators
 public:
  volatile uint8_t*      asChar();
  volatile uint16_t*  asShort();
  volatile uint32_t*   asLong();
};


/*
  \fn CVME<T>::CVME<T>(VmeSpace space, UInt_t base, UInt_t length)

  Operation Type:
     Constructor

  Purpose:
     Construct a CVME<T> by newing into existence a 
     CVMEptr<T> and encapsulating it in a RefcountedPtr.
  
  \param VmeSpace space - indicates the vme device to map
         UInt_t base    - the base address of the module
	 UInt_t length  - the length of the mapping (bytes)
	 UInt_t crate   - VME Crate number.
 */
template<class T>
CVME<T>::CVME(VmeSpace space, uint32_t base, uint32_t length, UInt_t crate) :
  m_nOffset(0)
{
  CRefcountedPtr<CVMEptr<T> > p(new CVMEptr<T>(space, base, length, crate));
  m_pRCptr = p;
}

/*
  \fn CVME<T>::CVME<T>()

  Operation Type:
     Constructor

  Purpose:
     Construct a CVME<T> which points to nothing.
*/
template<class T>
CVME<T>::CVME() :
  m_nOffset(0)
{

}

/*
  CVME<T>::CVME<T>(CVMEptr<T>& aCVMEptr)

  Operation Type:
     Constructor

  Purpose:
     Constructs an object of type CVME<T> given a CVMEptr object.

  \param CVMEptr<T>& aCVMEptr - the CVMEptr from which to construct this
*/
template<class T>
CVME<T>::CVME(CVMEptr<T>* aCVMEptr) :
  m_nOffset(0)
{
  m_pRCptr = CRefcountedPtr<CVMEptr<T> >(aCVMEptr);

}

/*
  \fn CVME<T>::CVME<T>(const CVME<T>& aCVME)

  Operation Type:
     Copy Constructor

  Purpose:
     Construct a CVME<T> by copying one.
*/
template<class T>
CVME<T>::CVME(const CVME<T>& aCVME) :
  m_pRCptr(aCVME.m_pRCptr),
  m_nOffset(aCVME.m_nOffset)
{

}

/*
  \fn CVME<T>& CVME<T>::operator=(const CVME<T>& aCVME)

  Operation Type:
     Assignment

  Purpose:
     Assign another CVME to this
*/
template<class T>
CVME<T>&
CVME<T>::operator=(const CVME<T>& aCVME)
{
  if(this != &aCVME) {
  
    m_nOffset = m_nOffset;
    m_pRCptr  = aCVME.m_pRCptr;
  }
  return *this;
}

/*
  \fn T& CVME<T>::operator*()

  Operation Type:
     Pointer dereference

  Purpose:
     Returns the value stored by the encapsulated CVMEptr at its 
     current offset. Throws a CRangeError exception if it catches one.
*/
template<class T>
T&
CVME<T>::operator*()
{
  try {
    return (*m_pRCptr)[m_nOffset];
  }
  catch(CRangeError& re) {
    throw re;
  }
}

/*
  \fn T* CVME<T>::operator->()

  Operation Type:
     Pointer dereference

  Purpose:
     Returns a pointer to the value stored by the encapsulated CVMEptr at 
     its current offset. Throws a CRangeError exception if it catches one.
*/
template<class T>
T*
CVME<T>::operator->()
{
  uint32_t nOffset = m_pRCptr->getOffset();

  try {
    m_pRCptr->setOffset(m_nOffset);
    T* ptr =  m_pRCptr.operator*().operator->();   // Here's where we could throw.
    m_pRCptr->setOffset(nOffset);
    return ptr;
  }
  catch(CRangeError& re) {
    m_pRCptr->setOffset(nOffset);                  // Be sure to restore the offset.
    throw re;
  }
}

/*
  \fn T& CVME<T>::operator[](UInt_t nOffset)

  Operation Type:
     Indexing(read/write)

  Purpose:
     Returns the value stored in the encapsulated CVMEptr at
     index nOffset. Throws a CRangeError exception if it catches one.

  \param UInt_t nOffset - the offset at which the value is stored.
*/
template<class T>
T&
CVME<T>::operator[](uint32_t nOffset)
{
  try {
    return m_pRCptr.operator*().operator[](nOffset + m_nOffset);
  }
  catch(CRangeError& re) { 
    throw re;
  }
}

/*
  \fn T& CVME<T>::operator[](UInt_t nOffset) const

  Operation Type:
     Indexing(read/write)

  Purpose:
     Returns the value stored in the encapsulated CVMEptr at
     index nOffset. Throws a CRangeError exception if it catches one.

  \param UInt_t nOffset - the offset at which the value is stored.
*/
template<class T>
T&
CVME<T>::operator[](uint32_t nOffset) const
{
  try {
    return m_pRCptr.operator*().operator[](nOffset + m_nOffset);
  }
  catch(CRangeError& re) {
    throw re;
  }
}

/*
  \fn CVME<T>& CVME<T>::operator+(UInt_t nOffset)

  Operation Type:
     Mutator

     binary + note that this will involve a minimum of 2 copy
     constructions/destructions of a CVME<T>.. however this is not
     too expensive, since it's just a matter  of some reference counting
     stuff.

     No attempt is made to ensure that the resulting pointer actually
     points into the VME space range originally specified.

  \param UInt_t nOffset - the value to add to the current offset
*/
template<class T>
CVME<T>&
CVME<T>::operator+(uint32_t nOffset)
{
  CVME<T> result(*this);
  result.m_nOffset += nOffset;
  return result;

}

/*
  \fn CVME<T>& CVME<T>::operator-(UInt_t nOffset)

  Operation Type:
     Mutator

     Same as operator+ but nOffset is subtracted.

  \param UInt_t nOffset - the value to add to the current offset
*/
template<class T>
CVME<T>&
CVME<T>::operator-(uint32_t nOffset)
{
  return operator+(-nOffset);

}

/*
  \fn CVME<T>& CVME<T>::operator+=(UInt_t nOffset)

  Operation Type:
     Mutator

  Purpose:
     Add a value to the encapsulated CVMEptr<T>'s current
     offset. Returns a pointer to self .
     exception if it catches one.

  \param UInt_t nOffset - the value to add to the current offset
*/
template<class T>
CVME<T>&
CVME<T>::operator+=(uint32_t nOffset)
{
  m_nOffset += nOffset;
  return *this;

}

/*
  \fn CVME<T>& CVME<T>::operator-=(UInt_t nOffset)

  Operation Type:
     Mutator

  Purpose:
     Subtracts a value from the encapsulated CVMEptr<T>'s current
     offset. Returns the new 'pointer' or throws a CRangeError
     exception if it catches one.

  \param UInt_t nOffset - the value to add to the current offset
*/
template<class T>
CVME<T>&
CVME<T>::operator-=(uint32_t nOffset)
{
  m_nOffset -= nOffset;
  return *this;

}

/*
  \fn CVME<T>& CVME<T>::operator++()

  Operation Type:
     Mutator

  Purpose:
     (Pre)Increment the current offset into the encapsulated CVME<T>.
     Throws a CRangeError exception if it catches one.
*/
template<class T>
CVME<T>&
CVME<T>::operator++()
{
  m_nOffset++;
  return *this;

}

/*
  \fn CVME<T>& CVME<T>::operator--()

  Operation Type:
     Mutator

  Purpose:
     (Pre)Decrement the current offset into the encapsulated CVME<T>.
     Throws a CRangeError exception if it catches one.
*/
template<class T>
CVME<T>&
CVME<T>::operator--()
{
  m_nOffset--;
  return *this;

}

/*
  \fn CVME<T>& CVME<T>::operator++(Int_t n)

  Operation Type:
     Mutator

  Purpose:
     (Post)Increment the current offset into the encapsulated CVME<T>.
     Throws a CRangeError exception if it catches one.

  \param Int_t n - dummy parameter to indicate this is a 
                   post-increment operator.
*/
template<class T>
CVME<T>&
CVME<T>::operator++(Int_t n)
{
  CVME<T> result(*this);
  m_nOffset++;
  return result;

}

/*
  \fn CVME<T>& CVME<T>::operator--(Int_t n)

  Operation Type:
     Mutator

  Purpose:
     (Post)Decrement the current offset into the encapsulated CVME<T>.
     Throws a CRangeError exception if it catches one.

  \param Int_t n - dummy parameter to indicate this is a 
                   post-decrement operator.
*/
template<class T>
CVME<T>&
CVME<T>::operator--(Int_t n)
{
  CVME<T> result(*this);
  m_nOffset--;
  return result;

}

/*
  \fn Address_t CVME<T>::getgenptr(UInt_t nOffset)

  Operation Type:
     Selector

  Purpose:
     Return a pointer to the current address+offset into the module
     (via the encapsulated CVMEptr<T>).

  \param UInt_t nOffset - an offset to use to determine the pointer
*/
template<class T>
Address_t
CVME<T>::getgenptr(UInt_t nOffset)
{
  Address_t p = m_pRCptr.operator*().getgenptr(nOffset+m_nOffset);
  if(p) return p;
  else return (Address_t)-1;
}

/*
  \fn Address_t CVME<T>::getcurrptr()

  Operation Type:
     Selector

  Purpose:
     Return a pointer to the current address+current offset into the
     module (via the encapsulated CVMEptr<T>).
*/
template<class T>
Address_t
CVME<T>::getcurrptr()
{
  Address_t p = m_pRCptr.operator*().getgenptr(m_nOffset);
  return p;
}

/*
  \fn CVME<UChar_t> CVME<T>::asChar()

  Operation Type:
     Type conversion operator

  Purpose:
     Returns this as a CVME which maps m_pStart to an address space
     containing data of type UChar_t.
*/
template<class T>
volatile uint8_t*
CVME<T>::asChar()
{
  volatile uint8_t* p = (uint8_t*)m_pRCptr->getStart();
  p         += m_pRCptr->getOffset() * sizeof(T)/sizeof(uint8_t);

  return p;

}

/*
  \fn CVME<UShort_t> CVME<T>::asShort()

  Operation Type:
     Type conversion operator

  Purpose:
     Returns this as a CVME which maps m_pStart to an address space
     containing data of type UShort_t.
*/
template<class T>
volatile uint16_t*
CVME<T>::asShort()
{
  volatile uint16_t* p = (uint16_t*)m_pRCptr->getStart();
  p         += m_nOffset * sizeof(T)/sizeof(uint16_t);

  return p;
}

/*
  \fn CVME<ULong_t> CVME<T>::asLong()

  Operation Type:
     Type conversion operator

  Purpose:
     Returns this as a CVME which maps m_pStart to an address space
     containing data of type ULong_t.
*/
template<class T>
volatile uint32_t*
CVME<T>::asLong()
{
  volatile uint32_t* p = (uint32_t*)m_pRCptr->getStart();
  p         += m_nOffset * sizeof(T)/sizeof(uint32_t);

  return p;
}
#endif
