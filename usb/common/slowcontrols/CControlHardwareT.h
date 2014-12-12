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

#ifndef __CCONTROLHARDWARET_H
#define __CCONTROLHARDWARET_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#include <memory>

template<class Ctlr> class CControlModuleT;

/*!
   CControlHardwareT is an abstract base class for programmable electronics that is
   not read out as part of the event stream.  It provides a set of interfaces
   that the TclServer commands can use to communicate with a module.
   It also defines the configuration parameters the module can accept.
   The tacit assumption here is that modules only need to be initialized by
   'setting' controllable parameters.  If other initialization is required,
   it will be necessary for the module itself to maintain sufficient state to ensure
   that this happens at the first access, as presumably by then the configuration is
   initialized.
   
   Make the distinction between:
   - Configuration: Mostly static information used to access the module.
     for most devices this will consist solely of the base address.
   - Parameters: Control points within the device that are dynamically modified.
     For example for the GDG a parameter might be a channel delay.
*/

template<class Ctlr>
class CControlHardwareT
{
  public:
    using RdoList = typename Ctlr::RdoList;
protected:
  CControlModuleT<Ctlr>* m_pConfig;

public:
  // Canonicals:

  CControlHardwareT();
  CControlHardwareT(const CControlHardwareT& rhs);
  virtual ~CControlHardwareT();
  CControlHardwareT& operator=(const CControlHardwareT& rhs);
  int operator==(const CControlHardwareT& rhs) const;
  int operator!=(const CControlHardwareT& rhs) const;
public:
  CControlModuleT<Ctlr>* getConfiguration() const { return m_pConfig;}
  virtual std::unique_ptr<CControlHardwareT> clone() const = 0;	     //!< Virtual copy constr.

  // Pure virtuals the concrete class must override.

public:
  virtual void onAttach(CControlModuleT<Ctlr>& configuration) = 0;  //!< Create config.
  virtual void Initialize(Ctlr& vme);	                     //!< Optional initialization.
  virtual std::string Update(Ctlr& vme) = 0;               //!< Update module.
  virtual std::string Set(Ctlr& vme, 
			  std::string parameter, 
			  std::string value) = 0;            //!< Set parameter value
  virtual std::string Get(Ctlr& vme, 
			  std::string parameter) = 0;        //!< Get parameter value.

  // Interface to support monitoring. that is not used at the moment but maybe will be someday.

  virtual void addMonitorList(RdoList& list);     //!< add items to the monitor list.
  virtual void* processMonitorList(void* pData, size_t remaining);
  virtual std::string getMonitoredData();

};

#include <CControlHardwareT.hpp>
#endif
