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

#include "CVmeDMATransfer.h"

// Implementation of the CVmeDMATransfer base class non-pure methods.


/*!
  Construct a DMA transfer:
  \param addressModifier : unsigned short [in]
      Address modifier that selects an address space for the transfer.
   \param width : CVMEInterface::TransferWidth [in]
      Width at which transfers should take place.
   \param base : long [in]
      Address to/from which transfers will take place.
   \param length : size_t [in]
      Number of bytes in the transfer.
*/
CVmeDMATransfer::CVmeDMATransfer(unsigned short               addressModifier,
				 CVMEInterface::TransferWidth width,
				 unsigned long                base,
				 size_t                       length) :
  m_base(base),
  m_length(length),
  m_modifier(addressModifier),
  m_width(width)
{}

/*!
    Copy construct a DMA transfer.  This is done pretty much the same way
    as the normal construction except that the member values come from rhs.
*/
CVmeDMATransfer::CVmeDMATransfer(const CVmeDMATransfer& rhs) :
  m_base(rhs.m_base),
  m_length(rhs.m_length),
  m_modifier(rhs.m_modifier),
  m_width(rhs.m_width)
{}

/*!
  Destructor just needed to support chaining 
*/
CVmeDMATransfer::~CVmeDMATransfer()
{}


/*!
   Assignment.. very like copy construction.
*/
CVmeDMATransfer&
CVmeDMATransfer::operator=(const CVmeDMATransfer& rhs)
{
  if (&rhs != this) {
    m_base     = rhs.m_base;
    m_length   = rhs.m_length;
    m_modifier = rhs.m_modifier;
    m_width    = rhs.m_width;
  }
  return *this;
}
/*!
   Equality comparison.. all members are equal:
*/
int
CVmeDMATransfer::operator==(const CVmeDMATransfer& rhs) const
{
  return ((m_base       == rhs.m_base)        &&
	  (m_length     == rhs.m_length)      &&
          (m_modifier   == rhs.m_modifier)    &&
	  (m_width      == rhs.m_width));
}
/*!  
   Inequality is just the negation of equality:
*/
int
CVmeDMATransfer::operator!=(const CVmeDMATransfer& rhs) const
{
  return !(rhs == *this);
}

/*!
    Return the base of the transfer in vme space
*/
unsigned long 
CVmeDMATransfer::base() const
{
  return m_base;
}
/*!
   Return the length of the transfer.
*/
size_t
CVmeDMATransfer::length() const
{
  return m_length;
}
/*!
  Return the address modifier for the transfer.
*/
unsigned short
CVmeDMATransfer::modifier() const
{
  return m_modifier;
}
/*!
   Return the width of the transfer
*/
CVMEInterface::TransferWidth
CVmeDMATransfer::width() const
{
  return m_width;
}
