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
#include "CControlModule.h"
#include <CControlHardware.h>
#include <CVMUSB.h>
#include <CRunState.h>
#include <CControlQueues.h>

using namespace std;

/*!
   Construct a module.  This involves
   initializing the base class, our contained hardware
   and invoking the contained hardware's onAttach function so that
   configuration parameters can be registered.
   \param name : std::string
      Name of the module.  Should be a unique identifier.
   \param hardware : CControlHardware&
       The hardware that is configured by  us.
*/
CControlModule::CControlModule(string name, CControlHardware& hardware) :
  CConfigurableObject(name),
  m_pHardware(&hardware)
{
  //  m_pHardware->onAttach(*this);
}
/*!
   Destroy a module. 
*/
CControlModule::~CControlModule()
{
  // the CControlModule is owned by the hardware... 
}

/*! 
  Copy construction.  The hardware is just cloned....configuration must be redone.
*/
CControlModule::CControlModule(const CControlModule& rhs) :
  CConfigurableObject(rhs)
{
  m_pHardware->clone(*(rhs.m_pHardware));
  m_pHardware->onAttach(*this);
}
/*!
  Assignment, clear our configuration, destroy our hardware
and copy the new.  Note that the module must be reconfigured at this point
*/
CControlModule& 
CControlModule::operator=(const CControlModule& rhs)
{
  if (this != &rhs) {
    delete m_pHardware;
    clearConfiguration();
    CConfigurableObject::operator=(rhs);
    rhs.m_pHardware->clone(*(rhs.m_pHardware));
    clearConfiguration();
    m_pHardware->onAttach(*this);
    
  }
  return *this;
}

CControlModule*
CControlHardware::getConfiguration()
{
  return m_pConfig;
}


/*!
  Do post configuration initialiation.
*/
void
CControlModule::Initialize(CVMUSB& vme)
{
  bool mustRelease(false);
  if (CRunState::getInstance()->getState() == CRunState::Active) {
    mustRelease = true;
    CControlQueues::getInstance()->AcquireUsb();
  }
  m_pHardware->Initialize(vme);

  if (mustRelease) {
    CControlQueues::getInstance()->ReleaseUsb();
  }

}

/*!
   Update the module.  To do this we may need to acquire the
   Vmusb Interface from readout. 
   \param vme : CVMUSB&
      Reference to the vme interface.
*/
string
CControlModule::Update(CVMUSB& vme)
{
  bool mustRelease(false);
  if (CRunState::getInstance()->getState() == CRunState::Active) {
    mustRelease = true;
    CControlQueues::getInstance()->AcquireUsb();
  }
  string result =  m_pHardware->Update(vme);

  if (mustRelease) {
    CControlQueues::getInstance()->ReleaseUsb();
  }
  return result;
    
}
/*!
    Set a module parameter.
    \param vme  : CVMUSB& 
       Reference to the VME controller
    \param what : const char*
       Name of the parameter to change.
    \param value : const char*
       Textual value of parameter.
    \return std::string
    \retval Any message to return to the set command.  In general, strings that
           begin ERROR indicate an error.
*/
string
CControlModule::Set(CVMUSB& vme, const char* what, const char* value)
{
  bool mustRelease(false);
  if (CRunState::getInstance()->getState() == CRunState::Active) {
    mustRelease = true;
    CControlQueues::getInstance()->AcquireUsb();
  }
  string reply  = m_pHardware->Set(vme, what, value);

  if (mustRelease) {
    CControlQueues::getInstance()->ReleaseUsb();
  }
  return reply;
    
}
/*!
   Retrieve a value from a  module.
   \param vme : CVMUSB& 
      Reference to a vme usb controller through which the vme is accessedd.
   \param what : const char*
      Name of the control point that is being modified.
   \return std::string
   \retval Any message to return to the get command.  In general strings that
       begin ERROR indicate an error, and other strings will just be the value
       of the requested point.
*/
string
CControlModule::Get(CVMUSB& vme, const char* what)
{
  bool mustRelease(false);
  if (CRunState::getInstance()->getState() == CRunState::Active) {
    mustRelease = true;
    CControlQueues::getInstance()->AcquireUsb();
  }
  string reply = m_pHardware->Get(vme, what);;

  if (mustRelease) {
    CControlQueues::getInstance()->ReleaseUsb();
  }
  return reply;
    
}


/*----------------------------- Support for periodic monitoring --------------------*/

/**
 ** addMonitorList allows the encapsulated hardware driver to add entries to
 ** a VMUSBReadoutList (presumably the monitoring list
 ** @param vmeList  The list to add to (passed by reference so that it can be modified).
 */
void
CControlModule::addMonitorList(CVMUSBReadoutList& vmeList)
{
  m_pHardware->addMonitorList(vmeList);
}
/**
 ** process the monitor list... again delegated to the encapsluated driver.
 ** @param pData    - Pointer to the first unused byte of the data from the list.
 ** @param remaining- Number of bytes remaining in the list that have not yet been processed.
 ** @return void*
 ** @retval pointer to the first byte of data unconsumed by this driver.
 */
void* 
CControlModule::processMonitorList(void* pData, size_t remaining)
{
  return m_pHardware->processMonitorList(pData, remaining);
}
/**
 * Return the data monitored by the driver.
 * @return string
 * @retval monitored data in form and content that is
 *         driver dependent.
 */
string 
CControlModule::getMonitoredData()
{
  return m_pHardware->getMonitoredData();

}
