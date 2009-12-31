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

#ifndef CPH7XX_H
#define CPH7XX_H


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

/*!
  CAD811 is a device class for the Ortec AD811 (note the TD811
  should work as well for this class.  This is a very simple module.
  All 8 channnels are read out regardless of the occupancy as the 
  module does not support useful sparse readoutl.

  Options supported are:

\verbatim
Name            Type       Initial           Meaning
-slot           int         0      Slot module is installed in.
-id             int16       0      Marker identifying the module.


\endverbatim

*/
class  CAD811 : public CReadoutHardware
{
private:
  CReadoutModule*  m_pConfiguration; // Where the configuration params live.

  // class canonicals:

public:
  CAD811();
  CAD811(const CAD811& rhs);
  virtual ~CAD811();
  CAD811& operator=(const CAD811& rhs);

private:
  int operator==(const CAD811& rhs) const;
  int operator!=(const CAD811& rhs) const;

  // CReadoutHardware interface:

public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CCCUSB& controller);
  virtual void addReadoutList(CCCUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;

  // Utility functions:

private:
  unsigned int getIntegerParameter(std::string name);
  bool         getBoolParameter(std::string name);
  void         getArray(std::string name, std::vector<uint16_t>& values);
  void         checkedControl(CCCUSB& controller,
			      int n, int a, int f, std::string msgFormat);
  void         check(int status, uint16_t qx, 
		     int n, int a, int f, int d,
		     std::string prefix, std::string format);
};
#endif
