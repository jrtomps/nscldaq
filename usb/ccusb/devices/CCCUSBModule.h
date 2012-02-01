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
#ifndef __CCCUSBMODULE_H
#define __CCUSBMODULE_H

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
 Supplies support for the internal resources of the CC-USB module. Use this module as any other
module and stick it into a stack.. it won't generate a readout list.  The initialization, however 
is capable of setting up specific internal module devices.   These are managed via configuration options
just like any 'normal' module:
Note: We can't use CCCUSB as the name for this class since that class already exists in the 
      system and represents the CCUSB controller for primitive operations.

\verbatim

\endverbatim
Name             Type        Default         Meaning
-gdgasource      enum        disabled        Source of gate and delay generator A start.. This can be:
                                             in1, in2, in3, event, eventend, usbtrigger, or pulser
-gdbbsource      enum        disabled        Source of gate for GDG B start, can have any of the values
                                             as -gdgasource.
-gdgawidth       integer     1               GDGA gate width in 10ns units. between (1 and 65535).
-gdgadelay       integer     0               GDGB gate delay in 10ns units between (0 and 2^24-1).
-gdgbwidth       integer     1               GDB gate width in 10ns units, between 1 and 65535
-gdgbdelay       integer     0               GDGB delay in 10ns units between 0 and 2^24-1.
-out1            enum        busy            Source of O1 signal one of:
                                             busy, event, gdga, gdgb
				       
                                             or f2
-out1latch       bool        false           Output 1 is latched.
-out1invert      bool        false           Output 1 is inverted.

-out2            enum        trigger         Source of O2 signal.  One of:
                                             acquire, event, gdga gdgb
-out2latch       bool        false
-out2invert      bool        false
-out3            enum        busyend         Source of O3 trigger.. one of:
                                             busyend, busy, gdga, gdgb
-out3latch       bool        false
-out3invert      bool        false



*/
class CCCUSBModule : public CReadoutHardware
{
  // class canonicals:
  //    Implemented:
  
public:
  CCCUSBModule();
  CCCUSBModule(const CCCUSBModule& rhs);
  virtual ~CCCUSBModule();
  CCCUSBModule& operator=(const CCCUSBModule& rhs);
  
  //    Unimplemented:
private:
  int operator==(const CCCUSBModule& rhs) const;
  int operator!=(const CCCUSBModule& rhs) const;

  // THis module implements the CReadoutHardware interface below:

public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CCCUSB& controller);
  virtual void addReadoutList(CCCUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;


  // Module utility functions:
private:
  static int enumIndex(const char**  values, std::string parameter);
  void configureOutput(CCCUSB& controller);
  void configureGdg1(CCCUSB& controller);
  void configureGdg2(CCCUSB& controller);
  void configureDevices(CCCUSB& controller);
};

#endif
