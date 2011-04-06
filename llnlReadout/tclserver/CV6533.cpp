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
#include "CV6533.h"

#include "CControlModule.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"	// for the AM codes.

#include <stdint.h>
#include <stdio.h>
#include <tcl.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>

using namespace std;

// Register offset:'

#define Const(name) static const uint32_t name =

// Board parameters (common to all channels):

Const BoardVmax     0x50;
Const BoardImax     0x54;
Const BoardStatus   0x58;
Const Firmware      0x5c;

// Each Channels has a common format that is described later:

Const(Channels[6])  = {
  0x080, 0x100, 0x180, 0x200, 0x280, 0x300
};

// Board configuration parameters:

Const(ChannelCount)   0x8100;
Const(Description)    0x8102;
Const(Model)          0x8116;
Const(SerialNo)       0x811e;
Const(VmeFirmware)    0x8120;

//  Below are the register offsets within each of the Channels array elements:

Const(Vset)      0x00;
Const(Iset)      0x04;
Const(VMon)      0x08;
Const(Imon)      0x0c;
Const(PW)        0x10;
Const(ChStatus)  0x14;
Const(TripTime)  0x18;
Const(SVMax)     0x1c;
Const(RampDown)  0x20;
Const(RampUp)    0x24;
Const(PwDown)    0x28;
Const(Polarity)  0x2c;
Const(Temp)      0x30;

// Global status register bits:

Const(Chan0Alarm)    0x0001;
Const(Chan1Alarm)    0x0002;
Const(Chan2Alarm)    0x0004;
Const(Chan3Alarm)    0x0008;
Const(Chan4Alarm)    0x0010;
Const(PwrFail)       0x0080;
Const(OverPwr)       0x0100;
Const(MaxVUncal)     0x0200;
Const(MaxIUncal)     0x0400;

// Individual channel status register bits.

Const(On)           0x0001;
Const(RampUp)       0x0002;
Const(RampDown)     0x0004;
Const(OverCurrent)  0x0008;
Const(OverVoltage)  0x0010;
Const(UnderVoltage) 0x0020;
Const(MaxV)         0x0040;
Const(MaxI)         0x0080;
Const(Trip)         0x0100;
Const(OverPower)    0x0200;
Const(Disabled)     0x0400;
Const(Interlocked)  0x0800;
Const(Uncalibrated) 0x1000;


/*------------------------------------- Canonical methods */

/**
 * Construct the object.  For now we won't do anything
 * with the monitored registers.  We will just let those
 * populate as the update/monitor list runs.
 */
CV6533::CV6533(string name) :
  CControlHardware(name),
  m_pConfiguration(0)
{
}
/**
 * Copy construction; clone already does what we need
 * just invoke it:
 */
CV6533::CV6533(const CV6533& rhs) :
  CControlHardware(rhs)
{
  *this = clone(rhs);
}
/**
 * Destruction is not really needed since we are not going to be
 * destroyed for the life of the program.
 */
CV6533::~CV6533()
{
}
/**
 *  Assignment is also taken care of by clone for the most part.
 * Just need to be sure we don't do anything on a self assign.
 * @param rhs - The object being assigned to us.
 * @return CV6533
 * @retval *this
 */
CV6533&
CV6533::operator=(const CV6533& rhs)
{
  if (this != &rhs) {
    clone(rhs);
  }
  return *this;
}
/**
 * Equal configuration implies equality.
 * @param rhs  - The object to check with.
 */
int
CV6533::operator==(const CV6533& rhs) const
{
  return CControlHardware::operator==(rhs);
}
/**
 ** Inequality as usual is the inverse of equality:
 ** @param rhs - the object to compare with *this
 */
int
CV6533::operator!=(const CV6533& rhs) const
{
  return !(*this == rhs);
}
/*------------------ Operations for running the module -----*/


/**
 * This function is called when the object is attached
 * to its configuration.  We must save the configuration
 * and register our parameters.  At present, for HV
 * units we are not going to save any configuration.
 * that would be the responsibility of the GUI the user
 * interacts with
 * @param configuration - Configuration object that will
 *                        maintain our object's configuration.
 */
void
CV6533::onAttach(CControlModule& configuration)
{
  m_pConfiguration = &configuration;
  configuration.addParameter("-base", \
			     CConfigurableObject::isInteger,
			     NULL, string("0"));
}
/**
 * Called to initalize the module.  For us initialization is
 * playing it safe: All the voltages are set to zero.
 * Each channel is turned off.
 * @param vme   - Object that proxies for the VM-USB 
 *                controller module.
 */
void
CV6533::Initialize(CVMUSB& vme)
{


  CVMUSBReadoutList list;	// List of configuration ops.
  for (int i =0; i < 6; i++) {
    turnOff(list, i);
    setRequestVoltage(list, i, 0.0);
  }
  // Execute the list.
  size_t buffer;		// No actual reads..
  int status = vme.executeList(list,
			       &buffer, sizeof(buffer),
			       &buffer);

}
/**
 * For devices with write-only registers, this function
 * would set the device to a configuration that matches
 * state held in the driver.  This function serves
 * no purpose for the CAEN V6533 and thus is a no-op.
 * @param vme - VM-USB proxy object.
 * @return string
 * @retval OK
 */
string
CV6533::Update(CVMUSB& vme)
{
  return "OK";
}
/**
 *  Processes set operations for the device.  See
 * the header for a description of the supported 
 * parameters.
 * @param vme         - Proxy object for the VM-USB controller.
 * @param parameter   - Name of the parameter to set.
 *                      in the parmeter set described in the
 *                      header the n trailing each paramter name
 *                      is the channel number (0-5).
 * @param value       - Value of the parameter.
 * @return string
 * @retval "OK"   - The set worked correctly.
 * @retval "ERROR - ..." Some error occured. The remainder
 *                       of the string describes
 *                       the error
 */
string
CV6533::Set(CVMUSB& vme, string parameter, string value)
{
  // The meaning of the value depends on the actual parameter
  // type.  What this function does is determine the
  // root parameter name and the channel number (if
  // applicable) and dispatch to the appropriate function.

  unsigned channelNumber;
  CVMUSBReadoutList list;
  try {
    if (parameter == "globalmaxv") {
      setGlobalMaxVoltage(vme, value);
    }
    else if (parameter == "globalmaxI") {
      setGlobalMaxCurrent(vme, value);
    }
    else if (sscanf(parameter.c_str(), "v%u", &channelNumber) == 1) {
      setRequestVoltage(list, channelNumber, atof(value.c_str()));
    }
    else if (sscanf(parameter.c_str(), "i%u",
		    &channelNumber) == 1) {
      setRequestCurrent(list, channelNumber, atof(value.c_str()));
    }
    else if(sscanf(parameter.c_str(), "on%u", &channelNumber) == 1) {
      setChannelOnOff(list, channelNumber, strToBool(value));
    }
    else if (sscanf(parameter.c_str(), "ttrip%u", &channelNumber) == 1) {
      setTripTime(list, channelNumber, atof(value.c_str()));
    }
    else if (sscanf(parameter.c_str(), "svmax%u", &channelNumber) == 1) {
      setMaxVoltage(list, channelNumber, atof(value.c_str()));
    }
    else if (sscanf(parameter.c_str(), "rdown%u", &channelNumber) == 1) {
      setRampDownRate(list, channelNumber, atof(value.c_str()));
    } 
    else if (sscanf(parameter.c_str(), "rup%u", &channelNumber) == 1) {
      setRampUpRate(list, channelNumber, atof(value.c_str()));
    }
    else if (sscanf(parameter.c_str(), "pdownomode&u", &channelNumber) ==1 ) {
      setPowerDownMode(list, channelNumber, value);
    }
    else if (sscanf(parameter.c_str(), "polarity%u", &channelNumber) == 1) {
      setPolarity(list, channelNumber, value);
    }
    else {
      throw string("Unrecognized parameter");
    }

    // If there's a non zero length list, execute it.
    
    if (list.size() > 0) {
      size_t buffer;
      int status= vme.executeList(list,
				  buffer, sizeof(buffer),
				  &buffer);
      if (status != 0) {
	throw string("VME list execution failed");
      }
    }
    catch(string msg) {
      string error = "ERROR - ";
      error += msg;
      return error;
    }
    return string("OK");
}
