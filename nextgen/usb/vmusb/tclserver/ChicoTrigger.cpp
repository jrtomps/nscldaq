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
#include "ChicoTrigger.h"
#include "CControlModule.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"	// for the AM codes.

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


using namespace std;

// Trigger module registr definitions:

#define CONST(name) static const uint32_t name = 

static const uint8_t  am (CVMUSBReadoutList::a32UserData); // Address modifier.

CONST(ShortWidth) 0x1010;
CONST(LongWidth)  0x1014;
CONST(EnableMask) 0x1018;
CONST(Control)    0x101c;

/**
 * Construction pretty much does nothing
 * we are stateless.
 */
ChicoTrigger::ChicoTrigger(string name) :
  CControlHardware(name),
  m_pConfiguration(0)
{
}
/**
 * Copy constructinojust copies and clones us;
 */
ChicoTrigger::ChicoTrigger(const ChicoTrigger& rhs) :
  CControlHardware(rhs)
{
  m_pConfiguration = rhs.m_pConfiguration;
}
/**
 * Destructor
 */
ChicoTrigger::~ChicoTrigger()
{
}
/**
 * Assignment works like copy construction:
 *
 * @param rhs - source of assignment.
 *
 * @return ChicoTrigger&
 * @retval Referece to *this
 */
ChicoTrigger&
ChicoTrigger::operator=(const ChicoTrigger& rhs)
{
  if (this != &rhs) {
    m_pConfiguration = rhs.m_pConfiguration;
  }
  return *this;
}
/**
 * equality holds if the two configurations match:
 * 
 * @param rhs - object we're comparing *this to.
 * 
 * @return int
 * @retval 1 - equality.
 * @retval 0 - inequality.
 */
int
ChicoTrigger::operator==(const ChicoTrigger& rhs)
{
  return CControlHardware::operator==(rhs);
}
/**
 *  Inequality is also based on inequality
 *  of the configs
 *
 * @param rhs - Object we're being compared with
 * 
 * @retval 1 - inEquality
 * @retval 0 - not unequal.
 */
int
ChicoTrigger::operator!=(const ChicoTrigger& rhs)
{
  return CControlHardware::operator!=(rhs);
}

//////////////////////////////////////////////////////

/**
 * Invoked when the module is attached  to its cofiguration.
 * We only have the base configuration option.
 * - Save the configuration.
 * - register the -base option.
 */
void
ChicoTrigger::onAttach(CControlModule& configuration)
{
  m_pConfiguration = &configuration;
  configuration.addParameter("-base", 
			     CConfigurableObject::isInteger,
			     NULL, string("0"));
}
/**
 * Initialization is not required:
 */
void
ChicoTrigger::Initialize(CVMUSB& vme)
{
}
/**
 * Update is a no-op as well.
 */
void
ChicoTrigger::Update(CVMUSB& vme)
{
}
/**
 * Set a parameter value. Since I'm lazy and since
 * the two widths are coupled we only support 
 * the parameter 'all' which then is followed by
 * a Tcl List that contains in order
 * {short_width long_width mask control_register}
 *
 * @param vme - VM-USB object proxy.
 * @param string parameter - must be "all"
 * @param string value     - must be the list described
 *                           above.
 * @return string
 * @retval "OK" if successful
 * @retval ERROR - error description text
 */
string
ChicoTrigger::Set(CVMUSB& vme, string parameter, string value)
{
}
