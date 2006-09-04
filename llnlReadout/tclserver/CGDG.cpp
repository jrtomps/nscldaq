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
#include "CGDG.h"
#include "CControlModule.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"	// for the AM codes.

#include <stdio.h>

using namespace std;

/*!
  Construction is pretty much a no-op as the configuration is 
  handled at attach time.
*/
CGDG::CGDG(string name) :
  CControlHardware(name),
  m_pConfiguration(0)
{
  for (int i=0; i < 8; i++) {
    m_delays[i] = m_widths[i] = 0; // Testing.
  }
}
/*!
  Same for copy construction.. however this is done by clone just for
  regularity.
*/
CGDG::CGDG(const CGDG& rhs)  : 
  CControlHardware(rhs)
{
  clone(rhs);
}
/*!
   Destruction is also a no-op.
*/
CGDG::~CGDG()
{
}
/*!
   Assignment is a clone:
*/
CGDG&
CGDG::operator=(const CGDG& rhs) 
{
  if (this != &rhs) {
    clone(rhs);
  }
  return *this;
}
/*!
   All GDG's with the same configuration are equivalent.
*/
int
CGDG::operator==(const CGDG& rhs) const
{
  return CControlHardware::operator==(rhs);
}
int
CGDG::operator!=(const CGDG& rhs) const
{
  return CControlHardware::operator!=(rhs);
}
//////////////////////////////////////////////////////////////////////////

/*!
    onAttach is called when the module is attached to a configuration.
    The configuration is set up with the following
    configuration values:
    - base  - Module base address.
*/
void
CGDG::onAttach(CControlModule& configuration)
{
  m_pConfiguration = &configuration;
  configuration.addParameter("-base", CConfigurableObject::isInteger, NULL, 
			     string("0"));
}
/*!
   Update - This updates any internal state about the module that we would
   keep.  It may be, for example that for speed sake we'll cache all the
   module parameters and return them on a get... until one has been changed.
   Update ensures that the internal state is coherent with the module
   hardware state.
   \param vme : CVMUSB&
      Vme controller used to talk with the module.
   \return std::string
   \retval empty on success.
*/
string
CGDG::Update(CVMUSB& vme)
{
  return string("OK - but stub.");
}
/*!
  Set a parameter value. All values must be integers, and the parameters
  must be of one of the following forms:
  - delay[0-nchan-1]
  - width[0-nchan-1].

  \param vme  : CVMUSB&
     VME controller that connects to the VME crate this module is in.
  \param parameter : std::string
     Name of the parameter to modify.
  \param value     : std::string
     integer value to set the parameter to.

  \return std::string
  \retval ERROR - some text   - a problem was detected.
  \retval value - Success.

*/
string
CGDG::Set(CVMUSB& vme, string parameter, string value)
{

  unsigned int channelNumber;
  unsigned int parameterValue;

  // First decode the value as a uint32_t:

  if(sscanf(value.c_str(), "%u", &parameterValue) <= 0) {
    return string("ERROR - Value is not an integer and must be");
  }                       // May need to add range checking.

  // See if the parameter is a width:n:

  if (sscanf(parameter.c_str(), " width%u", &channelNumber) == 1) {
    return setWidth(vme, channelNumber, parameterValue);
  }
  else if(sscanf(parameter.c_str(), " delay%u", &channelNumber) == 1) {
    return setDelay(vme, channelNumber, parameterValue);
  }
  else {
    return string("ERROR - parameter specifier invalid");
  }
  
}
/*!
   Get a parameter value and return it to the caller
   \param vme : CVMUSB&
       reference to the VME controller that talks to the device.
   \param parameter : std::string
       Name of the paramter to retrieve.  Must be of the form
       - delay%u  To get a delay value. 
       - width%u  To get a width value.
   \return std::string
   \retval ERROR - yadayadya    Reports an error.
   \retval some unsigned integer Reports the value read.
*/
string
CGDG::Get(CVMUSB& vme, string parameter)
{
  unsigned int channelNumber;
  
  if(sscanf(parameter.c_str(), " width%u", &channelNumber) == 1) {
    return getWidth(vme, channelNumber);
  }
  else if (sscanf(parameter.c_str(), " delay%u", &channelNumber) == 1) {
    return getDelay(vme, channelNumber);
  }
  else {
    return string("ERROR - parameter specifier invalid");
  }
}
     
/*!
    Clone oursevles... a no op at this point
*/
void
CGDG::clone(const CControlHardware& rhs)
{
}

////////////////////////////////////////////////////////////////////////////

/*
   Retrieve the value of the base parameter.
   Note that if there is no configuration, we return 0 (uninitialized value).

*/
uint32_t 
CGDG::base()
{
  if (m_pConfiguration) {
    string strBase = m_pConfiguration->cget("-base");
    unsigned int base;
    sscanf(strBase.c_str(), "%u", &base);
    return static_cast<uint32_t>(base);
  } 
  else {
    return static_cast<uint32_t>(0);
  }
}
/*
    Set the delay of one of the channels.
    Parameters:
       CVMUSB& vme          - The vme controller communicating with the device.
       unsigned int channel - The channel number (0-7).
       unsigned int value   - The new value
    Returns:
       Stringified value set or an error.
*/
string
CGDG::setDelay(CVMUSB& vme, unsigned int channel, unsigned int value)
{
  if (channel > 7) {
    return string ("ERROR - invalid channel");
  }
  else {
    m_delays[channel] = value;
    return string("OK");
  }
}
/*
   Set the width of one of the channels.
    Parameters:
       CVMUSB& vme          - The vme controller communicating with the device.
       unsigned int channel - The channel number (0-7).
       unsigned int value   - The new value
    Returns:
       Stringified value set or an error.
*/
string
CGDG::setWidth(CVMUSB& vme, unsigned int channel, unsigned int value)
{
  if (channel > 7) {
    return string("ERROR - invalid channel");
  }
  else {
    m_widths[channel] = value;
    return string("OK");
  }

}

/*
   Get the value of a channel delay.
   Parameters:
    CVMUSB& vme           - Vme controller communicating with the device.
    unsigned int channel  - Channel number (0-7)

   Returns:
    stringified value or an error message.
*/
string
CGDG::getDelay(CVMUSB& vme, unsigned int channel)
{
  if (channel > 7) {
    return string("ERROR - invalid channel");
  }
  else {
    char msg[100];
    sprintf(msg, "%d", m_delays[channel]);
    return string(msg);
  }
}
/*
  Get the value of a channel width.
   Parameters:
    CVMUSB& vme           - Vme controller communicating with the device.
    unsigned int channel  - Channel number (0-7)
   Returns:
    stringified value or an error message.
*/
string
CGDG::getWidth(CVMUSB& vme, unsigned int channel)
{
  if (channel > 7) {
    return string("ERROR - invalid channel");
  }
  else {
    char msg[100];
    sprintf(msg, "%d", m_widths[channel]);
    return string(msg);
  }
}
