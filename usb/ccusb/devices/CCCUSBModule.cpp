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

// Implementation of the CCUSB module support module.

#include <config.h>
#include "CCCUSBModule.h"



#include "CReadoutModule.h"
#include <CCCUSB.h>
#include <CCCUSBReadoutList.h>

#include <tcl.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <set>

#include <iostream>

using namespace std;

///////////////////////////////////////////////////////////////////////
// Validat5or information.
///////////////////////////////////////////////////////////////////////

// Enumerator values for gdg sources.. this is a null terminated array:

const char* GdgSourceValues[] = {
  "disabled",  "in1", "in2", "in3", "event", "eventend", "usbtrigger", "pulser",
  NULL
};

// Enumerator values for out1:

const char* Out1Values[] = {
  "busy", "event", "gdga", "gdgb",
  NULL
};
// out2:

const char* Out2Values[] = {
  "acquire", "event", "gdga", "gdgb",
  NULL
};
const char* Out3Values[] = {
  "busyend", "busy", "gdga", "gdgb",
  NULL
};
static int outLookup[] = {
  0, 1, 6, 7
};

// Range of gdg delays/widths


// widths:

static CConfigurableObject::limit minwidth(1);
static CConfigurableObject::limit maxwidth(65535);
static CConfigurableObject::Limits WidthLimits(minwidth, maxwidth);

// gdga delay:

static CConfigurableObject::limit mindelay(0);
static CConfigurableObject::limit maxdelaya(0x00ffffff);
static CConfigurableObject::limit maxdelayb(0x00ffffff);
static CConfigurableObject::Limits DelayA(mindelay, maxdelaya);
static CConfigurableObject::Limits DelayB(mindelay, maxdelayb);


////////////////////////////////////////////////////////////////////////////////////////
// Canonicals:
///////////////////////////////////////////////////////////////////////////////////////

/*!
   Construction:
*/
CCCUSBModule::CCCUSBModule()
{
  m_pConfiguration = 0;
}
/*!
   Copy construction:
   @param rhs the module we are copying as a template for our construction.
*/
CCCUSBModule::CCCUSBModule(const CCCUSBModule& rhs)
{
  m_pConfiguration = 0;
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
}
/*!
   Assignment.. should be illegal really.
   @param rhs - the guy assigning to us.
   @return CCCUSBModule&
   @retval *this
*/
CCCUSBModule&
CCCUSBModule::operator=(const CCCUSBModule& rhs)
{
  return *this;
}

/*!
  Destruction .. leaks configurations I'm afraid otherwise there are other problems.

*/
CCCUSBModule::~CCCUSBModule()
{
}

/////////////////////////////////////////////////////////////////////
//  CReadoutHardware interface implementation
////////////////////////////////////////////////////////////////////

/*!
   Called when the module is attached to its configuration.
   At this time, the configuration parameters and their constraints
   are defined so that the configuration file can be processed.
   @param configuration - Reference to the module's configuration.
*/
static set<string> gdgsources;
static set<string> out1Values;
static set<string> out2Values;
static set<string> out3Values;

void
CCCUSBModule::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration;

  // gdga/bsource:

  if (gdgsources.empty() ) {
    for (const char** pSrc = GdgSourceValues; *pSrc; pSrc++) {
      gdgsources.insert(string(*pSrc));
    }
  }
  configuration.addParameter("-gdgasource",
			     CConfigurableObject::isEnum,
			     &gdgsources, "disabled");
  configuration.addParameter("-gdgbsource",
			     CConfigurableObject::isEnum,
			     &gdgsources, "disabled");
  // The gate generator width/delay parameters:

  configuration.addParameter("-gdgawidth",
			     CConfigurableObject::isInteger,
			     &WidthLimits, "1");
  configuration.addParameter("-gdgbwidth",
			     CConfigurableObject::isInteger,
			     &WidthLimits, "1");
  configuration.addParameter("-gdgadelay", 
			     CConfigurableObject::isInteger,
			     &DelayA, "0");
  configuration.addParameter("-gdgbdelay",
			     CConfigurableObject::isInteger,
			     &DelayB, "0");

  // Output 1:

  if (out1Values.empty()) {
    for (const char** pValues = Out1Values; *pValues; pValues++) {
      out1Values.insert(string(*pValues));
    }
  }
  configuration.addParameter("-out1", 
			     CConfigurableObject::isEnum,
			     &out1Values, "busy");
  configuration.addParameter("-out1latch",
			     CConfigurableObject::isBool, 0,
			     "false");
  configuration.addParameter("-out1invert",
			     CConfigurableObject::isBool, 0, 
			     "false");

  // Output 2:
 
  if (out2Values.empty()) {
    for (const char** pValues = Out2Values; *pValues; pValues++) {
      out2Values.insert(string(*pValues));
    }
  }
  configuration.addParameter("-out2",
			     CConfigurableObject::isEnum,
			     &out2Values, "trigger");
  configuration.addParameter("-out2latch",
			     CConfigurableObject::isBool, 0, "false");
  configuration.addParameter("-out2invert",
			     CConfigurableObject::isBool, 0, "false");

  // Output 3:


  if (out3Values.empty()) {
    for (const char** pValues = Out3Values; *pValues; pValues++) {
      out3Values.insert(string(*pValues));
    }
  }
  configuration.addParameter("-out3",
			     CConfigurableObject::isEnum,
			     &out3Values, "busyend");
  configuration.addParameter("-out3latch",
			     CConfigurableObject::isBool, 0, "false");
  configuration.addParameter("-out3invert",
			     CConfigurableObject::isBool, 0, "false");
  //

}
/*!

  Called just prior to the start of the run.  This is really the point of this module.
  The CCUSB resources are set up in accordance with the current configuration values.
  @param controller - Reference to a controller object that can be used to talk to the module.
*/

void
CCCUSBModule::Initialize(CCCUSB& controller)
{
  // the action is delegated to a series of utility functions:

  configureGdg1(controller);
  configureGdg2(controller);
  configureDevices(controller);
  configureOutput(controller);
}

/*!
   The module  does not actually read anything out.
   @param list - CVMUSBReadoutList to which we make no contributions:
*/
void
CCCUSBModule::addReadoutList(CCCUSBReadoutList& list)
{
}

/*!
  Used for virtual copy construction.. clone ourselves into a new object:
  @return CReadoutHardware*
  @retval pointer to a copy of ourself.
*/
CReadoutHardware*
CCCUSBModule::clone() const
{
  return new  CCCUSBModule(*this);
}

////////////////////////////////////////////////////////////////////////////
// Utilities:
///////////////////////////////////////////////////////////////////////////

/**
 * This utilitiy is used to translate an enumerated value into an
 * index into an enumerator table.
 * @param values - The table of enumerator values.  This is null terminated.
 * @param parameter - The configuration parameter value to translate.
 * @return int
 * @retval - The index corresp;onding to parameter.
 * @throw  - String exception if there's no match... This should not happen
 *           as the validator should catch that case.
 */
int
CCCUSBModule::enumIndex(const char** values, string parameter)
{
  for (int i =0; *values; i++, values++) {
    if (parameter == *values) return i;
  }
  throw string("CCCUSBModule::enumIndex - could not find a match!!!!");
}
/**
 * COnfigure the outputs.  This function processes the various -out* parameters,
 * builds up an image of the NIM Output selector and writes the register with it.
 * @param controller - CCCUSB controller object (reference).
 */
void
CCCUSBModule::configureOutput(CCCUSB& controller)
{
  uint32_t registerValue = 0;
  
  // O1
  uint32_t o1Register = outLookup[enumIndex(Out1Values, m_pConfiguration->cget("-out1"))];
  if (getBoolParameter("-out1latch")) {
    o1Register |= CCCUSB::OutputSourceRegister::nimO1Latch;
  }
  if (getBoolParameter("-out1invert")) {
    o1Register |= CCCUSB::OutputSourceRegister::nimO1Invert;
  }
  registerValue |= o1Register;


  // O2

  uint32_t o2Register = outLookup[enumIndex(Out2Values, m_pConfiguration->cget("-out2"))] << 8;
  if (getBoolParameter("-out2latch")) {
    o2Register |= CCCUSB::OutputSourceRegister::nimO2Latch;
  }
  if (getBoolParameter("-out2invert")) {
    o2Register |= CCCUSB::OutputSourceRegister::nimO2Invert;
  }
  registerValue |= o2Register;
 
  // O3

  uint32_t o3Register = outLookup[enumIndex(Out3Values, m_pConfiguration->cget("-out3"))] << 16;
  if (getBoolParameter("-out3latch")) {
    o3Register |= CCCUSB::OutputSourceRegister::nimO3Latch;
  }

  if (getBoolParameter("-out3invert")) {
    o3Register |= CCCUSB::OutputSourceRegister::nimO3Invert;
  }
  registerValue |= o3Register;

  // write the register.

  cerr << "Output selector: " << hex << registerValue 
       << dec << endl;
  controller.writeOutputSelector(registerValue);
}
/**
 * Configure Gate and Delay Geneartor A.  Specifically, we configure the gate width and the delay.
 * The source is configure by configureDevices.
 * @param controller - Reference to the controller we will be configuring.
 */
void
CCCUSBModule::configureGdg1(CCCUSB& controller)
{
  uint32_t delay = getIntegerParameter("-gdgadelay") & 0xffff;
  uint32_t width = getIntegerParameter("-gdgawidth");

  uint32_t registerValue = (width << 16) | delay;
  controller.writeDGGA(registerValue);
}
/**
 * Configure the gate and delay for GDGB.. this resource has a 24 bit
 * delay that's spread across two registers.
 * @param controller - Reference to the controller that is being configured.
 */
void
CCCUSBModule::configureGdg2(CCCUSB& controller)
{
  uint32_t delay = getIntegerParameter("-gdgbdelay");
  uint32_t width = getIntegerParameter("-gdgbwidth");
  uint32_t adelay= getIntegerParameter("-gdgadelay");

  uint32_t fineValue = (width << 16) | (delay & 0xffff);
  uint32_t coarseValue    = (delay & 0xff0000) | ((delay >> 16) & 0xff);


  cerr << "B fine value: " << hex << fineValue << endl;
  cerr << "B Coarse value: " << coarseValue << dec << endl;

  controller.writeDGGB(fineValue);
  controller.writeDGGExt(coarseValue);
}
/**
 * Configure the sourcdes for the various devices.  At this point in time
 * This is restricted to the two gate and delay generators... reflected by the
 * GdgSourceValues enumerator.
 * @param controller - The CCCUSB controller object.
 */
void
CCCUSBModule::configureDevices(CCCUSB& controller)
{
  int gdgaSource = enumIndex(GdgSourceValues, m_pConfiguration->cget("-gdgasource"));
  int gdgbSource = enumIndex(GdgSourceValues, m_pConfiguration->cget("-gdgbsource"));

  uint32_t registerValue = (gdgaSource << 16) | (gdgbSource << 24);

  cerr << hex << "Device source: " << registerValue << endl;
  controller.writeDeviceSourceSelectors(registerValue);
}
