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

// Implementation of the C785 class VM-USB support for the CAEN V785.


#include <config.h>

#include "CMarker.h"

#include <CItemConfiguration.h>
#include <CVMUSBReadoutList.h>


using namespace std;


static CItemConfiguration::limit valueLow(0);
static CItemConfiguration::limit valueHigh(0xffff);
static CItemConfiguration::Limits valueLimits(valueLow, valueHigh);


/////////////////////////////////////////////////////////////////////////////////
//////////////////////// Overridable operations /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/*!
    Attach the module to its configuration by  storing the config reference and
    adding the -value parameter to the config.
    \param CReadoutModule& configuration

*/
void
CMarker::onAttach()
{


  m_pConfiguration->addParameter("-value", CItemConfiguration::isInteger, &valueLimits, "0");

}
/*!
   The device does not need to be initialized.
*/
void 
CMarker::Initialize(CVMUSB& controller)
{}

/*!
    The module is added to the readout list by fetching the value parameter as a 16 bit integer
    and adding the marker command to the readout list.
*/
void
CMarker::addReadoutList(CVMUSBReadoutList& list)
{
  unsigned int value = getIntegerParameter("-value");

  list.addMarker(static_cast<uint16_t>(value));
}

/*!
  Virtual constructor:

*/
CVMUSBConfigurableObject*
CMarker::clone() const
{
  return new CMarker(*this);
}
/////////////////////////////////////////////////////////////////////
//////////////////// Private utility functions //////////////////////
/////////////////////////////////////////////////////////////////////

// Return the value of an integer parameter.
// Parameters:
//    std::string name - name of the parameter.
// Returns:
//    value
// Throws a string exception (from cget) if there is no such parameter.
// caller is responsible for ensuring the parameter is an int.
//
unsigned int
CMarker::getIntegerParameter(string name)
{
  string sValue =  m_pConfiguration->cget(name);
  unsigned int    value  = strtoul(sValue.c_str(), NULL, 0);

  return value;
}
