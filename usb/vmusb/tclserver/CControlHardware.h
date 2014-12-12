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

#ifndef __CCONTROLHARDWARE_H
#define __CCONTROLHARDWARE_H

#include <CVMUSB.h>
#include <CControlHardwareT.h>

using CControlHardware = CControlHardwareT<CVMUSB>;

//
//#ifndef __STL_STRING
//#include <string>
//#ifndef __STL_STRING
//#define __STL_STRING
//#endif
//#endif
//
//class CVMUSB;
//class CVMUSBReadoutList;
//class CControlModule;
//
///*!
//   CControlHardware is an abstract base class for programmable electronics that is
//   not read out as part of the event stream.  It provides a set of interfaces
//   that the TclServer commands can use to communicate with a module.
//   It also defines the configuration parameters the module can accept.
//   The tacit assumption here is that modules only need to be initialized by
//   'setting' controllable parameters.  If other initialization is required,
//   it will be necessary for the module itself to maintain sufficient state to ensure
//   that this happens at the first access, as presumably by then the configuration is
//   initialized.
//   
//   Make the distinction between:
//   - Configuration: Mostly static information used to access the module.
//     for most devices this will consist solely of the base address.
//   - Parameters: Control points within the device that are dynamically modified.
//     For example for the GDG a parameter might be a channel delay.
//*/
//
//class CControlHardware
//{
//protected:
//  CControlModule* m_pConfig; //!< not owned by this.
//public:
//  // Canonicals:
//
//  CControlHardware();
//  CControlHardware(const CControlHardware& rhs);
//  virtual ~CControlHardware();
//
//  CControlHardware& operator=(const CControlHardware& rhs);
//  int operator==(const CControlHardware& rhs) const;
//  int operator!=(const CControlHardware& rhs) const;
//public:
//
//  /**! Retrieve the configuration
//   *
//   * If this has not been attached yet, then it returns nullptr.
//   * Otherwise, it returns the CControlModule that contains its 
//   * configuration.
//   */
//  CControlModule* getConfiguration() const { return m_pConfig;}
//
//  // Pure virtuals the concrete class must override.
//
//public:
//  /**! Create the configuration for the hardware. 
//   *
//   * A CControlModule initially has no configuration parameters. It is 
//   * the hardware that tells it what configuration parameters are required.
//   * That happens in this method.
//   */
//  virtual void onAttach(CControlModule& configuration) = 0;  //!< Create config.
//  virtual void Initialize(CVMUSB& vme);	                     //!< Optional initialization.
//  virtual std::string Update(CVMUSB& vme) = 0;               //!< Update module.
//  virtual std::string Set(CVMUSB& vme, 
//			  std::string parameter, 
//			  std::string value) = 0;            //!< Set parameter value
//  virtual std::string Get(CVMUSB& vme, 
//			  std::string parameter) = 0;        //!< Get parameter value.
//  virtual void clone(const CControlHardware& rhs) = 0;	     //!< Virtual copy constr.
//
//  // Interface to support monitoring.
//
//  virtual void addMonitorList(CVMUSBReadoutList& vmeList);     //!< add items to the monitor list.
//  virtual void* processMonitorList(void* pData, size_t remaining);
//  virtual std::string getMonitoredData();
//  
//};
//
#endif
