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

#include <config.h>
#include "CVMEAddressRange.h"
#include <RangeError.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


/*!
   Construct the base class address range... just set the 
   member data from the parameters.
   \param am   : unsigned short [in]
      Address modifier to use when accessing this range.
   \param base : unsigned long [in]
      VME base address of the region.
   \param bytes : size_t [in]
      Number of bytes in the address range.

*/
CVMEAddressRange::CVMEAddressRange(unsigned short am, 
				   unsigned long base, 
				   size_t bytes) :
  m_nBase(base),
  m_nBytes(bytes),
  m_nAddressModifier(am)
{}

/*!
  Destruction is an no-op but must be defined to support
 a full destructor chaining back to the base class.
*/
CVMEAddressRange::~CVMEAddressRange()
{}

/*!
    Copy construction, derive classes may disallow this but
    we only know about \em our data and requirements.
*/
CVMEAddressRange::CVMEAddressRange(const CVMEAddressRange& rhs) :
  m_nBase(rhs.m_nBase),
  m_nBytes(rhs.m_nBytes),
  m_nAddressModifier(rhs.m_nAddressModifier)
{}

/*!
   Assignment.  
*/
CVMEAddressRange&
CVMEAddressRange::operator=(const CVMEAddressRange& rhs)
{
  if (&rhs != this) {
    m_nBase            = rhs.m_nBase;
    m_nBytes           = rhs.m_nBytes;
    m_nAddressModifier = rhs.m_nAddressModifier;
  }
  return *this;
}
/*!
   Equality just compare members.
*/
int
CVMEAddressRange::operator==(const CVMEAddressRange& rhs) const
{
  return ((m_nBase         == rhs.m_nBase)        &&
	  (m_nBytes        == rhs.m_nBytes)       &&
	  (m_nAddressModifier == rhs.m_nAddressModifier));
}

/*! 
  Inequality means not of equality.
 */
int
CVMEAddressRange::operator!=(const CVMEAddressRange& rhs) const
{
  return !(*this == rhs);
}

/*!
  Return the base of the map.
*/
unsigned long 
CVMEAddressRange::base() const 
{
  return m_nBase;
}
/*!
   Return the size of the map.
*/
size_t
CVMEAddressRange::size() const
{
  return m_nBytes;
}
/*!
   Return the address modifier used by the map.
*/
unsigned short
CVMEAddressRange::addressModifier() const
{
  return m_nAddressModifier;
}  
/*!
  Check an offset to see that it's in the map.
*/
void
CVMEAddressRange::rangeCheck(size_t offset) const
{
  if (offset >= m_nBytes) {
    throw CRangeError(0, m_nBytes - 1 , offset,
		      "CVMEAddressRange::rangeCheck failed");
  }
}
