#ifndef __SCALERFRAGMENT_H
#define __SCALERFRAGMENT_H

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

#ifndef __EVENTFRAGMENT_H
#include "EventFragment.h"
#endif

/*!
   The scaler event fragment is a fragment that contains
   scaler data.  We can construct this in two ways.

   - On a raw data buffer.
   - On the body of a data buffer and  node/type information,

   Doing this requires the help of several static utility functions.

*/
class ScalerFragment : public EventFragment
{
  uint16_t m_ssig;
  uint32_t m_lsig;
public:
  ScalerFragment(uint16_t* rawBuffer);
  ScalerFragment(uint16_t  node,
		 uint16_t  type,
		 uint16_t* body,
		 size_t    bodyWords);

  // Extract stuff from the body:

  uint32_t startTime() const;
  uint32_t endTime()   const;
  std::vector<uint32_t> scalers() const;
  size_t               size()     const;
  uint32_t             operator[](size_t index) const;  

private:
  uint32_t getLongword(size_t wordOffset);
  uint16_t getWord(size_t wordOffset);
  
};

#endif
