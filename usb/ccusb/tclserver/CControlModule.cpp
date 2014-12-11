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
#include <CCCUSB.h>
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
CControlModule::CControlModule(string name, 
                               std::unique_ptr<CControlHardware> hardware) :
  CConfigurableObject(name),
  m_pHardware(std::move(hardware))
{
  m_pHardware->onAttach(*this);
}
/*!
   Destroy a module.  The hardware riding along with us is assumed to have
   been dynamically created.
*/
CControlModule::~CControlModule()
{
}

/*! 
  Copy construction.  The hardware is just cloned....configuration must be redone.
*/
CControlModule::CControlModule(const CControlModule& rhs) :
  CConfigurableObject(rhs)
{
  m_pHardware = rhs.m_pHardware->clone();
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
    clearConfiguration();
    CConfigurableObject::operator=(rhs);
    m_pHardware = rhs.m_pHardware->clone();
    m_pHardware->onAttach(*this);
  }
  return *this;
}

/*!
  Do post configuration initialiation.
*/
void
CControlModule::Initialize(CCCUSB& ctlr)
{
  bool mustRelease(false);
  if (CRunState::getInstance()->getState() == CRunState::Active) {
    mustRelease = true;
    CControlQueues::getInstance()->AcquireUsb();
  }
  m_pHardware->Initialize(ctlr);

  if (mustRelease) {
    CControlQueues::getInstance()->ReleaseUsb();
  }

}

/*!
   Update the module.  To do this we may need to acquire the
   Vmusb Interface from readout. 
   \param ctlr : CCCUSB&
      Reference to the ctlr interface.
*/
string
CControlModule::Update(CCCUSB& ctlr)
{
  bool mustRelease(false);
  if (CRunState::getInstance()->getState() == CRunState::Active) {
    mustRelease = true;
    CControlQueues::getInstance()->AcquireUsb();
  }
  string result =  m_pHardware->Update(ctlr);

  if (mustRelease) {
    CControlQueues::getInstance()->ReleaseUsb();
  }
  return result;
    
}
/*!
    Set a module parameter.
    \param ctlr  : CCCUSB& 
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
CControlModule::Set(CCCUSB& ctlr, const char* what, const char* value)
{
  bool mustRelease(false);
  if (CRunState::getInstance()->getState() == CRunState::Active) {
    mustRelease = true;
    CControlQueues::getInstance()->AcquireUsb();
  }
  string reply  = m_pHardware->Set(ctlr, what, value);

  if (mustRelease) {
    CControlQueues::getInstance()->ReleaseUsb();
  }
  return reply;
    
}
/*!
   Retrieve a value from a  module.
   \param ctlr : CCCUSB& 
      Reference to a ctlr usb controller through which the ctlr is accessedd.
   \param what : const char*
      Name of the control point that is being modified.
   \return std::string
   \retval Any message to return to the get command.  In general strings that
       begin ERROR indicate an error, and other strings will just be the value
       of the requested point.
*/
string
CControlModule::Get(CCCUSB& ctlr, const char* what)
{
  bool mustRelease(false);
  if (CRunState::getInstance()->getState() == CRunState::Active) {
    mustRelease = true;
    CControlQueues::getInstance()->AcquireUsb();
  }
  string reply = m_pHardware->Get(ctlr, what);;

  if (mustRelease) {
    CControlQueues::getInstance()->ReleaseUsb();
  }
  return reply;
    
}
