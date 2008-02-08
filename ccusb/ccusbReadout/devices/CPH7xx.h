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
  The PH7xx class is a CC-USB device class for a Phillips 71xx series
CAMAC digizer.  This family includes the Ph7164 (ADC),  Ph 7186 (TDC) as
well as a qdc model.  

The options understood by the PH7xx module are:
\verbatim
Name           Type      Initial           Meaning
-slot          int         0          Slot the module is installed in
-sparse        bool      true         Read in sparse mode 
-readhits      bool      true         Include hit register in the event stream
-pedestals     int[16]   0's          The channel pedestals.
-llt           int[16]   0's          The channel low level thresholds.
-hlt           int[16]   4095's       The channel high level thresholds.
-usellt        bool      false        Enable low level thresholds.
-usehlt        bool      false        Enable high level thresholds.
-usepedestals  bool      false        Enable pedestal subtraction.

\endverbatim
For some analysis systems, you may not be allowed to vary these parameters.
For example, in the ccusbSpecTcl that understands data from this system,
you should always set:
\verbatim

-sparse    true
-readhits  true

\endverbatim

*/
class  CPH7xx : public CReadoutHardware
{
private:
  CReadoutModule*  m_pConfiguration; // Where the configuration params live.

  // class canonicals:

public:
  CPH7xx();
  CPH7xx(const CPH7xx& rhs);
  virtual ~CPH7xx();
  CPH7xx& operator=(const CPH7xx& rhs);

private:
  int operator==(const CPH7xx& rhs) const;
  int operator!=(const CPH7xx& rhs) const;

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
  void         checkedWrite16(CCCUSB& controller,
			      int n, int a, int f, uint16_t data,
			      std::string msgFormat);
  void         check(int status, uint16_t qx, 
		     int n, int a, int f, int d,
		     std::string prefix, std::string format);
};
#endif
