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

#ifndef __CCONTROLMODULE_H
#define __CCONTROLMODULE_H

#ifndef __CCONFIGURABLEOBJECT_H
#include <CConfigurableObject.h>
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

class CControlHardware;
class CVMUSB;
class CVMUSBReadoutList;

/*!
   A control module is a configurable object that is attached to an object
   derived from CControlHardware. 
   - This object maintains the configuration for the module.
   - This object defines and delegates to the CControlHardware object
     the following functions:
     - Update  - Updates the internal state of the object from the hardware.
     - Set     - Sets some controllable point in the hardware to a new value
     - Get     - Retreives the value of some controllable point in the hardware
                 from the most recently updated values.

     Hardware objects must also implement onAttach, which is called when
     the hardware is attached to the configuration.  The hardware is required
     to register any configuration parameters (e.g. -base) with the 
     configuration at that time.

     In delegating these operations, CControlModule takes care to synchronize
     with the readout thread if necessary, so that the
     individual hardware modules can be written without any knowledge of
     the existence of the readout thread.
*/
class CControlModule : public CConfigurableObject
{
private:
  CControlHardware* m_pHardware;

public:

  // Canonicals

  CControlModule(std::string name, CControlHardware& hardware);
  virtual ~CControlModule();
  CControlModule(const CControlModule& rhs);
  CControlModule& operator=(const CControlModule& rhs);

private:
  int operator==(const CControlModule& rhs) const;
  int operator!=(const CControlModule& rhs) const;
public:
  // Functions:

  void        Initialize(CVMUSB& vme);
  std::string Update(CVMUSB& vme);
  std::string Set(CVMUSB& vme, const char* what, const char* value);
  std::string Get(CVMUSB& vme, const char* what);

  // Monitor list handling methods:

  void addMonitorList(CVMUSBReadoutList& vmeList);
  void* processMonitorList(void* pData, size_t remaining);

};



#endif
