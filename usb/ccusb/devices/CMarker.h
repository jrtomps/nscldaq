/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


/**
 * @file CMarker.h
 * @brief Defines the support class for creating Marker words.
 * @author Ron Fox <fox@nscl.msu.edu>
 */

#ifndef __CMARKER_H
#define __CMARKER_H


#ifndef __CREADOUTHARDWARE_H
#include "CReadoutHardware.h"
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif


// Forward class definitions:

class CReadoutModule;
class CCCUSB;
class CCCUSBReadoutList;

/**
 * CMarker
 *    Class to insert a marker in the output buffer.  There is one configuration
 *    option: -value which sets the unsigned 16 bit marker value.
 */
class CMarker : public CReadoutHardware
{
public:
  CMarker();
  CMarker(const CMarker& rhs);
  virtual ~CMarker();
  CMarker& operator= (const CMarker& rhs);
private:
  int operator==(const CMarker& rhs) const;
  int operator!=(const CMarker& rhs) const;

  // The CReadoutHardware interface we need to implement:

public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CCCUSB& controller);
  virtual void addReadoutList(CCCUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;
  
};


#endif
