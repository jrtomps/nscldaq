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

#ifndef __CCONTROLMODULET_H
#define __CCONTROLMODULET_H

#ifndef __CCONFIGURABLEOBJECT_H
#include <CConfigurableObject.h>
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#include <memory>

template<class Ctlr> class CControlHardwareT;

/*!
   A control module is a configurable object that is attached to an object
   derived from CControlHardwareT. 
   - This object maintains the configuration for the module.
   - This object defines and delegates to the CControlHardwareT object
     the following functions:
     - Update  - Updates the internal state of the object from the hardware.
     - Set     - Sets some controllable point in the hardware to a new value
     - Get     - Retreives the value of some controllable point in the hardware
                 from the most recently updated values.

     Hardware objects must also implement onAttach, which is called when
     the hardware is attached to the configuration.  The hardware is required
     to register any configuration parameters (e.g. -base) with the 
     configuration at that time.

     In delegating these operations, CControlModuleT takes care to synchronize
     with the readout thread if necessary, so that the
     individual hardware modules can be written without any knowledge of
     the existence of the readout thread.
*/
template<class Ctlr>
class CControlModuleT : public CConfigurableObject
{
  public:
    using RdoList = typename Ctlr::RdoList;

private:
  std::unique_ptr<CControlHardwareT<Ctlr>> m_pHardware;

public:

  // Canonicals

  CControlModuleT(std::string name, std::unique_ptr<CControlHardwareT<Ctlr>> hardware);
  virtual ~CControlModuleT();
  CControlModuleT(const CControlModuleT& rhs);
  CControlModuleT& operator=(const CControlModuleT& rhs);

private:
  int operator==(const CControlModuleT& rhs) const;
  int operator!=(const CControlModuleT& rhs) const;
public:
  // Functions:

  void        Initialize(Ctlr& crate);
  std::string Update(Ctlr& crate);
  std::string Set(Ctlr& crate, const char* what, const char* value);
  std::string Get(Ctlr& crate, const char* what);

  // Monitor list handling methods:

  void addMonitorList(RdoList& vmeList);
  void* processMonitorList(void* pData, size_t remaining);
  std::string getMonitoredData();


  CControlHardwareT<Ctlr>* getHardware() { return m_pHardware.get(); }
};

#include <CControlModuleT.hpp>

#endif
